#pragma once

#include "MeshViewer.h"


typedef OpenMesh::TriMesh_ArrayKernelT<> Mesh;
typedef OpenMesh::FPropHandleT<Vec3f> VfieldFProp;

class PathFinder
{
public:
	PathFinder(Mesh mesh_, VfieldFProp vf_fprop);

	std::vector<std::vector<Vec3f>> getParticlePaths();

protected:

	Mesh mesh;
	VfieldFProp vfield;
	double dt;
	std::vector<Vec3f> getParticlePath(Mesh::FaceHandle face);

};

