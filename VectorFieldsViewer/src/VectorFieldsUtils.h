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

typedef OpenMesh::VectorT<Point,3> Triangle;

#pragma endregion

#define NUMERICAL_ERROR_THRESH				DBL_EPSILON * 10

class VectorFieldsUtils
{
public:
	//static Point stdToBarycentric(const Point& original, const Point& v1, const Point& v2, const Point& v3);
	//static Point barycentricToStd(const Point& barycentric, const Point& v1, const Point& v2, const Point& v3);

	static Point stdToBarycentric(const Point& original, const Triangle& triangle);
	static Point barycentricToStd(const Point& original, const Triangle& triangle);

	static bool isCloseToZero(double val);
	static float fRand(float fMin, float fMax);

	static bool intersectionRaySegment(const Point& start, const Vec3f& field, const Point& v1, const Point& v2, Point& intersection)
	{
		// field 21 
		if(field.length() < NUMERICAL_ERROR_THRESH)
			return false;

		Vec3f segmentRay = v2 - v1; // 43 
		
		if(segmentRay.length() < NUMERICAL_ERROR_THRESH)
			return false;

		Vec3f v1Start = v1 - start; // 13 


		float dotV1StartSegmentRay = dot(v1Start, segmentRay);	   // d1343
		float dotSegmentRayField = dot(segmentRay, field);		   // d4321
		float dotV1StartField = dot(v1Start, field);		   // d1321
		float segmentRayLengthSqr = dot(segmentRay, segmentRay);		   // d4343
		float fieldLengthSqr = dot(field, field);			   // d2121

		float denom = fieldLengthSqr * segmentRayLengthSqr - dotSegmentRayField * dotSegmentRayField;
		if(abs(denom) < NUMERICAL_ERROR_THRESH)
			return false;

		float numer = dotV1StartSegmentRay * dotSegmentRayField - dotV1StartField * segmentRayLengthSqr;

		float fieldTime = numer / denom;

		if(fieldTime < 0) 
			return false;

		float segmentTime = (dotV1StartSegmentRay + dotSegmentRayField * fieldTime) / segmentRayLengthSqr;

		if(segmentTime < 0 || segmentTime > 1) 
			return false;
		

		intersection = start + field * fieldTime;
		return true;
		/*
		p21 = field;
		p13 = vs


		denom = d2121 * d4343 - d4321 * d4321;
		if (ABS(denom) < EPS)
		return(FALSE);
		numer = d1343 * d4321 - d1321 * d4343;

		*mua = numer / denom;
		*mub = (d1343 + d4321 * (*mua)) / d4343;

		pa->x = p1.x + *mua * p21.x;
		pa->y = p1.y + *mua * p21.y;
		pa->z = p1.z + *mua * p21.z;
		pb->x = p3.x + *mub * p43.x;
		pb->y = p3.y + *mub * p43.y;
		pb->z = p3.z + *mub * p43.z;

		return(TRUE);*/

	}
};


typedef struct 
{
	Vec3f	f; 
	float	t;
} VectorFieldTimeVal;
