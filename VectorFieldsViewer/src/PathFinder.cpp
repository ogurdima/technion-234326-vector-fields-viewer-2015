#include "PathFinder.h"
#include "FieldedMesh.h"
#include "../OpenMesh/Core/Mesh/AttribKernelT.hh"
#include <chrono>
#include <omp.h>

PathFinder::PathFinder() : 
	dt(0.1f),
	tmin(0),
	tmax(1),
	hasValidConfig(false),
	pathDepth(10),
	fuckupCount(0)
{
	
}

bool PathFinder::configure(const FieldedMesh& aMesh_, const Time& dt_)
{
	
	
	fieldedMesh = FieldedMesh(aMesh_);

	Time tmax_ = fieldedMesh.maxTime();
	Time tmin_ = fieldedMesh.minTime();

	if (tmax_ <= tmin_|| (dt_ >= (tmax_ - tmin_))) 
	{
		hasValidConfig = false;
		return hasValidConfig;
	}

	// caching
	int size = fieldedMesh.n_faces();
	if(size < 1)
	{
		hasValidConfig = false;
		return false;
	}
	triangles.resize(size);
	centroids.resize(size);
	oneRingFaceIds.resize(size);
	faceFields.resize(size);
	normals.resize(size);

	for(Mesh::ConstFaceIter fit(fieldedMesh.faces_begin()), fitEnd(fieldedMesh.faces_end()); fit != fitEnd; ++fit ) 
	{
		int idx = fit.handle().idx();

		triangles[idx] = fieldedMesh.getFacePoints(fit);
		faceFields[idx] = fieldedMesh.getVectorField(fit);

		centroids[idx] = VectorFieldsUtils::getTriangleCentroid(triangles[idx]);
		normals[idx] = VectorFieldsUtils::getTriangleNormal(triangles[idx]);

		for(Mesh::ConstFaceFaceIter curFace(fieldedMesh.cff_begin(fit)), end(fieldedMesh.cff_end(fit));
			curFace != end; ++curFace) 
		{
			int neighborId = curFace.handle().idx();
			if (neighborId < 0) 
			{
				continue;
			}
			oneRingFaceIds[idx].push_back(neighborId);
		}
	}

	tmax = tmax_;
	tmin = tmin_;
	dt = dt_;
	hasValidConfig = true;
	return hasValidConfig;
}

