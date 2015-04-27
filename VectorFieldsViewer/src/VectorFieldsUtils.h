#pragma once

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#pragma region using

using OpenMesh::Vec3f;
using OpenMesh::Vec4f;
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

#define NUMERICAL_ERROR_THRESH				FLT_EPSILON * 10



class VectorFieldTimeVal 
{
public:
	VectorFieldTimeVal(const Vec3f& _field, const Time& _time) :
		field(_field), time(_time)
	{}
	Vec3f	field;
	Time	time;
};

class VectorFieldsUtils
{
public:

	static Point stdToBarycentric(const Point& original, const Triangle& triangle);
	
	
	static inline Point barycentricToStd(const Point& barycentric, const Triangle& triangle)
	{
		return triangle[0] * barycentric[0] + triangle[1] * barycentric[1] + triangle[2] * barycentric[2];
	}

	static inline bool isInnerPoint(const Point& original, const Triangle& triangle)
	{
		Point barycentric(stdToBarycentric(original, triangle));
		return (barycentric[0] >= 0) && (barycentric[1] >= 0) && (barycentric[2] >= 0);	
	}

	static inline bool isCloseToZero(double val) { return abs(val) < NUMERICAL_ERROR_THRESH; }

	static inline float fRand(float fMin, float fMax) { return fMin + ((float)rand() / RAND_MAX) * (fMax - fMin); }

	static inline bool intersectionRaySegment(const Point& start, const Vec3f& f, const Point& v1, const Point& v2, const Normal& n, Point& intersection)
	{
		if(f.length() < NUMERICAL_ERROR_THRESH) return false;

		Vec3f segment = v2 - v1;
		float segmentLength = segment.length();
		if(segmentLength < NUMERICAL_ERROR_THRESH) return false;

		Vec3f a = start - v1;
		Vec3f u = (segment % (segment % a)).normalize();

		float denom = dot(f, u);
		if(abs(denom) < NUMERICAL_ERROR_THRESH) return false;

		float fieldTime = - dot(a,u) / denom;
		if(fieldTime <= 0) return false;

		float segmentTime = (fieldTime * dot(f, segment) + dot(a, segment)) / (segmentLength * segmentLength);
		if(segmentTime < -0.001 || segmentTime > 1.001) return false;

		intersection = v1 + segment * segmentTime;
		if (_isnan(intersection[0]) || !_finite(intersection[0]))
		{
			bool debug = true;
			return false;
		}
		return true;
	}

	static inline Vec3f lerp(const Vec3f& first, const Vec3f& second, const Time& time)
	{
		assert(time <= 1.0 && time >= 0.0);
		return (first * (1. - time)) + (second * (float)time);
	}

	static inline Vec4f lerp(const Vec4f& first, const Vec4f& second, const Time& time)
	{
		assert(time <= 1.0 && time >= 0.0);
		return (first * (1. - time)) + (second * (float)time);
	}

	static inline Point getTriangleCentroid(const Triangle& t) { return (t[0] + t[1] + t[2]) / 3.0;}

	static inline float getPerimeter(const Triangle& t) { return (t[1] - t[0]).length() + (t[2] - t[1]).length() + (t[0] - t[2]).length(); }

	static inline Vec3f getTriangleNormal(const Triangle& t) { return ((t[1] - t[0]) % (t[2] - t[1])).normalized(); }

	static inline Vec3f projectVectorOntoTriangle(const Vec3f& v, const Normal& n) { return n % (v % n); }

	static Vec3f calculateField(const vector<VectorFieldTimeVal>& fieldSamples, const Time& time);

	template<class T>
	static inline T intepolate(const Point& barycentric, const OpenMesh::VectorT<T, 3>& items)
	{
		return items[0] * barycentric[0] + items[1] * barycentric[1] + items[2] * barycentric[2];
	}

	template<class T>
	static inline T intepolate(const Point& barycentric, const T& item0, const T& item1, const T& item2)
	{
		return item0 * barycentric[0] + item1 * barycentric[1] + item2 * barycentric[2];
	}

	static inline Vec3f average(const vector<Vec3f>& values)
	{
		Vec3f sum = Vec3f(0,0,0);
		uint size = values.size();
		if(size == 0)
			return sum;
		for (uint count = 0; count < values.size(); count++)
		{
			sum += values[count];
		}
		return sum / size;
	}

};
