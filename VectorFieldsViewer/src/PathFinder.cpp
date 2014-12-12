#include "PathFinder.h"
#include "FieldedMesh.h"


PathFinder::PathFinder() : 
	dt(0.1),
	tmin(0),
	tmax(1),
	hasValidConfig(false)
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
	static int count = 0;
	vector<Point> particlePath;
	Triangle pts(fieldedMesh.getFacePoints(faceHandle));
	Vec3f field = fieldedMesh.faceVectorField(faceHandle, 0);
	Point pstart = VectorFieldsUtils::barycentricToStd(Point(1./3.), pts);
	particlePath.push_back(pstart);
	Point plast = pstart;
	bool found(false);
	Mesh::HalfedgeHandle halfEdge;
	
	for(int i = 0; i < 3; ++i)
	{
		if(VectorFieldsUtils::intersectionRaySegmentDima(pstart, field , pts[i], pts[(i + 1) % 3], plast))
		{
			found = true;
			break;
		}
	}
	if(found)
	{
		particlePath.push_back(plast);
	}
	else
	{
		throw new std::exception("Intersection wasn't found!!! AAAA");
	}
	return particlePath;
}


