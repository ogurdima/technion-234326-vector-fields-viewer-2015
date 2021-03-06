#include "PathsManager.h"

const unsigned int PathHandle::UnitSize		= 8;
const unsigned int PathHandle::TimeOffset	= 3;
const unsigned int PathHandle::ColorOffset	= 4;
const unsigned int PathHandle::AlphaOffset	= 7;


#pragma region PathsManager

PathsManager::PathsManager() :
	data(NULL), 
	headBaseColor(Vec4f(0,1,0,1)),
	tailBaseColor(Vec4f(0,1,0,0))
{

}

void PathsManager::Clear()
{
	numOfPoints = 0;
	if (NULL != data)
	{
		delete[] data;
		data = NULL;
	}
	handles.clear();
	counts.clear();
	starts.clear();
}

void PathsManager::Configure(const vector<ParticlePath>& paths)
{
	cout << "Configuring PathsManager with " << paths.size() << " paths" << endl;
	handles.clear();
	handles.resize(paths.size());
	
	int pointsProcessedSoFar = 0;
	for (unsigned int pathIdx = 0; pathIdx < paths.size(); pathIdx++)
	{
		handles[pathIdx] = PathHandle(pointsProcessedSoFar, paths[pathIdx].size(), maxPathTimeSpan, headBaseColor, tailBaseColor);
		pointsProcessedSoFar += paths[pathIdx].size();
	}
	numOfPoints = pointsProcessedSoFar;

	if (NULL != data)
	{
		delete[] data;
		data = NULL;
	}
	int attemps = 100;
	
	while( NULL == data && --attemps > 0)
	{
		try
		{
			data = new float[PathHandle::UnitSize * numOfPoints];
		}
		catch(...) {}
	}
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
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 4] = headBaseColor[0];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 5] = headBaseColor[1];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 6] = headBaseColor[2];
			handles[pathIdx].data[PathHandle::UnitSize * pointIdx + 7] = headBaseColor[3];
		}
	}

	starts.clear();
	starts.resize(handles.size());
	counts.clear();
	counts.resize(handles.size());
	for (unsigned int pathIdx = 0; pathIdx < handles.size(); pathIdx++)
	{
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
	counts = &(this->counts[0]);
	starts = &(this->starts[0]);
	pathCount = handles.size();
}

void PathsManager::ChangeBaseColor(const Vec4f& head, const Vec4f& tail)
{
	headBaseColor = head;
	tailBaseColor = tail;
	//for (int pointIdx = 0; pointIdx < numOfPoints; pointIdx++)
	//{
	//	data[pointIdx * PathHandle::UnitSize + PathHandle::ColorOffset + 0] = baseColor[0];
	//	data[pointIdx * PathHandle::UnitSize + PathHandle::ColorOffset + 1] = baseColor[1];
	//	data[pointIdx * PathHandle::UnitSize + PathHandle::ColorOffset + 2] = baseColor[2];
	//	data[pointIdx * PathHandle::UnitSize + PathHandle::ColorOffset + 3] = baseColor[3];
	//}
	for (unsigned int pathIdx = 0; pathIdx < handles.size(); pathIdx++)
	{
		handles[pathIdx].headBaseColor = headBaseColor;
		handles[pathIdx].tailBaseColor = tailBaseColor;
	}
}

void PathsManager::ChangePathWindow(float pathWindow)
{
	maxPathTimeSpan = pathWindow;
	for(uint i = 0; i < handles.size(); ++i)
	{
		handles[i].maxPathTimeSpan = pathWindow;
	}
}

#pragma endregion

void PathHandle::evolve(float dt)
{
	restoreCurrentHeadTail();
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
	if (curTime - maxPathTimeSpan < minTime())
	{
		tail = 0;
	}
	else
	{
		while (data[tail + TimeOffset] < curTime - maxPathTimeSpan)
		{
			tail += UnitSize;
		}
		if (tail != 0)
		{
			tail -= UnitSize;
		}
	}

	substituteHeadTail();
	updatePathColors();
}

void PathHandle::setTime(float t)
{
	restoreCurrentHeadTail();
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

void PathHandle::setColor(float* elem, const Vec4f& c)
{
	elem[ColorOffset + 0] = c[0];
	elem[ColorOffset + 1] = c[1];
	elem[ColorOffset + 2] = c[2];
	elem[AlphaOffset] = c[3];
}

void PathHandle::updatePathColors()
{
	setColor(&data[head], headBaseColor);
	setColor(&data[tail], tailBaseColor);
	if (head == tail)
	{
		return;
	}
	float pathMaxTime = data[head + TimeOffset];
	float pathMinTime = data[tail + TimeOffset];
	float timeSpan = pathMaxTime - pathMinTime;
	assert(timeSpan > 0);
	for (unsigned int i = tail + UnitSize; i < head; i+=UnitSize)
	{
		float strength = (data[i + TimeOffset] - pathMinTime) / timeSpan;
		assert(strength < 1);
		Vec4f interpolatedColor = VectorFieldsUtils::lerp(headBaseColor, tailBaseColor, 1-strength);
		setColor(&data[i], interpolatedColor);
	}
}

void PathHandle::storeCurrentHeadTail()
{
	for (int i = 0; i < UnitSize; i++)
	{
		curHeadData[i] = data[head + i];
		curTailData[i] = data[tail + i];
	}
	lastStoredHead = head;
	lastStoredTail = tail;
}

void PathHandle::restoreCurrentHeadTail()
{
	if (lastStoredHead < 0 || lastStoredTail < 0)
	{
		return;
	}

	for (int i = 0; i < UnitSize; i++)
	{
		data[lastStoredHead + i] = curHeadData[i];
		data[lastStoredTail + i] = curTailData[i];
	}
}

void PathHandle::substituteHeadTail()
{
	if (head == tail)
	{
		return;
	}
	storeCurrentHeadTail();
	
	if (head != lastIdx() && head != 0)
	{
		interpolateNeighbors(head, curTime);
	}
	if (tail != lastIdx() && tail != 0)
	{
		interpolateNeighbors(tail, -1);
	}
}

void PathHandle::interpolateNeighbors(unsigned int target, float time)
{
	unsigned int next = target + UnitSize;
	unsigned int prev = target - UnitSize;
	float nextTime = data[next + TimeOffset];
	float prevTime = data[prev + TimeOffset];
	if (time < 0)
	{
		time = (nextTime + prevTime) / 2;
	}

	float ratio = (time - prevTime) / (nextTime - prevTime);
	data[target + 0] = (1-ratio) * data[prev + 0] + ratio * data[next + 0];
	data[target + 1] = (1-ratio) * data[prev + 1] + ratio * data[next + 1];
	data[target + 2] = (1-ratio) * data[prev + 2] + ratio * data[next + 2];
	data[target + 3] = (1-ratio) * data[prev + 3] + ratio * data[next + 3];
	data[target + 4] = (1-ratio) * data[prev + 4] + ratio * data[next + 4];
	data[target + 5] = (1-ratio) * data[prev + 5] + ratio * data[next + 5];
	data[target + 6] = (1-ratio) * data[prev + 6] + ratio * data[next + 6];
	data[target + 7] = (1-ratio) * data[prev + 7] + ratio * data[next + 7];
}


