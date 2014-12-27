#include "VectorFieldsUtils.h"

Point VectorFieldsUtils::stdToBarycentric(const Point& original, const Triangle& triangle) 
{
    Vec3f e1 = triangle[1] - triangle[0] , e2 = triangle[2] - triangle[0], v1ToOriginal = original - triangle[0];

    float d00 = e1 | e1;
    float d01 = e1 | e2;
    float d11 = e2 | e2;
    float d20 = v1ToOriginal | e1;
    float d21 = v1ToOriginal | e2;
    float denom = d00 * d11 - d01 * d01;
	Point p;
    p[0] = (d11 * d20 - d01 * d21) / denom;
    p[1]= (d00 * d21 - d01 * d20) / denom;
    p[2] = 1.0f - p[0] - p[1];
	return p;
}

//Point VectorFieldsUtils::stdToBarycentric(const Point& original, const Point& v1, const Point& v2, const Point& v3) 
//{
//    Vec3f e1 = v2 - v1 , e2 = v3 - v1, v1ToOriginal = original - v1;
//
//    float d00 = e1 | e1;
//    float d01 = e1 | e2;
//    float d11 = e2 | e2;
//    float d20 = v1ToOriginal | e1;
//    float d21 = v1ToOriginal | e2;
//    float denom = d00 * d11 - d01 * d01;
//	Point p;
//    p[0] = (d11 * d20 - d01 * d21) / denom;
//    p[1]= (d00 * d21 - d01 * d20) / denom;
//    p[2] = 1.0f - p[0] - p[1];
//	return p;
//}
//
//
//Point VectorFieldsUtils::barycentricToStd(const Point& barycentric, const Point& v1, const Point& v2, const Point& v3) 
//{
//	return v1 * barycentric[0] + v2 * barycentric[1] + v3 * barycentric[2];
//}

Point VectorFieldsUtils::barycentricToStd(const Point& barycentric, const Triangle& triangle) 
{
	return triangle[0] * barycentric[0] + triangle[1] * barycentric[1] + triangle[2] * barycentric[2];
}

bool VectorFieldsUtils::isInnerPoint(const Point& point, const Triangle& triangle) 
{
	Point& barycentric = stdToBarycentric(point, triangle);
	return (barycentric[0] >= 0) && (barycentric[1] >= 0) && (barycentric[2] >= 0);
	
}


bool VectorFieldsUtils::isCloseToZero(double val)
{
	return abs(val) < NUMERICAL_ERROR_THRESH;
}

float VectorFieldsUtils::fRand(float fMin, float fMax)
{
    float f = (float)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}


Vec3f VectorFieldsUtils::lerp(Vec3f first, Vec3f second, double time) {
	assert(time <= 1.0);
	return (first * (1.0 - time)) + (second * time);
}

Point VectorFieldsUtils::getTriangleCentroid(Triangle t)
{
	return (t[0] + t[1] + t[2]) / 3.0;
}

TriIntersectionDataT VectorFieldsUtils::segmentTriangleIntersect(const Point& segA, const Point& segB, const Triangle& tri)
{
	Vec3f ray = (segB - segA); // from A to B
	Point intersectionPoint;
	bool foundRayEdgeIntersect = false;
	TriIntersectionDataT theIntersection;
	theIntersection.found = false;
	for (int fromIndex = 0; fromIndex < 3; fromIndex++)
	{
		int toIndex = (fromIndex + 1)%3;
		Point from = tri[fromIndex];
		Point to = tri[toIndex];
		if(intersectionRaySegmentDima(segA, ray , from, to, intersectionPoint))
		{
			foundRayEdgeIntersect = true;
			if (ray.length() >= (intersectionPoint - segA).length()) {
				// This means segment segA,segB intersects triangle edge, not just ray				
				theIntersection.edgeIndex = fromIndex;
				theIntersection.p = intersectionPoint;
				theIntersection.found = true;
				return theIntersection;
			}
		}
	}
	//assert(foundRayEdgeIntersect);
	if (!foundRayEdgeIntersect) {
		bool OK = false;
	}
	return theIntersection;
}

Vec3f VectorFieldsUtils::getTriangleNormal(Triangle t)
{
	return ((t[1] - t[0]) % (t[2] - t[1])).normalized();
}

Vec3f VectorFieldsUtils::projectVectorOntoTriangle(Vec3f v, Triangle t)
{
	Vec3f n = getTriangleNormal(t);
	Vec3f onPlanePerpToV = v % n;
	Vec3f onPlaneClosestToV = n % onPlanePerpToV;
	return onPlaneClosestToV; // TODO: check lengths, maybe need to normalize one step
}