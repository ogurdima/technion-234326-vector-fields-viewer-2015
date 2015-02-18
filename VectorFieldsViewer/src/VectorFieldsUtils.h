#pragma once

#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#pragma region using

using OpenMesh::Vec3f;
using OpenMesh::TriMesh_ArrayKernelT;
using OpenMesh::FPropHandleT;
using OpenMesh::AttribKernelT;

using std::vector;

#pragma endregion

#pragma region typedef



#define BEGINNING_OF_TIME -DBL_MAX
typedef float Time;

typedef TriMesh_ArrayKernelT<> Mesh;
typedef FPropHandleT<Vec3f> VfieldFProp;
typedef Mesh::Point Point;
typedef Mesh::Normal Normal;

typedef OpenMesh::VectorT<Point,3> Triangle;

#pragma endregion

#define NUMERICAL_ERROR_THRESH				DBL_EPSILON * 10



class VectorFieldTimeVal 
{
public:
	VectorFieldTimeVal(const Vec3f& _field, const Time& _time) :
		field(_field), time(_time)
	{}
	Vec3f	field;
	Time	time;
};

//class VectorField
//{
//	vector<VectorFieldTimeVal> samples;
//public:
//	VectorField()
//	{}
//};

class VectorFieldsUtils
{
public:
	//static Point stdToBarycentric(const Point& original, const Point& v1, const Point& v2, const Point& v3);
	//static Point barycentricToStd(const Point& barycentric, const Point& v1, const Point& v2, const Point& v3);

	static Point stdToBarycentric(const Point& original, const Triangle& triangle);
	static Point barycentricToStd(const Point& original, const Triangle& triangle);
	static bool isInnerPoint(const Point& original, const Triangle& triangle);

	static bool isCloseToZero(double val);
	static float fRand(float fMin, float fMax);

	static bool intersectionRaySegment(const Point& start, const Vec3f& field, const Point& v1, const Point& v2, Point& intersection)
	{
		// field 21 (end - start)
		if(field.length() < NUMERICAL_ERROR_THRESH)
			return false;

		Vec3f segmentRay = v2 - v1; // 43 
		float segmentRayLength = segmentRay.length();

		if(segmentRayLength < NUMERICAL_ERROR_THRESH)
			return false;

		

		Vec3f v1Start = v1 - start; // 13 

		float dotV1StartSegmentRay = dot(v1Start, segmentRay);			// d1343
		float dotSegmentRayField = dot(segmentRay, field);				// d4321
		float dotV1StartField = dot(v1Start, field);					// d1321
		float segmentRayLengthSqr = segmentRayLength * segmentRayLength;		// d4343
		float fieldLengthSqr = dot(field, field);						// d2121

		float denom = fieldLengthSqr * segmentRayLengthSqr - dotSegmentRayField * dotSegmentRayField;
		if(abs(denom) < NUMERICAL_ERROR_THRESH)
			return false;

		float numer = dotV1StartSegmentRay * dotSegmentRayField - dotV1StartField * segmentRayLengthSqr;

		float fieldTime = numer / denom;

		if(fieldTime < 0) 
			return false;

		float segmentTime = (dotV1StartSegmentRay + dotSegmentRayField * fieldTime) / (segmentRayLength * segmentRayLength);

		if(segmentTime < 0 || segmentTime > 1) 
			return false;
		
		intersection = start + field * fieldTime;
		return true;
	}

	static bool intersectionRaySegmentDima(
		const Point& start, 
		const Vec3f& field, 
		const Point& v1, 
		const Point& v2, 
		const Normal& normal, 
		Point& intersection)
	{
		if(field.length() < NUMERICAL_ERROR_THRESH)
			return false;

		Vec3f segment = v2 - v1;
		float segmentLength = segment.length();

		if(segmentLength < NUMERICAL_ERROR_THRESH)
			return false;

		Vec3f a = start - v1;

		Vec3f u = (segment % normal).normalize();
		

		float denom = dot(field, u);

		if(abs(denom) < NUMERICAL_ERROR_THRESH)
			return false;
		

		//assert(dot(a,u) < 0);

		float fieldTime = - dot(a,u) / denom;

		if(fieldTime < 0) 
			return false;
		
		float segmentTime = (fieldTime * dot(field, segment) + dot(a, segment)) / (segmentLength * segmentLength);

		if(segmentTime < 0 || segmentTime > 1) 
			return false;

		intersection = v1 + segment * segmentTime;
		return true;

	}


	
	static Vec3f lerp(const Vec3f& first, const Vec3f& second, const Time& time);

	static Point getTriangleCentroid(const Triangle& t);

	//static TriIntersectionDataT segmentTriangleIntersect(const Point& segA, const Point& segB, const Triangle& tri);

	static Vec3f getTriangleNormal(const Triangle& t);

	static Vec3f projectVectorOntoTriangle(const Vec3f& v, const Normal& n);

	static Vec3f calculateField(const vector<VectorFieldTimeVal>& fieldSamples, const Time& time);

	template<class T>
	static inline T intepolate(const Point& barycentric, const OpenMesh::VectorT<T, 3>& items)
	{
		return items[0] * barycentric[0] + items[1] * barycentric[1] + items[2] * barycentric[2];
	}

	
};
