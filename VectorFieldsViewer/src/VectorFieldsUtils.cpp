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
	assert(fieldSamples[i - 1].time < time && time <= fieldSamples[i].time);

	// todo: add interpolation by angle and length
	return VectorFieldsUtils::lerp(fieldSamples[i-1].field, fieldSamples[i].field,
		(time - fieldSamples[i - 1].time) / (fieldSamples[i].time - fieldSamples[i - 1].time));

}
