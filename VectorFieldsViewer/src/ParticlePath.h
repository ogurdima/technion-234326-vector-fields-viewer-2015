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
		int s = (int) size();
		assert(s > 0 && particleLoc >= 0 && particleLoc < s);
		double currentTime = times[particleLoc];
		if (particleLoc == s - 1)
		{
			particleLoc = rand() % std::max( ((int) s / 5), s);
			return;
		}
		for (int nextLoc = particleLoc; nextLoc < times.size(); nextLoc++)
		{
			if (times[nextLoc] >= currentTime + dt)
			{
				particleLoc = nextLoc;
				assert(particleLoc >= 0);
				return;
			}
		}
		particleLoc = times.size() - 1;
	}
};

