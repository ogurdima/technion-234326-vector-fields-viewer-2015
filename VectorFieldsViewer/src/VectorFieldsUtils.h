#pragma once

#pragma region include

#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#pragma endregion

#pragma region using

using OpenMesh::Vec3f;
using OpenMesh::TriMesh_ArrayKernelT;
using OpenMesh::FPropHandleT;

using std::vector;

#pragma endregion

#pragma region typedef

typedef TriMesh_ArrayKernelT<> Mesh;
typedef FPropHandleT<Vec3f> VfieldFProp;
typedef Mesh::Point Point;

#pragma endregion


class VectorFieldsUtils
{
public:
	static Point stdToBarycentric(Point original, Point v1, Point v2, Point v3);
	static Point barycentricToStd(Point barycentric, Point v1, Point v2, Point v3);
};

