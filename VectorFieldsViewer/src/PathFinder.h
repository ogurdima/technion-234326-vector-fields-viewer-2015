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
	void												cleareCache();
	ParticlePath										getParticlePath(Mesh::FaceHandle& face_);
	Vec3f												getField(const Point& p,const int fid, const Time time);
	bool												configure(const FieldedMesh& aMesh_, const Time& dt_, const Time& minTime, const Time& maxTime);
};

