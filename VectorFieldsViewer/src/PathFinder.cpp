#include "PathFinder.h"
#include "FieldedMesh.h"
#include "../OpenMesh/Core/Mesh/AttribKernelT.hh"
#include <chrono>
#include <omp.h>

using std::cout;
using std::endl;
using std::chrono::milliseconds;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;


void reportPathsStatistics(vector<ParticlePath>& paths)
{
	if (paths.size() < 1)
	{
		return;
	}
	
	std::nth_element(paths.begin(), paths.begin() + paths.size()/2, paths.end(), ParticlePath::compareBySize);
	int maxLen = std::max_element(paths.begin(), paths.end(), ParticlePath::compareBySize)->size();
	int minLen = std::min_element(paths.begin(), paths.end(), ParticlePath::compareBySize)->size();
	float sum = 0;
	for (uint i = 0; i < paths.size(); i++)
	{
		sum += paths[i].size();
	}

	std::cout << "====================< Paths Statistics >==================="	<< endl;
	cout << "Max path length: " << maxLen										<< endl;
	cout << "Min path length: " << minLen										<< endl;
	cout << "Med path length: " << paths[paths.size()/2].size()					<< endl;
	cout << "Avg path length: " << (sum/paths.size())							<< endl;
	std::cout << "==========================================================="	<< endl;
}

PathFinder::PathFinder() : 
	dt(0.1f),
	tmin(0),
	tmax(1),
	hasValidConfig(false),
	fuckupCount(0)
{
	
}

bool PathFinder::configure(const FieldedMesh& aMesh_, const Time& dt_, const Time& minTime, const Time& maxTime)
{
	hasValidConfig = false;
	fieldedMesh = aMesh_;
	int size = fieldedMesh.n_faces();

	dt = dt_;
	tmax = maxTime;
	tmin = minTime;

	if (tmax <= tmin|| (dt_ >= (tmax - tmin)) || size < 1) 
	{
		return hasValidConfig;
	}
	hasValidConfig = true;
	return hasValidConfig;
}

vector<ParticlePath> PathFinder::getParticlePaths(const FieldedMesh& aMesh_, const Time& dt_, const Time& minTime, const Time& maxTime)
{

	configure(aMesh_, dt_, minTime, maxTime);
	if (!hasValidConfig) 
	{
		throw std::exception("Calling getParticlePaths with badly configured PathFinder");
	}
	auto start_time = high_resolution_clock::now();
	int totalFaces = fieldedMesh.n_faces();
	vector<Mesh::FaceHandle> faceHandles;
	faceHandles.reserve(totalFaces);
	for(Mesh::ConstFaceIter fit(fieldedMesh.faces_begin()), fitEnd(fieldedMesh.faces_end()); fit != fitEnd; ++fit ) 
	{
		faceHandles.push_back(fit.handle());
	}
	vector<ParticlePath> allPaths;
	allPaths.resize(totalFaces);
	cache();

//#pragma omp parallel for schedule(dynamic, 500)
	for(int i = 0; i < totalFaces; ++i )
	{
		allPaths[i] = getParticlePath(faceHandles[i]);
		int fractionDone = (int)((double)(i+1) * 100. / (double)totalFaces);
		int prevFractionDone = (int)((double)(i) * 100. / (double)totalFaces);
		if ( (fractionDone % 10) == 0  && fractionDone != prevFractionDone)
		{
			cout << fractionDone << "th percentile path computation finished" << endl;
		}
	}
//==========< End of parallel part >============// 
	cleareCache();
	cout << "Run took " << duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count()/1000 << " seconds" << endl;
	cout << "Problematic paths count: " << fuckupCount << endl;
	cout << "Expected min path length: " << floorf(((maxTime-minTime)/dt)) << endl;
	reportPathsStatistics(allPaths);
	return allPaths;
}

void PathFinder::cache()
{
	int size = fieldedMesh.n_faces();
	triangles.resize(size);
	centroids.resize(size);
	normals.resize(size);
	faceVertexFields.resize(size);

	for(Mesh::ConstFaceIter fit(fieldedMesh.faces_begin()), fitEnd(fieldedMesh.faces_end()); fit != fitEnd; ++fit ) 
	{
		int idx = fit.handle().idx();
		Mesh::ConstFaceVertexIter cvit(fieldedMesh.cfv_iter(fit));
		for(int i = 0; i < 3; ++i, ++cvit)
		{
			triangles[idx][i] = fieldedMesh.point(cvit);
			faceVertexFields[idx][i] = fieldedMesh.vertexField(cvit);
		}
		centroids[idx] = VectorFieldsUtils::getTriangleCentroid(triangles[idx]);
		normals[idx] = VectorFieldsUtils::getTriangleNormal(triangles[idx]);
	}
}

void PathFinder::cleareCache()
{
	triangles = vector<Triangle>();
	centroids = vector<Point>();
	normals = vector<Normal>();
	faceVertexFields  = vector<VectorT<vector<VectorFieldTimeVal>,3>>();
}

ParticlePath PathFinder::getParticlePath(Mesh::FaceHandle& faceHandle)
{
	ParticlePath particlePath;

	Mesh::FaceHandle& currentFace = faceHandle;
	Point currentPoint = VectorFieldsUtils::barycentricToStd(Point(1.f/3.f), triangles[faceHandle.idx()]);
	Time currentTime = tmin;
	particlePath.pushBack(currentPoint, currentTime);

	//int convergenceCheckCounter = 0;
	int numConsecutiveClosePoints = 0;

	while (currentTime < tmax)
	{
		// Next section locally simplifies path. It looks at two last points in the path, and
		// if they are close enough collapses them into one.
		if (particlePath.size() > 3)
		{
			if (particlePath.tryCollapseLastPoints(NUMERICAL_ERROR_THRESH * 10))
			{
				numConsecutiveClosePoints++;
			}
			else
			{
				numConsecutiveClosePoints = 0;
			}
		}
		Time timeDelta = std::min(tmax - currentTime, dt);
		if (numConsecutiveClosePoints > 5)
		{
			// There were numConsecutiveClosePoints consecutive collapses. So minimal sphere's containing
			// last numConsecutiveClosePoints points radius is ar most numConsecutiveClosePoints * pointConvRadius
			// Means we are stuck in one place. Increase curTime by dt and see maybe field changed (continue)
			numConsecutiveClosePoints = 0;
			currentTime += timeDelta;
			particlePath.pushBack(currentPoint, currentTime);
			continue;
		}

		// Every so often check if last points in the path are bounded.
		// If so we are stuck, need to wait for field change (continue)
		//convergenceCheckCounter = (convergenceCheckCounter + 1) % 50; 
		//if (convergenceCheckCounter == 0) 
		//{
		//	if (particlePath.isConverged(pointConvRadius, (dt/100), 10))
		//	{
		//		cout << "Converged" << endl;
		//		currentTime += dt;
		//		particlePath.pushBack(currentPoint, currentTime);
		//		continue;
		//	}
		//}

		if(!getintersection(currentFace, currentPoint, currentTime, timeDelta))
		{
			fuckupCount++; 
			break;
		}
		particlePath.pushBack(currentPoint,currentTime);

		if(currentFace.idx() < 0) 
		{
			// This means we got to the boundary of the mesh, so we need to stop the computation here
			break;
		}
	}
	return particlePath;
}