#include "PathsManager.h"

const unsigned int PathHandle::UnitSize		= 8;
const unsigned int PathHandle::TimeOffset	= 3;
const unsigned int PathHandle::ColorOffset	= 4;
const unsigned int PathHandle::AlphaOffset	= 6;


#pragma region PathsManager

PathsManager::PathsManager() :
	data(NULL)
{

}

void PathsManager::Configure(int _maxPathLength, Vec4f _baseColor, vector<ParticlePath> paths)
{
	maxPathLength = _maxPathLength;
	baseColor = _baseColor;
	handles.clear();
	handles.resize(paths.size());
	
	int pointsProcessedSoFar = 0;
	for (unsigned int pathIdx = 0; pathIdx < paths.size(); pathIdx++)
	{
		PathHandle h(pointsProcessedSoFar, paths[pathIdx].size(), maxPathLength);
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
		handles[pathIdx].data = &data[handles[pathIdx].globalPointIndex * PathHandle::UnitSize];
	}

	for (unsigned int pathIdx = 0; pathIdx < paths.size(); pathIdx++)
	{
		const vector<Vec3f>& pathPoints = paths[pathIdx].getPoints();
		const vector<Time>& pathTimes = paths[pathIdx].getTimes();
		for (unsigned int pointIdx = 0; pointIdx < pathPoints.size(); pointIdx++)
		{
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 0] = pathPoints[pointIdx][0];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 1] = pathPoints[pointIdx][1];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 2] = pathPoints[pointIdx][2];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 3] = (float)pathTimes[pointIdx];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 4] = baseColor[0];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 5] = baseColor[1];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 6] = baseColor[2];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 7] = baseColor[3];
		}
	}

	//for (unsigned int pathIdx = 0; pathIdx < indices.size(); pathIdx++)
	//{
	//	if (NULL != indices[pathIdx])
	//	{
	//		delete[] indices[pathIdx];
	//		indices[pathIdx] = NULL;
	//	}
	//}
	//indices.clear();
	//indices.resize(handles.size());

	starts.clear();
	starts.resize(handles.size());
	counts.clear();
	counts.resize(handles.size());
	for (unsigned int pathIdx = 0; pathIdx < handles.size(); pathIdx++)
	{
		//indices[pathIdx] = new unsigned int[maxPathLength];
		starts[pathIdx] = counts[pathIdx] = 0;
	}

}

void PathsManager::Evolve(Time dt)
{
	for (unsigned int pathIdx = 0; pathIdx < handles.size(); pathIdx++)
	{
		handles[pathIdx].evolve((float)dt);
		unsigned int first = handles[pathIdx].tailGlobIdx();
		unsigned int last = handles[pathIdx].headGlobIdx();
		UpdateStartsAndCounts(pathIdx, first, last);
	}
}

void PathsManager::SetTime(Time t)
{
	for (unsigned int pathIdx = 0; pathIdx < handles.size(); pathIdx++)
	{
		handles[pathIdx].setTime((float)t);
		unsigned int first = handles[pathIdx].tailGlobIdx();
		unsigned int last = handles[pathIdx].headGlobIdx();
		UpdateStartsAndCounts(pathIdx, first, last);
	}
}

//void PathsManager::UpdateIndecesAndCounts(unsigned int pathIdx, unsigned int first, unsigned int last)
//{
//	if (first != last)
//	{
//		int count = 0;
//		for (unsigned int i = first; i <= last; i+=PathHandle::UnitSize, count++)
//		{
//			indices[pathIdx][count] = i;
//		}
//		counts[pathIdx] = count;
//	}
//	else
//	{
//		counts[pathIdx] = 0;
//	}
//}

void PathsManager::UpdateStartsAndCounts(unsigned int pathIdx, unsigned int first, unsigned int last)
{
	if (first != last)
	{
		int count = last - first + 1;
		starts[pathIdx] = first;
		counts[pathIdx] = count;
	}
	else
	{
		counts[pathIdx] = 0;
	}
}

void PathsManager::GetCurrentPaths(float*& dataArray, unsigned int*& starts, unsigned int*& counts, unsigned int& pathCount)
{
	dataArray = data;
	//indices = &(this->indices[0]);
	counts = &(this->counts[0]);
	starts = &(this->starts[0]);
	pathCount = handles.size();
}

void PathsManager::ChangeBaseColor(const Vec4f& rgba)
{
	baseColor = rgba;
	for (int pointIdx = 0; pointIdx < numOfPoints; pointIdx++)
	{
		data[pointIdx * PathHandle::UnitSize + PathHandle::ColorOffset + 0] = baseColor[0];
		data[pointIdx * PathHandle::UnitSize + PathHandle::ColorOffset + 1] = baseColor[1];
		data[pointIdx * PathHandle::UnitSize + PathHandle::ColorOffset + 2] = baseColor[2];
		data[pointIdx * PathHandle::UnitSize + PathHandle::ColorOffset + 3] = baseColor[3];
	}
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
	if (curTime <= minTime())
	{
		head = tail = 0;
		return;
	}
	while (data[head + TimeOffset] < curTime)
	{
		head += UnitSize;
		assert(head <= lastIdx());
	}
	head -= UnitSize;
	tail = max(0, (int)head - (int)(maxPathLength * UnitSize));
	updateAlphaValues();
}

void PathHandle::setTime(float t)
{
	curTime = 0;
	head = tail = 0;
	evolve(t);
}

unsigned int PathHandle::headGlobIdx()
{
	return globalPointIndex + head / UnitSize;
}

unsigned int PathHandle::tailGlobIdx()
{
	return globalPointIndex + tail / UnitSize;
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

float* PathHandle::at(unsigned int pointIndex)
{
	return &data[pointIndex *UnitSize];
}

unsigned int PathHandle::lastIdx()
{
	return ((numPoints - 1) * UnitSize);
}

void PathHandle::updateAlphaValues()
{

}