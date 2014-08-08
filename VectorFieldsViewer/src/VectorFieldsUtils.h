#pragma once

#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

using namespace OpenMesh;

typedef TriMesh_ArrayKernelT<> Mesh;
typedef FPropHandleT<Vec3f> VfieldFProp;
typedef Mesh::Point Point;


class VectorFieldsUtils
{
public:
	static Point stdToBarycentric(Point original, Point v1, Point v2, Point v3);
	static Point barycentricToStd(Point barycentric, Point v1, Point v2, Point v3);
};

