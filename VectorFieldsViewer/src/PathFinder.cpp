#include "PathFinder.h"
#include "FieldedMesh.h"
#include "../OpenMesh/Core/Mesh/AttribKernelT.hh"

PathFinder::PathFinder() : 
	dt(0.1),
	tmin(0),
	tmax(1),
	hasValidConfig(false),
	pathDepth(10),
	fuckupCount(0)
{

}

bool PathFinder::configure(const FieldedMesh& aMesh_, Time dt_, Time tmin_, Time tmax_)
{
	if (tmax_ <= tmin_|| (dt_ >= (tmax_ - tmin_))) 
	{
		hasValidConfig = false;
		return hasValidConfig;
	}
	fieldedMesh = FieldedMesh(aMesh_);
	tmax = tmax_;
	tmin = tmin_;
	dt = dt_;
	hasValidConfig = true;
	return hasValidConfig;
}

vector<ParticlePath> PathFinder::getParticlePaths()
{
	if (!hasValidConfig) {
		throw std::exception("Calling getParticlePaths with badly configured PathFinder");
	}

	vector<ParticlePath> allPaths;
	int totalFaces = fieldedMesh.n_faces();
	int facesDone = 0;
	for(Mesh::ConstFaceIter fit(fieldedMesh.faces_begin()), fitEnd(fieldedMesh.faces_end()); fit != fitEnd; ++fit ) 
	{
		ParticlePath facePath = getParticlePath(fit.handle());
		allPaths.push_back(facePath);
		facesDone++;
		if(facesDone % 100 == 0)
		{
			std:: cout << "Calculated " << facesDone << " paths of " << totalFaces << std::endl;
		}
	}
	std::cout << "Fuckup count: " << fuckupCount << std::endl;
	return allPaths;
}




