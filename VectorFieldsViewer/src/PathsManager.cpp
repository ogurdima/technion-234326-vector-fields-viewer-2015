#include "PathsManager.h"

const unsigned int PathHandle::UnitSize		= 8;
const unsigned int PathHandle::TimeOffset	= 3;
const unsigned int PathHandle::ColorOffset	= 4;
const unsigned int PathHandle::AlphaOffset	= 6;


#pragma region PathsManager

PathsManager::PathsManager() :
	data(NULL)
{
	indices.clear();
}

void PathsManager::Configure(int _maxPathLength, Vec3f _baseColor, vector<ParticlePath> paths)
{
	maxPathLength = _maxPathLength;
	baseColor = _baseColor;
	handles.clear();
	handles.resize(paths.size());
	
	int pointsProcessedSoFar = 0;
	for (unsigned int pathIdx = 0; pathIdx < paths.size(); pathIdx++)
	{
		PathHandle h(PathHandle::UnitSize * pointsProcessedSoFar, PathHandle::UnitSize * paths[pathIdx].size(), maxPathLength);
		handles[pathIdx] = h;
		pointsProcessedSoFar += paths[pathIdx].size();
	}
	numOfPoints = pointsProcessedSoFar;

	if (NULL != data)
	{
		delete[] data;
		data = NULL;
	}
	data = new float[PathHandle::UnitSize * numOfPoints];

	for (unsigned int pathIdx = 0; pathIdx < handles.size(); pathIdx++)
	{
		handles[pathIdx].data = &data[handles[pathIdx].dataIndex];
	}

	for (unsigned int pathIdx = 0; pathIdx < paths.size(); pathIdx++)
	{
		vector<Vec3f> pathPoints = paths[pathIdx].getPoints();
		vector<Time> pathTimes = paths[pathIdx].getTimes();
		for (unsigned int pointIdx = 0; pointIdx < pathPoints.size(); pointIdx++)
		{
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 0] = pathPoints[pointIdx][0];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 1] = pathPoints[pointIdx][1];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 2] = pathPoints[pointIdx][2];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 3] = (float)pathTimes[pointIdx];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 4] = baseColor[0];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 5] = baseColor[1];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 6] = baseColor[2];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 7] = 1.f;
		}
	}

	for (unsigned int pathIdx = 0; pathIdx < indices.size(); pathIdx++)
	{
		if (NULL != indices[pathIdx])
		{
			delete[] indices[pathIdx];
			indices[pathIdx] = NULL;
		}
	}
	indices.clear();
	indices.resize(handles.size());
	counts.clear();
	counts.resize(handles.size());
	for (unsigned int pathIdx = 0; pathIdx < indices.size(); pathIdx++)
	{
		indices[pathIdx] = new unsigned int[maxPathLength];
		counts[pathIdx] = 0;
	}

}

void PathsManager::Evolve(Time dt)
{
	for (unsigned int pathIdx = 0; pathIdx < handles.size(); pathIdx++)
	{
		handles[pathIdx].evolve((float)dt);
		unsigned int first = handles[pathIdx].tailGlobIdx();
		unsigned int last = handles[pathIdx].headGlobIdx();
		UpdateIndecesAndCounts(pathIdx, first, last);
	}
}

void PathsManager::SetTime(Time t)
{
	for (unsigned int pathIdx = 0; pathIdx < handles.size(); pathIdx++)
	{
		handles[pathIdx].setTime((float)t);
		unsigned int first = handles[pathIdx].tailGlobIdx();
		unsigned int last = handles[pathIdx].headGlobIdx();
		UpdateIndecesAndCounts(pathIdx, first, last);
	}
}

void PathsManager::UpdateIndecesAndCounts(unsigned int pathIdx, unsigned int first, unsigned int last)
{
	if (first != last)
	{
		int count = 0;
		for (unsigned int i = first; i <= last; i+=PathHandle::UnitSize, count++)
		{
			indices[pathIdx][count] = i;
		}
		counts[pathIdx] = count;
	}
	else
	{
		counts[pathIdx] = 0;
	}
}

void PathsManager::GetCurrentPaths(float*& dataArray, unsigned int**& indices, unsigned int*& counts, unsigned int& pathCount)
{
	dataArray = data;
	indices = &(this->indices[0]);
	counts = &(this->counts[0]);
	pathCount = handles.size();
}

#pragma endregion


void PathHandle::evolve(float dt)
{
	curTime += dt;
	if (curTime > maxTime())
	{
		head = tail = lastIdx();
		return;
	}
	if (curTime < minTime())
	{
		head = tail = 0;
		return;
	}
	while (data[head + TimeOffset] < curTime)
	{
		head += UnitSize;
		assert(head < dataSize);
	}
	head -= UnitSize;

	int offset = -(int)maxPathLength * UnitSize;
	offset += head;
	tail = max(0,offset);
}

void PathHandle::setTime(float t)
{
	curTime = 0;
	head = tail = 0;
	evolve(t);
}

unsigned int PathHandle::headGlobIdx()
{
	return dataIndex + head;
}

unsigned int PathHandle::tailGlobIdx()
{
	return dataIndex + tail;
}

float PathHandle::maxTime()
{
	return last()[TimeOffset];
}

float PathHandle::minTime()
{
	return data[TimeOffset];
}

float* PathHandle::last()
{
	return &data[lastIdx()];
}

unsigned int PathHandle::lastIdx()
{
	return (dataSize - UnitSize);
}

