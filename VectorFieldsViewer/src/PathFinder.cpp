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

bool PathFinder::configure(const FieldedMesh& aMesh_, double dt_, double tmin_, double tmax_)
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

vector<vector<Vec3f>> PathFinder::getParticlePaths()
{
	if (!hasValidConfig) {
		throw std::exception("Calling getParticlePaths with badly configured PathFinder");
	}

	vector<vector<Vec3f>> allPaths;
	for(Mesh::ConstFaceIter fit(fieldedMesh.faces_begin()), fitEnd(fieldedMesh.faces_end()); fit != fitEnd; ++fit ) 
	{
		vector<Vec3f> facePath = getParticlePath(fit.handle());
		allPaths.push_back(facePath);
	}

	return allPaths;
}

vector<Vec3f> PathFinder::getParticlePath(const Mesh::FaceHandle& faceHandle)
{
	vector<Point> particlePath;
	Triangle pts(fieldedMesh.getFacePoints(faceHandle));
	Vec3f field = fieldedMesh.faceVectorField(faceHandle, 0);
	Point pstart = VectorFieldsUtils::barycentricToStd(Point(1./3.), pts);
	particlePath.push_back(pstart);
	Point last = pstart;
	Mesh::FaceHandle ownerFace = faceHandle;

	ParticleSimStateT curState;
	curState.ownerFace = faceHandle;
	curState.p = pstart;
	curState.t = tmin;

	while (curState.t <= tmax) {
		ParticleSimStateT nextState = particleSimulationStep(curState, dt);
		particlePath.push_back(nextState.p);
		curState = nextState;
	}
	return particlePath;
}


ParticleSimStateT PathFinder::particleSimulationStep(const ParticleSimStateT prevState, double timeInterval)
{
	Vec3f field = getOneRingLerpField(prevState.p, prevState.ownerFace);
	Point next = prevState.p + field * timeInterval;
	Mesh::FaceHandle nextOwnerFace;

	// Next we find next owner face. If owner face changed then we need to change next particle point to be on the
	// edge of the new owner face
	TriIntersectionDataT theIntersection = VectorFieldsUtils::segmentTriangleIntersect(prevState.p, next, fieldedMesh.getFacePoints(prevState.ownerFace));
	if(!theIntersection.found) {
		nextOwnerFace = prevState.ownerFace;
	}
	else {
		int i = 0;
		for(Mesh::ConstFaceHalfedgeIter cfhei(fieldedMesh.cfh_begin(prevState.ownerFace)); cfhei != fieldedMesh.cfh_end(prevState.ownerFace); ++cfhei, ++i)
		{
			if (theIntersection.edgeIndex == i) {
				Mesh::FaceHandle opposite = fieldedMesh.opposite_face_handle(cfhei.handle());
				nextOwnerFace = opposite;
				break;
			}
		}
	}

	ParticleSimStateT nextState;
	if (nextOwnerFace == prevState.ownerFace) {
		nextState.ownerFace = prevState.ownerFace;
		nextState.p = next;
		nextState.t = prevState.t + timeInterval;
		return nextState;
	}
	// Here we know ownerFace changed
	double actualTimeInterval = timeInterval * ( (theIntersection.p - prevState.p).length() / (next - prevState.p).length() );
	nextState.ownerFace = nextOwnerFace;
	nextState.p = theIntersection.p;
	nextState.t = prevState.t + actualTimeInterval;
	return nextState;
}

Vec3f PathFinder::getOneRingLerpField(const Point p, const Mesh::FaceHandle& ownerFace)
{
	Vec3f totalField = Vec3f(0,0,0);
	double totalDist = 0;
	Point faceCentroid = VectorFieldsUtils::getTriangleCentroid(fieldedMesh.getFacePoints(ownerFace));
	totalDist = (p - faceCentroid).length();
	for(Mesh::FFIter curFace = fieldedMesh.ff_begin(ownerFace); curFace != fieldedMesh.ff_end(ownerFace); curFace++) {
		Vec3f faceField = fieldedMesh.faceVectorField(curFace.handle(), 0);
		Point faceCentroid = VectorFieldsUtils::getTriangleCentroid(fieldedMesh.getFacePoints(curFace.handle()));
		double dist = (p - faceCentroid).length();
		totalField = VectorFieldsUtils::lerp(totalField, faceField, dist / (totalDist + dist));
		totalDist += dist;
	}
	// now we cheat by projecting totalField onto ownerFace's plane
	totalField = VectorFieldsUtils::projectVectorOntoTriangle(totalField, fieldedMesh.getFacePoints(ownerFace));
	return totalField;
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
