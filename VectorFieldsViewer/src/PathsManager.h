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
	unsigned int					maxPathLength;
	float							baseAlpha;

	PathHandle() :
		data(NULL),
		globalPointIndex(0),
		numPoints(0),
		curTime(0),
		head(0),
		tail(0),
		baseAlpha(1)
	{}

	PathHandle(unsigned int _dataIndex, unsigned int _dataSize, unsigned int _maxPathLength) :
		data(NULL),
		globalPointIndex(0),
		numPoints(0),
		curTime(0),
		head(0),
		tail(0),
		baseAlpha(1)
	{
		globalPointIndex = _dataIndex;
		numPoints = _dataSize;
		maxPathLength = _maxPathLength;
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
};


class PathsManager
{
public:
	PathsManager();
	void Configure(int _maxPathLength, Vec4f _baseColor, vector<ParticlePath> paths);
	void Evolve(Time dt);
	void SetTime(Time t);
	void GetCurrentPaths(float*& dataArray, unsigned int*& starts, unsigned int*& counts, unsigned int& pathCount);
	void ChangeBaseColor(const Vec4f& rgba);
protected:
	Vec4f					baseColor;
	int						maxPathLength;
	int						numOfPoints;
	float*					data;
	vector<PathHandle>		handles;
	//vector<unsigned int *>	indices;
	vector<unsigned int>	counts;
	vector<unsigned int>	starts;

	//void UpdateIndecesAndCounts(unsigned int pathIdx, unsigned int first, unsigned int last);
	void UpdateStartsAndCounts(unsigned int pathIdx, unsigned int first, unsigned int last);

};


