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
	
    std::cout << "The median is " << paths[paths.size()/2].size() << '\n';

	cout << "Max path length: " << maxLen <<  endl;
	cout << "Min path length: " << minLen <<  endl;
	cout << "Med path length: " << paths[paths.size()/2].size() <<  endl;
	float sum = 0;
	for (uint i = 0; i < paths.size(); i++)
	{
		sum += paths[i].size();
	}
	cout << "Avg path length: " << (sum/paths.size()) <<  endl;
}

void simplifyPaths(vector<ParticlePath>& paths, Time minTime)
{
	cout << "Simplifying paths: min time will be " << minTime << endl;
	#pragma omp parallel for schedule(dynamic, 500)
	for (int i = 0; i < paths.size(); i++)
	{
		paths[i].simplify(minTime);
	}
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
#pragma omp parallel for schedule(dynamic, 500)
	for(int i = 0; i < totalFaces; ++i )
	{
		allPaths[i] = getParticlePath(faceHandles[i]);
		int fractionDone = (int)((double)(i+1) * 100. / (double)totalFaces);
		int prevFractionDone = (int)((double)(i) * 100. / (double)totalFaces);
		if ( (fractionDone % 10) == 0  && fractionDone != prevFractionDone)
		{
			cout << fractionDone << "% Done" << endl;
		}
	}
	cleareCache();
	cout << "run took " << duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count() << " milliseconds" << endl;
	std::cout << "Fuckup count: " << fuckupCount << endl;
	cout << "Expected min path length: " << floorf(((maxTime-minTime)/dt)) << endl;
	reportPathsStatistics(allPaths);
	//simplifyPaths(allPaths, dt / 10.f);
	//reportPathsStatistics(allPaths);
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

	Mesh::HalfedgeHandle excludeHalfEdge;
	bool exclude = false;

	int convergenceCheckCounter = 0;
	int numConsecutiveClosePoints = 0;

	while (currentTime <= tmax )
	{
		const int currentOwnerIdx = currentFace.idx();
		const Triangle& currentTriangle = triangles[currentOwnerIdx];
		float pointConvRadius = VectorFieldsUtils::getPerimeter(currentTriangle) / 1000.f;
		Vec3f field = getField(currentPoint, currentOwnerIdx, currentTime);

		if (particlePath.size() > 3)
		{
			if (particlePath.tryCollapseLastPoints(pointConvRadius))
			{
				numConsecutiveClosePoints++;
			}
			else
			{
				numConsecutiveClosePoints = 0;
			}
		}

		if (numConsecutiveClosePoints > 50)
		{
			currentTime += dt;
			particlePath.pushBack(currentPoint, currentTime);
			continue;
		}
		
		if (VectorFieldsUtils::isCloseToZero(field.length()))
		{
			currentTime += dt;
			particlePath.pushBack(currentPoint, currentTime);
			continue;
		}

		convergenceCheckCounter = (convergenceCheckCounter + 1) % 50; 
		if (convergenceCheckCounter == 0) // Every so often
		{
			Point conv;
			if (particlePath.isConverged(pointConvRadius, (dt/100), 10, &conv))
			{
				currentTime += dt;
				particlePath.pushBack(currentPoint, currentTime);
				continue;
			}
		}
		
		Point next = currentPoint + field * dt;

		if( VectorFieldsUtils::isInnerPoint(next, currentTriangle))
		{
			currentPoint = next;
			currentTime = currentTime + dt;
			particlePath.pushBack(next, currentTime);
			continue;
		}

		// Next we find next owner face. If owner face changed then we need to change next particle point to be on the
		// edge of the new owner face

		const Normal& normal = normals[currentOwnerIdx];
		Point intersection;
		bool breakSearch = false;
		bool intersectionFound = false;
		bool removeExclusion = false;
		for(Mesh::ConstFaceHalfedgeIter cfhei(fieldedMesh.cfh_begin(currentFace)); cfhei != fieldedMesh.cfh_end(currentFace); ++cfhei)
		{
			/*if(exclude && cfhei.handle() == excludeHalfEdge)
			{
				removeExclusion = true;
				continue;
			}
*/
			Point& from = fieldedMesh.point(fieldedMesh.from_vertex_handle(cfhei));
			Point& to = fieldedMesh.point(fieldedMesh.to_vertex_handle(cfhei));

			if (!VectorFieldsUtils::intersectionRaySegment(currentPoint, field, from, to, normal, intersection)) 
			{
				continue;
			}

			double actualTimeInterval = dt * ( (intersection - currentPoint).length() / (next - currentPoint).length());
			currentFace = fieldedMesh.opposite_face_handle(cfhei.handle());
			currentPoint = intersection;
			int ccc = 1;
			Triangle& t = triangles[currentFace.idx()];
			Vec3f delta = (centroids[currentFace.idx()] - intersection).normalized() * FLT_EPSILON * 10;
			currentPoint += delta;
			while(!VectorFieldsUtils::isInnerPoint(currentPoint, t))
			{
				currentPoint = intersection + delta * ccc;
				++ccc;
			}
			currentTime = currentTime + (Time)actualTimeInterval;
			particlePath.pushBack(intersection, currentTime);
			excludeHalfEdge = fieldedMesh.opposite_halfedge_handle(cfhei.handle());
			exclude = true;
			removeExclusion = false;
			intersectionFound = true;
			break;
		}

		if(currentFace.idx() < 0) 
		{
			break;
		}
		if (!intersectionFound)
		{
			fuckupCount++; 
			break;
		}
		if(removeExclusion)
		{
			exclude = false;
		}
	}
	return particlePath;
}

Vec3f PathFinder::getField(const Point& p,const int fid, const Time time)
{
	return VectorFieldsUtils::projectVectorOntoTriangle(
								VectorFieldsUtils::intepolate<Vec3f>(
								VectorFieldsUtils::stdToBarycentric(p, triangles[fid]), 
								VectorFieldsUtils::calculateField(faceVertexFields[fid][0], time),
								VectorFieldsUtils::calculateField(faceVertexFields[fid][1], time), 
								VectorFieldsUtils::calculateField(faceVertexFields[fid][2], time)),
								normals[fid]);
}