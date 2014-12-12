#pragma once

#include "VectorFieldsUtils.h"
#include "FieldedMesh.h"


class PathFinder
{
public:
	PathFinder();

	bool					configure(const FieldedMesh& aMesh_, double dt_, double tmin_, double tmax_);

	vector<vector<Vec3f>>	getParticlePaths();

protected:

	FieldedMesh		fieldedMesh;
	double			dt;
	double			tmin;
	double			tmax;
	bool			hasValidConfig;

	int				pathDepth;

	vector<Vec3f> getParticlePath(const Mesh::FaceHandle& face_);
};

