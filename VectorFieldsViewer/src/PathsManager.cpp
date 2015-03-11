#include "PathsManager.h"



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
		PathHandle h(8 * pointsProcessedSoFar, 8 * paths[pathIdx].size(), maxPathLength);
		handles[pathIdx] = h;
		pointsProcessedSoFar += paths[pathIdx].size();
	}
	numOfPoints = pointsProcessedSoFar;

	if (NULL != data)
	{
		delete[] data;
		data = NULL;
	}
	data = new float[8 * numOfPoints];

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
			handles[pathIdx].data[8*pointIdx + 0] = pathPoints[pointIdx][0];
			handles[pathIdx].data[8*pointIdx + 1] = pathPoints[pointIdx][1];
			handles[pathIdx].data[8*pointIdx + 2] = pathPoints[pointIdx][2];
			handles[pathIdx].data[8*pointIdx + 3] = (float)pathTimes[pointIdx];
			handles[pathIdx].data[8*pointIdx + 4] = baseColor[0];
			handles[pathIdx].data[8*pointIdx + 5] = baseColor[1];
			handles[pathIdx].data[8*pointIdx + 6] = baseColor[2];
			handles[pathIdx].data[8*pointIdx + 7] = 1.f;
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
		unsigned int first = handles[pathIdx].dataIndex + handles[pathIdx].tail;
		unsigned int last = handles[pathIdx].dataIndex + handles[pathIdx].head;
		if (first != last)
		{
			int count = 0;
			for (unsigned int i = first; i <= last; i+=8, count++)
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
}

void PathsManager::GetCurrentPaths(float*& dataArray, unsigned int**& indices, unsigned int*& counts, unsigned int& pathCount)
{
	dataArray = data;
	indices = &(this->indices[0]);
	counts = &(this->counts[0]);
	pathCount = handles.size();
}


void PathHandle::evolve(float dt)
{
	curTime += dt;
	float maxTimef = maxTime();
	float minTimef = minTime();
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
	while (data[head + 3] < curTime)
	{
		head += 8;
		assert(head < dataSize);
	}
	int offset = -(int)maxPathLength * 8;
	offset += head;
	tail = max(0,offset);
}

float PathHandle::maxTime()
{
	return last()[3];
}

float PathHandle::minTime()
{
	return data[3];
}

float* PathHandle::last()
{
	return &data[lastIdx()];
}

unsigned int PathHandle::lastIdx()
{
	return (dataSize - 8);
}

