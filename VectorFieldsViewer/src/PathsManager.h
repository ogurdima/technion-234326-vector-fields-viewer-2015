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
	unsigned int					dataIndex;
	unsigned int					dataSize;
	unsigned int					head;
	unsigned int					tail;
	unsigned int					maxPathLength;

	PathHandle() :
		data(NULL),
		dataIndex(0),
		dataSize(0),
		curTime(0),
		head(0),
		tail(0)
	{}

	PathHandle(unsigned int _dataIndex, unsigned int _dataSize, unsigned int _maxPathLength) :
		data(NULL),
		dataIndex(0),
		dataSize(0),
		curTime(0),
		head(0),
		tail(0)
	{
		dataIndex = _dataIndex;
		dataSize = _dataSize;
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
	unsigned int lastIdx();
};


class PathsManager
{
public:
	PathsManager();
	void Configure(int _maxPathLength, Vec3f _baseColor, vector<ParticlePath> paths);
	void Evolve(Time dt);
	void SetTime(Time t);
	void GetCurrentPaths(float*& dataArray, unsigned int**& indices, unsigned int*& counts, unsigned int& pathCount);
protected:
	Vec3f					baseColor;
	int						maxPathLength;
	int						numOfPoints;
	float*					data;
	vector<PathHandle>		handles;
	vector<unsigned int *>	indices;
	vector<unsigned int>	counts;

	void UpdateIndecesAndCounts(unsigned int pathIdx, unsigned int first, unsigned int last);
};


