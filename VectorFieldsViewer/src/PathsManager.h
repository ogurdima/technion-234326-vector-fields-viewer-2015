#pragma once

#include "ParticlePath.h"
#include <vector>
#include <iostream>

using namespace std;

class PathHandle
{
public:

	static const unsigned int		UnitSize;
	static const unsigned int		TimeOffset;
	static const unsigned int		ColorOffset;
	static const unsigned int		AlphaOffset;


	float							curTime;
	float*							data;
	unsigned int					globalPointIndex;
	unsigned int					numPoints;
	unsigned int					head;
	unsigned int					tail;
	float							maxPathTimeSpan;
	float							baseAlpha;
	

	PathHandle() :
		data(NULL),
		globalPointIndex(0),
		numPoints(0),
		curTime(0),
		head(0),
		tail(0),
		baseAlpha(1),
		lastStoredHead(-1),
		lastStoredTail(-1),
		maxPathTimeSpan(1)
	{}

	PathHandle(unsigned int _dataIndex, unsigned int _dataSize, float _baseAlpha, float _maxPathTimeSpan) :
		data(NULL),
		globalPointIndex(0),
		numPoints(0),
		curTime(0),
		head(0),
		tail(0),
		baseAlpha(1),
		lastStoredHead(-1),
		lastStoredTail(-1),
		maxPathTimeSpan(1)
	{
		globalPointIndex = _dataIndex;
		numPoints = _dataSize;
		baseAlpha = _baseAlpha;
		maxPathTimeSpan = _maxPathTimeSpan;
	}

	void evolve(float dt);

	void setTime(float t);

	unsigned int headGlobIdx();

	unsigned int tailGlobIdx();

	
	

private:
	float maxTime();
	float minTime();
	float* last();
	float* at(unsigned int pointIndex);
	unsigned int lastIdx();
	void updateAlphaValues();
	void storeCurrentHeadTail();
	void restoreCurrentHeadTail();
	void substituteHeadTail();
	void interpolateNeighbors(unsigned int target, float time);

	float	curHeadData[8];
	float	curTailData[8];
	int lastStoredHead;
	int lastStoredTail;
};


class PathsManager
{
public:
	PathsManager();
	void Configure(Vec4f _baseColor, vector<ParticlePath> paths, float _maxPathTimeSpan);
	void Evolve(Time dt);
	void SetTime(Time t);
	void GetCurrentPaths(float*& dataArray, unsigned int*& starts, unsigned int*& counts, unsigned int& pathCount);
	void ChangeBaseColor(const Vec4f& rgba);
	void ChangePathWindow(double pathWindow);

private:
	Vec4f					baseColor;
	float					maxPathTimeSpan;
	int						numOfPoints;
	float*					data;
	vector<PathHandle>		handles;
	vector<unsigned int>	counts;
	vector<unsigned int>	starts;
	void UpdateStartsAndCounts(unsigned int pathIdx, unsigned int first, unsigned int last);

};


