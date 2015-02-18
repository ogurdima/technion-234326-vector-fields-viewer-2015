#pragma once

#include "VectorFieldsUtils.h"
#include "FieldedMesh.h"
#include "ParticlePath.h"

typedef struct
{
	Time t;
	Point p;
	Mesh::FaceHandle ownerFace;
}  ParticleSimStateT;

class PathFinder
{
public:
	PathFinder();

	bool					configure(const FieldedMesh& aMesh_, const Time& dt_);

	vector<ParticlePath>	getParticlePaths();

protected:

	FieldedMesh		fieldedMesh;
	Time			dt;
	Time			tmin;
	Time			tmax;
	bool			hasValidConfig;

	int				pathDepth;
	int				fuckupCount;

	// cache
	vector<Triangle>						triangles;
	vector<Point>							centroids;
	vector<vector<int>>						oneRingFaceIds;
	vector<vector<VectorFieldTimeVal>>		faceFields;
	vector<Normal>							normals;

	vector<OpenMesh::VectorT<Point,3>>		faceVertices;
	vector<OpenMesh::VectorT<Vec3f,3>>		faceVertexFields;
	


	ParticlePath getParticlePath(const Mesh::FaceHandle& face_);

	Vec3f getOneRingLerpField(const Point& p, const int ownerIdx, const Time time);
	Vec3f getField(const Point& p, int fid);
};

