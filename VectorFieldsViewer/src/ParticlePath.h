#pragma once

#include "VectorFieldsUtils.h"
#include <math.h>

class ParticlePath
{

private:
	vector<Point> points;
	vector<Time> times;
	int particleLoc;
public:

	ParticlePath():
		particleLoc(0)
	{
		points.reserve(500);
		times.reserve(500);
	}

	inline const vector<Point>& getPoints()
	{
		return points;
	}

	inline const vector<Time>& getTimes()
	{
		return times;
	}

	inline const uint size()
	{
		assert(points.size() == times.size());
		return points.size();
	}

	inline void pushBack(const Point& point, const Time& time)
	{
		points.push_back(point);
		times.push_back(time);
		assert(points.size() == times.size());
		assert(times.size() < 2 || times[times.size() - 1] >= times[times.size() - 2]);
	}

	void clear()
	{
		points.clear();
		times.clear();
	}

	const Point* getActivePathPoints(int requestedLength, int* actualLength)
	{
		int firstIndex = particleLoc - requestedLength + 1;
		if (firstIndex < 0) 
		{
			firstIndex = 0;
		}
		*actualLength = particleLoc - firstIndex + 1;
		assert(*actualLength > 0);
		return &points[firstIndex];
	}

	const Time* getActivePathTimes(int requestedLength, int* actualLength)
	{
		int firstIndex = particleLoc - requestedLength;
		if (firstIndex < 0) 
		{
			firstIndex = 0;
		}
		*actualLength = particleLoc - firstIndex + 1;
		assert(*actualLength > 0);
		return &times[firstIndex];
	}

	void evolveParticleLoc(double dt)
	{
		int pathLength = (int) size();
		assert(pathLength > 0 && particleLoc >= 0 && particleLoc < pathLength);
		double currentTime = times[particleLoc];
		if (particleLoc == pathLength - 1)
		{
			particleLoc = rand() % std::max( ((int) (pathLength / 10)),  pathLength);
			return;
		}
		for (int nextLoc = particleLoc; nextLoc < pathLength; nextLoc++)
		{
			if (times[nextLoc] >= currentTime + dt)
			{
				particleLoc = nextLoc;
				assert(particleLoc >= 0);
				return;
			}
		}
		particleLoc = pathLength - 1;
	}
	
	bool isConverged(float pointRadius, float timeRadius = NUMERICAL_ERROR_THRESH*100, int pointsToCheck = 10, Point* convergencePoint = NULL)
	{
		if (pointsToCheck > points.size())
		{
			return false; // Not enough data to deciede convergence
		}
		int last = times.size() - 1;
		if (abs(times[last] - times[last-pointsToCheck]) > timeRadius)
		{
			return false; // Times are not close enough
		}
		Point centroid = Point(0,0,0);
		float totalDist = -(points[last - pointsToCheck - 1] - points[last - pointsToCheck]).length();
		for (int i = last - pointsToCheck; i <= last; i++)
		{
			centroid += points[i];
			totalDist += (points[i] - points[i-1]).length();
			float distanceToCentroid = (points[i] - centroid).length();
			bool debug = true;
		}
		centroid /= pointsToCheck;
		if (NULL != convergencePoint)
		{
			*convergencePoint = centroid;
		}

		// Added this because centroid does not work
		if (totalDist < pointRadius)
		{
			return true;
		}

		// I cannot understand why the code below does not work... Distance to centroid is always a very large number
		for (int i = last - pointsToCheck; i <= last; i++)
		{
			float distanceToCentroid = (points[i] - centroid).length();
			if (distanceToCentroid > pointRadius)
			{
				return false; // A point is too far
			}
		}
		return true;
	}
};

