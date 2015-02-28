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

Point VectorFieldsUtils::barycentricToStd(const Point& barycentric, const Triangle& triangle) 
{
	return triangle[0] * barycentric[0] + triangle[1] * barycentric[1] + triangle[2] * barycentric[2];
}

bool VectorFieldsUtils::isInnerPoint(const Point& point, const Triangle& triangle) 
{
	Point barycentric(stdToBarycentric(point, triangle));
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

Vec3f VectorFieldsUtils::lerp(const Vec3f& first, const Vec3f& second, const Time& time) 
{
	assert(time <= 1.0 && time >= 0.0);
	return (first * (1. - time)) + (second * (float)time);
}

Point VectorFieldsUtils::getTriangleCentroid(const Triangle& t)
{
	return (t[0] + t[1] + t[2]) / 3.0;
}

Vec3f VectorFieldsUtils::getTriangleNormal(const Triangle& t)
{
	return ((t[1] - t[0]) % (t[2] - t[1])).normalized();
}

Vec3f VectorFieldsUtils::projectVectorOntoTriangle(const Vec3f& v, const Normal& n)
{
	Vec3f onPlanePerpToV = v % n;
	Vec3f onPlaneClosestToV = n % onPlanePerpToV;
	return onPlaneClosestToV; // TODO: check lengths, maybe need to normalize one step
}

Vec3f VectorFieldsUtils::calculateField(const vector<VectorFieldTimeVal>& fieldSamples, const Time& time)
{
	uint size = fieldSamples.size();
	if (size == 0) // no field
	{
		return Vec3f(0,0,0);
	}
	if( time <= fieldSamples[0].time) // smaller time
	{
		return fieldSamples[0].field;
	}
	if(time >= fieldSamples[size - 1].time) // bigger time
	{
		return fieldSamples[size - 1].field;
	}
	uint i = 1;
	for (; i < size; i++) // can be optimized with binary search
	{
		if ( time <= fieldSamples[i].time) 
		{
			break;
		}
	}
	assert(i < size);
	// fieldSamples[i - 1].time < time && time <= fieldSamples[i].time
	assert(fieldSamples[i - 1].time < time && time <= fieldSamples[i].time);

	// todo: add interpolation by angle and length
	return VectorFieldsUtils::lerp(fieldSamples[i-1].field, fieldSamples[i].field,
		(time - fieldSamples[i - 1].time) / (fieldSamples[i].time - fieldSamples[i - 1].time));

}
