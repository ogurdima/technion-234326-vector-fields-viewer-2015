#pragma once

#include "VectorFieldsUtils.h"
#include <math.h>
#include "psimpl.h"


class ParticlePath
{

private:
	vector<Point>	points;
	vector<Time>	times;
	int				particleLoc;
public:

	static bool compareBySize(const ParticlePath& left, const ParticlePath& right)
	{
		return (left.size() < right.size());
	}

	ParticlePath():
		particleLoc(0)
	{
		points.reserve(500);
		times.reserve(500);
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

	static bool compareFloats(float& a, float& b) { return (a < b); }

	bool tryCollapseLastPoints(float pointRadius)
	{
		int last = times.size() - 1;
		int prev = times.size() - 2;
		float dist = (points[last] - points[prev]).length();

		if (dist < pointRadius)
		{
			float t = times[last];
			Vec3f p = points[last];

			points.pop_back(); 
			points.pop_back();
			times.pop_back();
			times.pop_back();

			points.push_back(p);
			times.push_back(t);
			return true;
		}
		return false;
	}

	void simplify(Time requiredMinTime)
	{
		if (size() < 2)
		{
			return;
		}
		//psimpl::simplify_reumann_witkam<4>();


		
		float* flattened = new float[4*points.size()];
		vector<float> result;
		for(int i = 0; i < points.size(); i++)
		{
			flattened[4*i + 0] = points[i][0];
			flattened[4*i + 1] = points[i][1];
			flattened[4*i + 2] = points[i][2];
			flattened[4*i + 3] = times[i];
		}

		psimpl::simplify_reumann_witkam<4>(flattened, flattened + 4*points.size(), requiredMinTime, std::back_inserter(result));

		points.clear();
		times.clear();
		delete[] flattened;
		points.resize(result.size() / 4);
		times.resize(result.size() / 4);

		for (int i = 0; i < points.size(); i++)
		{
			float x = result[4 * i + 0];
			float y = result[4 * i + 1];
			float z = result[4 * i + 2];
			float t = result[4 * i + 3];
			points[i] = Vec3f(x,y,z);
			times[i] = t;
		}

		return;


		float actualMinTime = times[1] - times[0];
		for (unsigned int i = 1; i < times.size(); ++i)
		{
			actualMinTime = std::min(times[i] - times[i-1], actualMinTime);
		}
		if (actualMinTime > requiredMinTime)
		{
			return;
		}
		
		vector<bool> deleted;
		for (unsigned int i = 0; i < times.size(); ++i)
		{
			deleted.push_back(false);
		}

		int l = 0, r;
		Vec3f pointApprox;
		Time timeApprox;
		float count;
		while (l < points.size())
		{
			r = l + 1;
			count = 1;
			timeApprox = times[l];
			pointApprox = points[l];

			while (r < times.size() && times[r] - times[l] < requiredMinTime)
			{
				deleted[r] = true;
				timeApprox += times[r];
				pointApprox += points[r];
				count++;
				r++;
			}
			if (count > 1)
			{
				// collapse range (l,r-1) to r-1, new l is r-1.
				timeApprox /= count;
				pointApprox /= count;
				points[r-1] = pointApprox;
				times[r-1] = timeApprox;
				deleted[r-1] = false;
				deleted[l] = true;
				l = r-1;
			}
			else
			{
				l++;
			}
		}

		vector<Vec3f> newPoints;
		vector<Time> newTimes;

		for (int i = 0; i < points.size(); i++)
		{
			if (!deleted[i])
			{
				newPoints.push_back(points[i]);
				newTimes.push_back(times[i]);
			}
		}
		points = newPoints;
		times = newTimes;
	}
};

