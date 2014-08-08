#include "PathFinder.h"


PathFinder::PathFinder(Mesh mesh_, VfieldFProp vf_fprop)
{
	mesh = mesh_;
	vfield = vf_fprop;
	dt = 100;
}



std::vector<std::vector<Vec3f>> PathFinder::getParticlePaths()
{
	std::vector<std::vector<Vec3f>> allPaths;
	for(Mesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit) 
	{
		std::vector<Vec3f> facePath = getParticlePath(fit.handle());
		allPaths.push_back(facePath);
	}

	return allPaths;
}

std::vector<Vec3f> PathFinder::getParticlePath(Mesh::FaceHandle face)
{
	std::vector<Vec3f> vertexPos;
	std::vector<Vec3f> particlePath;
	for(Mesh::FVIter vit = mesh.fv_begin(face); vit != mesh.fv_end(face); ++vit)  {
		vertexPos.push_back(mesh.point(vit.handle()));
	}
	assert(vertexPos.size() == 3);
	Vec3f pstart = (vertexPos[0] + vertexPos[1] + vertexPos[2]) / 3.0;

	Vec3f pnext = pstart + ( mesh.property(vfield, face) * (float)dt);

	particlePath.push_back(pstart);
	particlePath.push_back(pnext);
	

	return particlePath;
}