#pragma once

#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

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

#define NUMERICAL_ERROR_THRESH				0.000001

class VectorFieldsUtils
{
public:
	static Point stdToBarycentric(const Point& original, const Point& v1, const Point& v2, const Point& v3);
	static Point barycentricToStd(const Point& barycentric, const Point& v1, const Point& v2, const Point& v3);

	static bool isCloseToZero(double val);
	static float fRand(float fMin, float fMax);
};


typedef struct 
{
	Vec3f	f; 
	float	t;
} VectorFieldTimeVal;
