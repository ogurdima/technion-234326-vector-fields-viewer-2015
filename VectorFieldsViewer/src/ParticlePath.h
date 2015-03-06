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

	const Point* getActivePathPoints(int requestedLength, int* actualLength) const
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

	bool isConverged(float pointRadius, float timeRadius = NUMERICAL_ERROR_THRESH*100, unsigned int pointsToCheck = 10, Point* convergencePoint = NULL)
	{
		if (pointsToCheck > points.size())
		{
			return false; // Not enough data to deciede convergence
		}

		int last = times.size() - 1;
		int firstToCheck = last - pointsToCheck + 1;

		if (abs(times[last] - times[firstToCheck]) > timeRadius)
		{
			return false; // Times are not close enough
		}
		Point centroid = Point(0,0,0);
		double totalDist(0.);
		for (unsigned int i = 0; i < pointsToCheck; ++i)
		{
			int current = firstToCheck + i;
			int next = firstToCheck + ((i + 1) % pointsToCheck);
			centroid += points[current];
			totalDist += (points[current] - points[next]).length();
			float distanceToCentroid = (points[current] - centroid).length();
			bool debug = true;
		}

		centroid /= (float)pointsToCheck;
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
		for (int i = firstToCheck; i <= last; ++i)
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

