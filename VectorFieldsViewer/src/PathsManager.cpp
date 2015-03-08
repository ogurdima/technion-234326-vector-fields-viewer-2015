#include "PathsManager.h"



PathsManager::PathsManager() :
	colorData(NULL),
	pointData(NULL)
{
	
}

void PathsManager::Configure(int _maxPathLength, Vec3f _baseColor, vector<ParticlePath> paths)
{
	maxPathLength = _maxPathLength;
	baseColor = _baseColor;
	
	
}


void PathsManager::ComputeRequiredSpace(vector<ParticlePath> paths)
{
	pathStartIndices.clear();
	numOfPoints = 0;
	for (unsigned int pathIdx = 0; pathIdx < paths.size(); pathIdx++)
	{
		pathStartIndices.push_back(numOfPoints);
		numOfPoints += paths[pathIdx].size();
	}
	pointDataLength = 3 * numOfPoints;

	//colorIndex.resize(maxPathLength + 1);
	colorIndex.clear();
	numOfColors = 0;
	for (unsigned int pathLen = 0; pathLen <= (unsigned int) maxPathLength; pathLen++)
	{
		numOfColors += pathLen;
		colorIndex.push_back(numOfColors);
	}
	colorDataLength = 4 * numOfColors;
}

void PathsManager::AllocateSpace()
{
	cout << "Allocating " << sizeof(float) * (pointDataLength + colorDataLength) << " bytes of memory" << endl;

	if (NULL != colorData)
	{
		delete[] colorData;
		colorData = NULL;
	}
	colorData = new float[colorDataLength];

	if (NULL != pointData)
	{
		delete[] pointData;
		pointData = NULL;
	}
	pointData = new float[pointDataLength];
}

void PathsManager::FillPointData(vector<ParticlePath> paths)
{
	unsigned int writeHead = 0;
	for (unsigned int pathIdx = 0; pathIdx < paths.size(); pathIdx++)
	{
		vector<Vec3f> pathPoints = paths[pathIdx].getPoints();
		for (unsigned int pointIdx = 0; pointIdx < pathPoints.size(); pointIdx++)
		{
			pointData[writeHead++] = pathPoints[pointIdx][0];
			pointData[writeHead++] = pathPoints[pointIdx][1];
			pointData[writeHead++] = pathPoints[pointIdx][2];
		}
	}
}

void PathsManager::FillColorData()
{
	
}








