#pragma once

#include "ParticlePath.h"
#include <vector>
#include <iostream>

using namespace std;

class PathHandle
{
public:
	float curTime;
	float* data;
	unsigned int dataIndex;
	unsigned int dataSize;
	unsigned int head;
	unsigned int tail;
	unsigned int maxPathLength;

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
		curTime(0),
		head(0),
		tail(0)
	{
		dataIndex = _dataIndex;
		dataSize = _dataSize;
		maxPathLength = _maxPathLength;
	}

	void evolve(float dt);

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
	void GetCurrentPaths(float*& dataArray, unsigned int**& indices, unsigned int*& counts, unsigned int& pathCount);
protected:
	Vec3f					baseColor;
	int						maxPathLength;
	int						numOfPoints;
	float*					data;
	vector<PathHandle>		handles;
	
	vector<unsigned int *>	indices;
	vector<unsigned int>	counts;
};

struct PathDataItem
{
private:
	float* data;
public:
	PathDataItem(float* _data) { data = _data; }
	inline float*		p() { return &data[0]; }
	inline float*		t() { return &data[3]; }
	inline float*		c() { return &data[4]; }
};


