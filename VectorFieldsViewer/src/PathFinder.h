#pragma once

#include "VectorFieldsUtils.h"
#include "FieldedMesh.h"

typedef struct
{
	double t;
	Point p;
	Mesh::FaceHandle ownerFace;
}  ParticleSimStateT;

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

	ParticleSimStateT particleSimulationStep(const ParticleSimStateT prevState, const double& timeInterval);
	Vec3f getOneRingLerpField(const Point p, const Mesh::FaceHandle& ownerFace);
	void addDistanceAndField(const Point& p, const Mesh::FaceHandle & face, vector<std::pair<double, Vec3f>>& outDistanceAndFields, double& outTotalDistance);
	Mesh::FaceHandle getNextOwnerFace(const Point& prevPoint, const Point& nextPoint, const Mesh::FaceHandle& ownerFace);
};