vector<ParticlePath> PathFinder::getParticlePaths()
{
	if (!hasValidConfig) 
	{
		throw std::exception("Calling getParticlePaths with badly configured PathFinder");
	}

	vector<ParticlePath> allPaths;
	int totalFaces = fieldedMesh.n_faces();
	int facesDone = 0;
	vector<Mesh::FaceHandle> faceHandles;
	faceHandles.reserve(totalFaces);

	for(Mesh::ConstFaceIter fit(fieldedMesh.faces_begin()), fitEnd(fieldedMesh.faces_end()); fit != fitEnd; ++fit ) 
	{
		faceHandles.push_back(fit.handle());
	}
	allPaths.resize(totalFaces);

	auto start_time = std::chrono::high_resolution_clock::now();
#pragma omp parallel for schedule(dynamic, 500)
	for(int i = 0; i < totalFaces; ++i ) 
	{
		allPaths[i] = getParticlePath(faceHandles[i]);
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	auto time = end_time - start_time;
	
	
	std::cout << "run took " <<  std::chrono::duration_cast<std::chrono::milliseconds>(time).count() << " milliseconds.\n";

	std::cout << "Fuckup count: " << fuckupCount << std::endl;
	return allPaths;
}

ParticlePath PathFinder::getParticlePath(const Mesh::FaceHandle& faceHandle)
{
	ParticlePath particlePath;

	Point pstart = VectorFieldsUtils::barycentricToStd(Point(1.f/3.f), triangles[faceHandle.idx()]);
	
	particlePath.pushBack(pstart, tmin);

	Mesh::FaceHandle ownerFace = faceHandle;

	ParticleSimStateT curState;
	curState.ownerFace = faceHandle;
	curState.p = pstart;
	curState.t = tmin;
	
	Mesh::HalfedgeHandle excludeHalfEdge;
	bool exclude = false;
	while (curState.t <= tmax )
	{
		const int currentOwnerIdx = curState.ownerFace.idx();
		Vec3f field = getOneRingLerpField(curState.p, currentOwnerIdx, curState.t);
		if (VectorFieldsUtils::isCloseToZero(field.length()))
		{
			curState.t += dt;
			particlePath.pushBack(curState.p, curState.t);
			continue;
		}

		Point next = curState.p + field * dt;
		if (!_finite(field[0]) || !_finite(field[1]) || !_finite(field[2]))
		{
			bool debug = true;
		}

		if( VectorFieldsUtils::isInnerPoint(next, triangles[curState.ownerFace.idx()]))
		{
			curState.p = next;
			curState.t = curState.t + dt;
			particlePath.pushBack(next, curState.t);
			//std::cout << "Owner face stayed " << curState.ownerFace << std::endl;
			
			continue;
		}

		// Next we find next owner face. If owner face changed then we need to change next particle point to be on the
		// edge of the new owner face

		const Normal& normal = fieldedMesh.normal(curState.ownerFace);
		Point intersection;
		bool breakSearch = false;
		bool intersectionFound = false;
		for(Mesh::ConstFaceHalfedgeIter cfhei(fieldedMesh.cfh_begin(curState.ownerFace)); cfhei != fieldedMesh.cfh_end(curState.ownerFace); ++cfhei)
		{
			if(exclude && cfhei.handle() == excludeHalfEdge)
			{
				exclude = false;
				continue;
			}

			Point& from = fieldedMesh.point(fieldedMesh.from_vertex_handle(cfhei));
			Point& to = fieldedMesh.point(fieldedMesh.to_vertex_handle(cfhei));

			if (!VectorFieldsUtils::intersectionRaySegmentDima(curState.p, field, from, to, normal, intersection)) 
			{
				continue;
			}

			double actualTimeInterval = dt * ( (intersection - curState.p).length() / (next - curState.p).length());
			//std::cout << "Owner face changed from " << curState.ownerFace << " to " << fieldedMesh.opposite_face_handle(cfhei.handle()) << std::endl;
			curState.ownerFace = fieldedMesh.opposite_face_handle(cfhei.handle());
			curState.p = intersection;
			curState.t = curState.t + (Time)actualTimeInterval;
			particlePath.pushBack(intersection, curState.t);
			excludeHalfEdge = fieldedMesh.opposite_halfedge_handle(cfhei.handle());
			exclude = true;
			intersectionFound = true;

			if (curState.ownerFace.idx() != -1)
			{
				bool b = false;
				if (!VectorFieldsUtils::isInnerPoint(curState.p, triangles[curState.ownerFace.idx()]))
				{
					Vec3f fieldContinuationVec = field.normalized() * (NUMERICAL_ERROR_THRESH * 2);
					curState.p = intersection + VectorFieldsUtils::projectVectorOntoTriangle(fieldContinuationVec, normals[curState.ownerFace.idx()]);
				}
				if (!VectorFieldsUtils::isInnerPoint(curState.p, triangles[curState.ownerFace.idx()]))
				{
					Vec3f fieldContinuationVec = field.normalized() * (NUMERICAL_ERROR_THRESH * 4);
					curState.p = intersection - VectorFieldsUtils::projectVectorOntoTriangle(fieldContinuationVec, normals[curState.ownerFace.idx()]);
				}
			}

			break;
		}

		if(curState.ownerFace.idx() < 0) 
		{
			break;
		}
		if (!intersectionFound)
		{
			if(!VectorFieldsUtils::isInnerPoint(curState.p, triangles[curState.ownerFace.idx()]))
			{
				fuckupCount++; 
				
			}
			break;
			//throw new std::exception("Intersection was not found");
		}

	}
	return particlePath;
}

Vec3f PathFinder::getOneRingLerpField(const Point& p, const int ownerIdx, const Time time)
{
	vector<int>& ringIds = oneRingFaceIds[ownerIdx];
	int size = ringIds.size();
	if(size == 0)
	{
		return VectorFieldsUtils::calculateField(faceFields[ownerIdx], time);
	}


	vector<double> distances(size + 1);
	vector<Vec3f> fields(size + 1);
	double totalDist(0);
	int i = 0;
	for(; i < size; ++i)
	{
		int idx = ringIds[i];
		distances[i] =  (p - centroids[idx]).length();
		totalDist += distances[i];
		fields[i] = VectorFieldsUtils::calculateField(faceFields[idx], time);
	}

	distances[i] = (p - centroids[ownerIdx]).length();
	totalDist += distances[i];
	fields[i] = VectorFieldsUtils::calculateField(faceFields[ownerIdx], time);
	
	Vec3f totalField(0.f);
	for(int j = 0; j <= i; ++j)
	{
		/*if (!_finite(fields[j][0]))
		{
			bool debug = true;
		}*/
		totalField += fields[j] * (float)((totalDist - distances[j]) / totalDist);
		/*if (!_finite(totalField[0]))
		{
			bool debug = true;
		}*/
	}
	
	// now we cheat by projecting totalField onto ownerFace's plane
	return VectorFieldsUtils::projectVectorOntoTriangle(totalField, normals[ownerIdx]);
}