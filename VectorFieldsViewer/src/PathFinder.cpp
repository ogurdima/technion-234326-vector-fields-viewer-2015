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
	static int count = 0;
	vector<Point> particlePath;
	Triangle pts(fieldedMesh.getFacePoints(faceHandle));
	Vec3f field = fieldedMesh.faceVectorField(faceHandle, 0);
	Point pstart = VectorFieldsUtils::barycentricToStd(Point(1./3.), pts);
	particlePath.push_back(pstart);
	Point plast = pstart;
	bool found(false);
	Mesh::HalfedgeHandle halfEdge;
	
		// find first intersection
	
	for(Mesh::ConstFaceHalfedgeIter cfhei(fieldedMesh.cfh_begin(faceHandle)),  cfheEnd(fieldedMesh.cfh_end(faceHandle)); 
		cfhei != cfheEnd; ++cfhei)
	{
		Mesh::VertexHandle from = fieldedMesh.from_vertex_handle(cfhei);
		Mesh::VertexHandle to = fieldedMesh.to_vertex_handle(cfhei);
 
		if(VectorFieldsUtils::intersectionRaySegmentDima(pstart, field , fieldedMesh.point(from), fieldedMesh.point(to), plast))
		{
			found = true;
			particlePath.push_back(plast);
			halfEdge = cfhei.handle();
			break;
		}
	}

	if(!found)
	{
		throw new std::exception("First intersection wasn't found");
	}


	for(int depth = pathDepth;depth > 0; --depth)
	{
		found = false;
		pstart = plast;
		halfEdge = fieldedMesh.opposite_halfedge_handle(halfEdge);
		Mesh::FaceHandle& oppositeFace = fieldedMesh.face_handle(halfEdge);
		field = fieldedMesh.faceVectorField(oppositeFace, 0);
		

		for(int i = 2; i > 0; --i)
		{
			halfEdge = fieldedMesh.next_halfedge_handle(halfEdge);
			Mesh::VertexHandle from = fieldedMesh.from_vertex_handle(halfEdge);
			Mesh::VertexHandle to = fieldedMesh.to_vertex_handle(halfEdge);
			if(VectorFieldsUtils::intersectionRaySegmentDima(pstart, field , fieldedMesh.point(from), fieldedMesh.point(to), plast))
			{
				found = true;
				particlePath.push_back(plast);
				break;
			}
		}

		if(!found)
			break;
	}

	return particlePath;
}


Point PathFinder::getNextParticlePosition(const Point p, const Mesh::FaceHandle& ownerFace)
{
	Vec3f field = getOneRingLerpField(p, ownerFace);
	Point next = p + field * dt;
	// TODO add face owner change if needed
	return next;
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
	return totalField;
}