ParticlePath PathFinder::getParticlePath(const Mesh::FaceHandle& faceHandle)
{
	ParticlePath particlePath;

	Triangle pts(fieldedMesh.getFacePoints(faceHandle));
	Vec3f field = fieldedMesh.faceVectorField(faceHandle, tmin);
	Point pstart = VectorFieldsUtils::barycentricToStd(Point(1./3.), pts);
	
	particlePath.pushBack(pstart, tmin);

	Mesh::FaceHandle ownerFace = faceHandle;

	ParticleSimStateT curState;
	curState.ownerFace = faceHandle;
	curState.p = pstart;
	curState.t = tmin;
	
	double stepThreshold = dt / 500;
	
	Mesh::HalfedgeHandle excludeHalfEdge;
	bool exclude = false;
	while (curState.t <= tmax && particlePath.size() < 1000)
	{
		Vec3f field = getOneRingLerpField(curState.p, curState.ownerFace);
		Point next = curState.p + field * dt;

		Triangle triangle(fieldedMesh.getFacePoints(curState.ownerFace));

		if( VectorFieldsUtils::isInnerPoint(next, triangle))
		{
			curState.p = next;
			curState.t = curState.t + dt;

			particlePath.pushBack(next, curState.t);
			//std::cout << "Owner face stayed " << curState.ownerFace << std::endl;
			
			continue;
		}

		// Next we find next owner face. If owner face changed then we need to change next particle point to be on the
		// edge of the new owner face
		if (curState.ownerFace.idx() == 27)
		{
			bool debug = true;
		}
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

			if (!VectorFieldsUtils::intersectionRaySegmentDima(curState.p, field,from, to, normal, intersection)) 
			{
				continue;
			}

			double actualTimeInterval = dt * ( (intersection - curState.p).length() / (next - curState.p).length());
			//std::cout << "Owner face changed from " << curState.ownerFace << " to " << fieldedMesh.opposite_face_handle(cfhei.handle()) << std::endl;
			curState.ownerFace = fieldedMesh.opposite_face_handle(cfhei.handle());
			curState.p = intersection;
			curState.t = curState.t + actualTimeInterval;
			particlePath.pushBack(intersection, curState.t);
			excludeHalfEdge = fieldedMesh.opposite_halfedge_handle(cfhei.handle());
			exclude = true;
			intersectionFound = true;

			if (curState.ownerFace.idx() != -1)
			{
				if (!VectorFieldsUtils::isInnerPoint(curState.p, fieldedMesh.getFacePoints(curState.ownerFace)))
				{
					Vec3f fieldContinuationVec = field.normalized() * (NUMERICAL_ERROR_THRESH * 2);
					curState.p = intersection + VectorFieldsUtils::projectVectorOntoTriangle(fieldContinuationVec, fieldedMesh.getFacePoints(curState.ownerFace));
				}
				if (!VectorFieldsUtils::isInnerPoint(curState.p, fieldedMesh.getFacePoints(curState.ownerFace)))
				{
					Vec3f fieldContinuationVec = field.normalized() * (NUMERICAL_ERROR_THRESH * 4);
					curState.p = intersection - VectorFieldsUtils::projectVectorOntoTriangle(fieldContinuationVec, fieldedMesh.getFacePoints(curState.ownerFace));
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
			if(!VectorFieldsUtils::isInnerPoint(curState.p, triangle))
			{
				fuckupCount++; 
				
			}
			break;
			//throw new std::exception("Intersection was not found");
		}

	}
	return particlePath;
}

ParticleSimStateT PathFinder::particleSimulationStep(const ParticleSimStateT prevState, const Time& timeInterval)
{
	Vec3f field = getOneRingLerpField(prevState.p, prevState.ownerFace);
	Point next = prevState.p + field * timeInterval;

	Triangle triangle(fieldedMesh.getFacePoints(prevState.ownerFace));
	
	if( VectorFieldsUtils::isInnerPoint(next, triangle))
	{
		ParticleSimStateT nextState;
		nextState.ownerFace = prevState.ownerFace;
		nextState.p = next;
		nextState.t = prevState.t + timeInterval;
		return nextState;
	}

	// Next we find next owner face. If owner face changed then we need to change next particle point to be on the
	// edge of the new owner face
	
	Normal& normal = fieldedMesh.calc_face_normal(prevState.ownerFace);
	Point intersection;
	for(Mesh::ConstFaceHalfedgeIter cfhei(fieldedMesh.cfh_begin(prevState.ownerFace)); cfhei != fieldedMesh.cfh_end(prevState.ownerFace); ++cfhei)
	{
		Point& from = fieldedMesh.point(fieldedMesh.from_vertex_handle(cfhei));
		Point& to = fieldedMesh.point(fieldedMesh.to_vertex_handle(cfhei));

		if (!VectorFieldsUtils::intersectionRaySegmentDima(prevState.p, field,from, to, normal, intersection)) 
		{
			continue;
		}

		ParticleSimStateT nextState;
		double actualTimeInterval = timeInterval * ( (intersection - prevState.p).length() / (next - prevState.p).length());
		nextState.ownerFace = fieldedMesh.opposite_face_handle(cfhei.handle());
		nextState.p = intersection;
		nextState.t = prevState.t + actualTimeInterval;
		return nextState;
	}

	return prevState;
}

Vec3f PathFinder::getOneRingLerpField(const Point p, const Mesh::FaceHandle& ownerFace)
{
	vector<std::pair<double, Vec3f>> distanceAndFields;
	distanceAndFields.reserve(10);
	double totalDist(0);

	addDistanceAndField(p, ownerFace, distanceAndFields, totalDist);

	for(Mesh::FFIter curFace = fieldedMesh.ff_begin(ownerFace); curFace != fieldedMesh.ff_end(ownerFace); ++curFace) 
	{
		if (curFace.handle().idx() < 0) 
		{
			continue;
		}
		addDistanceAndField(p, curFace.handle(), distanceAndFields, totalDist);
	}

	if (distanceAndFields.size() == 1)
	{
		return distanceAndFields[0].second;
	}

	Vec3f totalField(0.f);
	for(int i = 0, size = distanceAndFields.size(); i < size; ++i)
	{
		std::pair<double, Vec3f>& current = distanceAndFields[i];
		totalField += current.second * (totalDist - current.first) / totalDist;
	}
	
	// now we cheat by projecting totalField onto ownerFace's plane
	return VectorFieldsUtils::projectVectorOntoTriangle(totalField, fieldedMesh.getFacePoints(ownerFace));
}



void PathFinder::addDistanceAndField(const Point& p, const Mesh::FaceHandle & face, vector<std::pair<double, Vec3f>>& outDistanceAndFields, double& outTotalDistance)
{
	Point faceCentroid = VectorFieldsUtils::getTriangleCentroid(fieldedMesh.getFacePoints(face));
	double currentDistance = (p - faceCentroid).length();
	outTotalDistance += currentDistance;
	outDistanceAndFields.push_back(std::pair<double, Vec3f>(currentDistance, fieldedMesh.faceVectorField(face, 0)));
}


//Mesh::FaceHandle PathFinder::getNextOwnerFace(const Point& prevPoint, const Point& nextPoint, const Mesh::FaceHandle& ownerFace)
//{
//	TriIntersectionDataT theIntersection = VectorFieldsUtils::segmentTriangleIntersect(prevPoint, nextPoint, fieldedMesh.getFacePoints(ownerFace));
//	if(!theIntersection.found) {
//		return ownerFace;
//	}
//	int i = 0;
//	for(Mesh::ConstFaceHalfedgeIter cfhei(fieldedMesh.cfh_begin(ownerFace)); cfhei != fieldedMesh.cfh_end(ownerFace); ++cfhei, ++i)
//	{
//		if (theIntersection.edgeIndex == i) {
//			Mesh::FaceHandle opposite = fieldedMesh.opposite_face_handle(cfhei.handle());
//			return opposite;
//		}
//	}
//	throw std::exception("Invalid edge index in getNextOwnerFace");
//	return ownerFace;
//}
