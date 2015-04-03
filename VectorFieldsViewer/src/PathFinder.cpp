#include "PathFinder.h"
#include "FieldedMesh.h"
#include "../OpenMesh/Core/Mesh/AttribKernelT.hh"
#include <chrono>
#include <omp.h>
#include <math.h>

using std::cout;
using std::endl;
using std::chrono::milliseconds;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;

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
	int step = 5;
	int count = 0;
	int fractionDone = 0;
//#pragma omp parallel for schedule(dynamic, 500) shared(count) shared(fractionDone)
	for(int i = 0; i < totalFaces; ++i )
	{
		allPaths[i] = getParticlePath(faceHandles[i]);
		++count;
		if ( (count * 100 / totalFaces) == (fractionDone + step))
		{
			fractionDone += step;
//#pragma omp critical(PRINT)
			cout << "\r" << fractionDone << "% done";
		}
	}
	cout << endl;
	cleareCache();
	cout << "run took " << duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count() << " milliseconds" << endl;
	std::cout << "Fuckup count: " << fuckupCount << endl;
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

	int convergenceCheckCounter = 0;

	while (currentTime <= tmax )
	{
		float timeDelta = std::min(dt, tmax - currentTime);
		const int currentOwnerIdx = currentFace.idx();
		const Triangle& currentTriangle = triangles[currentOwnerIdx];
		Vec3f field = getField(currentPoint, currentOwnerIdx, currentTime);
		if (VectorFieldsUtils::isCloseToZero(field.length()))
		{
			currentTime += dt;
			particlePath.pushBack(currentPoint, currentTime);
			continue;
		}

		if ((++convergenceCheckCounter % 50) == 0) // Every so often
		{
			Point conv;
			if (particlePath.isConverged(VectorFieldsUtils::getPerimeter(currentTriangle) / 1000.f, (dt/100), 10, &conv))
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
		bool intersectionFound = false;
		for(Mesh::ConstFaceHalfedgeIter cfhei(fieldedMesh.cfh_begin(currentFace)); cfhei != fieldedMesh.cfh_end(currentFace); ++cfhei)
		{
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
				currentPoint = intersection + delta * (ccc++);
			}
			currentTime = currentTime + (Time)actualTimeInterval;
			particlePath.pushBack(intersection, currentTime);
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