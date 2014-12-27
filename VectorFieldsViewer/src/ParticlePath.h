#pragma once

#include "VectorFieldsUtils.h"

class ParticlePath
{

private:
	vector<Point> points;
	vector<Time> times;
public:

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
};

