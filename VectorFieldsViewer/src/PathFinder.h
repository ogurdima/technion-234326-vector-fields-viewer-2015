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

	bool					configure(const FieldedMesh& aMesh_, Time dt_, Time tmin_, Time tmax_);

	vector<ParticlePath>	getParticlePaths();

protected:

	FieldedMesh		fieldedMesh;
	Time			dt;
	Time			tmin;
	Time			tmax;
	bool			hasValidConfig;

	int				pathDepth;

	ParticlePath getParticlePath(const Mesh::FaceHandle& face_);

	ParticleSimStateT particleSimulationStep(const ParticleSimStateT prevState, const Time& timeInterval);
	Vec3f getOneRingLerpField(const Point p, const Mesh::FaceHandle& ownerFace);
	void addDistanceAndField(const Point& p, const Mesh::FaceHandle & face, vector<std::pair<double, Vec3f>>& outDistanceAndFields, double& outTotalDistance);
	//Mesh::FaceHandle getNextOwnerFace(const Point& prevPoint, const Point& nextPoint, const Mesh::FaceHandle& ownerFace);
};

