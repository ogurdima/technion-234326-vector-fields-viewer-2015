#pragma once

#include "ParticlePath.h"
#include <vector>
#include <iostream>

using namespace std;

class PathsManager
{
public:
	PathsManager();
	void Configure(int _maxPathLength, Vec3f _baseColor, vector<ParticlePath> paths);
protected:
	Vec3f					baseColor;
	int						maxPathLength;
	int						numOfPoints;
	int						numOfColors;
	int						colorDataLength;
	int						pointDataLength;
	float*					colorData;
	float*					pointData;

	vector<unsigned int>	colorIndex;
	vector<unsigned int>	pathStartIndices;

	void ComputeRequiredSpace(vector<ParticlePath> paths);
	void AllocateSpace();
	void FillPointData(vector<ParticlePath> paths);
	void FillColorData();
};
