#pragma once

#include "VectorFieldsUtils.h"
#include <math.h>

class ParticlePath
{

private:
	vector<Point>	points;
	vector<Time>	times;
	int				particleLoc;
public:

	ParticlePath():
		particleLoc(0)
	{
	}

	inline const vector<Point>& getPoints() const
	{
		return points;
	}

	inline const vector<Time>& getTimes() const
	{
		return times;
	}

	inline const uint size() const
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

