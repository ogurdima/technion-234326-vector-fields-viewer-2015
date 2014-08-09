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
	vector<Vec3f> vertexPos;
	vector<Vec3f> particlePath;

	Mesh::ConstFaceVertexIter	cfv_it(fieldedMesh.cfv_begin(faceHandle)), cfv_end(fieldedMesh.cfv_end(faceHandle));
	for(; cfv_it != cfv_end; cfv_it++)  {
		vertexPos.push_back(fieldedMesh.point(cfv_it.handle()));
	}
	assert(vertexPos.size() == 3);

	Vec3f pstart = (vertexPos[0] + vertexPos[1] + vertexPos[2]) / 3.0;
	particlePath.push_back(pstart);
	Vec3f plast = pstart;

	

	for (double t = tmin; t <= tmax; t++) {

		float x = VectorFieldsUtils::fRand(-0.1,0.1);
		float y = VectorFieldsUtils::fRand(-0.1,0.1);
		float z = VectorFieldsUtils::fRand(-0.1,0.1);
		Vec3f randVariation = Vec3f(x,y,z);

		Vec3f field = fieldedMesh.faceVectorField(faceHandle, t);
		Vec3f pnext = plast + (randVariation+field) * dt;
		plast = pnext;
		particlePath.push_back(pnext);

		// Add face toggle
	}

	return particlePath;
}


