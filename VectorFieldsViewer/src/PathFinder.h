#pragma once

#include "VectorFieldsUtils.h"
#include "FieldedMesh.h"
#include "ParticlePath.h"
#include <math.h>

using OpenMesh::VectorT;

class PathFinder
{
public:
	PathFinder();
	vector<ParticlePath>								getParticlePaths(const FieldedMesh& aMesh_, const Time& dt_, const Time& minTime, const Time& maxTime);

private:

	FieldedMesh											fieldedMesh;
	Time												dt;
	Time												tmin;
	Time												tmax;
	bool												hasValidConfig;
	int													fuckupCount;

	// cache
	vector<Triangle>									triangles;
	vector<Point>										centroids;
	vector<Normal>										normals;
	vector<VectorT<vector<VectorFieldTimeVal>,3>>		faceVertexFields;

	void												cache();
	void												clearCache();
	ParticlePath										getParticlePath(Mesh::FaceHandle& face_);
	inline Vec3f										getField(const Point& p,const int fid, const Time time);
	bool												configure(const FieldedMesh& aMesh_, const Time& dt_, const Time& minTime, const Time& maxTime);
	inline bool											getIntersection(Mesh::FaceHandle& currentFace, Point& currentPoint, Time& currentTime, Time timeDelta);
};

bool PathFinder::getIntersection(Mesh::FaceHandle& currentFace, Point& currentPoint, Time& currentTime, Time timeDelta)
{
	const Triangle& currentTriangle = triangles[currentFace.idx()];
	Vec3f field = getField(currentPoint, currentFace.idx(), currentTime);
	if (VectorFieldsUtils::isCloseToZero(field.length()))
	{
		// The field is ridiculously small, we will not move from here. 
		// Increase curTime and check maybe field changed (continue)
		currentTime += timeDelta;
		return true;
	}

	Point next = currentPoint + field * timeDelta;
	// The next point is in the same face as the last. 
	// last = next; continue
	if( VectorFieldsUtils::isInnerPoint(next, currentTriangle))
	{
		currentPoint = next;
		currentTime += timeDelta;
		return true;
	}

	// Next we find next owner face. The owner face changed so we need to change next particle point to be on the
	// edge of the new owner face
	const Normal& normal = normals[currentFace.idx()];

	for(Mesh::ConstFaceHalfedgeIter cfhei(fieldedMesh.cfh_begin(currentFace)); cfhei != fieldedMesh.cfh_end(currentFace); ++cfhei)
	{
		Point& from = fieldedMesh.point(fieldedMesh.from_vertex_handle(cfhei));
		Point& to = fieldedMesh.point(fieldedMesh.to_vertex_handle(cfhei));
		Point intersection;
		if (!VectorFieldsUtils::intersectionRaySegment(currentPoint, field, from, to, normal, intersection)) 
		{ continue;	}
		currentFace = fieldedMesh.opposite_face_handle(cfhei.handle());
		if (currentFace.idx() == -1)
		{
			// We reached boundary - need to stop computation here
			return false;
		}

		Triangle& t = triangles[currentFace.idx()];

		// Here we move the intersection point a little bit to the center of the new ownerFace
		// This is needed to reduce possibility of numeric instability
		int ccc = 1;
		Point nextPoint;
		Vec3f delta = (centroids[currentFace.idx()] - intersection).normalized() * NUMERICAL_ERROR_THRESH;
		do
		{
			nextPoint = intersection + delta * ccc++;
		}
		while (!VectorFieldsUtils::isInnerPoint(nextPoint, t) && ccc < 10);
		Time tmpTime = currentTime + (Time)((intersection - currentPoint).length() / field.length());
		if (_isnan(tmpTime) || !_finite(tmpTime))
		{
			bool debug = true;
		}
		currentTime = tmpTime;
		currentPoint = nextPoint;
		return true;
	}
	fuckupCount++; 
	return false;
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