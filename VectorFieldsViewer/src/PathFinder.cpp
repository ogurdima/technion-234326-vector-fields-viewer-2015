#include "PathFinder.h"
#include "FieldedMesh.h"


PathFinder::PathFinder() : 
	dt(0.1),
	tmin(0),
	tmax(1),
	hasValidConfig(false),
	pathDepth(10)
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
	for(Mesh::ConstFaceIter fit(fieldedMesh.faces_begin()), fitEnd(fieldedMesh.faces_end()); fit != fitEnd; ++fit ) 
	{
		ParticlePath facePath = getParticlePath(fit.handle());
		allPaths.push_back(facePath);
	}

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

	while (curState.t <= tmax)
	{
		ParticleSimStateT nextState = particleSimulationStep(curState, dt);
		particlePath.pushBack(nextState.p,nextState.t);
		if(nextState.t == curState.t)
			break;
		curState = nextState;

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
	
	TriIntersectionDataT theIntersection = VectorFieldsUtils::segmentTriangleIntersect(prevState.p, next, triangle);
	if(!theIntersection.found)
	{
		return prevState;
	}

	int i = 0;
	for(Mesh::ConstFaceHalfedgeIter cfhei(fieldedMesh.cfh_begin(prevState.ownerFace)); cfhei != fieldedMesh.cfh_end(prevState.ownerFace); ++cfhei, ++i)
	{
		Point& from = fieldedMesh.point(fieldedMesh.from_vertex_handle(cfhei));
		if (from != triangle[theIntersection.edgeIndex]) 
		{
			continue;
		}
		ParticleSimStateT nextState;
		double actualTimeInterval = timeInterval * ( (theIntersection.p - prevState.p).length() / (next - prevState.p).length());
		nextState.ownerFace = fieldedMesh.opposite_face_handle(cfhei.handle());
		nextState.p = theIntersection.p;
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

Mesh::FaceHandle PathFinder::getNextOwnerFace(const Point& prevPoint, const Point& nextPoint, const Mesh::FaceHandle& ownerFace)
{
	TriIntersectionDataT theIntersection = VectorFieldsUtils::segmentTriangleIntersect(prevPoint, nextPoint, fieldedMesh.getFacePoints(ownerFace));
	if(!theIntersection.found) {
		return ownerFace;
	}
	int i = 0;
	for(Mesh::ConstFaceHalfedgeIter cfhei(fieldedMesh.cfh_begin(ownerFace)); cfhei != fieldedMesh.cfh_end(ownerFace); ++cfhei, ++i)
	{
		if (theIntersection.edgeIndex == i) {
			Mesh::FaceHandle opposite = fieldedMesh.opposite_face_handle(cfhei.handle());
			return opposite;
		}
	}
	throw std::exception("Invalid edge index in getNextOwnerFace");
	return ownerFace;
}
