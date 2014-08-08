#include "PathFinder.h"
#include "FieldedMesh.h"


PathFinder::PathFinder(const FieldedMesh& fieldedMesh_) : 
	fieldedMesh(fieldedMesh_),
	dt(0.005)
{
	
}

std::vector<std::vector<Vec3f>> PathFinder::getParticlePaths()
{
	std::vector<std::vector<Vec3f>> allPaths;
	for(Mesh::FaceIter fit = fieldedMesh.faces_begin(); fit != fieldedMesh.faces_end(); ++fit) 
	{
		std::vector<Vec3f> facePath = getParticlePath(fit.handle());
		allPaths.push_back(facePath);
	}

	return allPaths;
}

std::vector<Vec3f> PathFinder::getParticlePath(const Mesh::FaceHandle& faceHandle)
{
	std::vector<Vec3f> vertexPos;
	std::vector<Vec3f> particlePath;

	Mesh::ConstFaceVertexIter	cfv_it(fieldedMesh.cfv_begin(faceHandle)), cfv_end(fieldedMesh.cfv_end(faceHandle));

	for(; cfv_it != cfv_end; cfv_it++)  {
		vertexPos.push_back(fieldedMesh.point(cfv_it.handle()));
	}
	assert(vertexPos.size() == 3);
	Vec3f pstart = (vertexPos[0] + vertexPos[1] + vertexPos[2]) / 3.0;

	Vec3f pnext = pstart * (/* fieldedMesh.property(vectorfi, faceHandle) */ (float)dt);

	particlePath.push_back(pstart);
	particlePath.push_back(pnext);
	
	return particlePath;
}


