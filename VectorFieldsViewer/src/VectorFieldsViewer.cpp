#include "VectorFieldsViewer.h"
#include <math.h>

VectorFieldsViewer VectorFieldsViewer::instance;
int VectorFieldsViewer::drawingTimeout;

VectorFieldsViewer::VectorFieldsViewer(void) :
	visualisationTimeInterval(0.001),
	drawState(DrawStateType::SOLID_SMOOTH),
	resetSceneEvent(NULL),
	redrawEvent(NULL),
	maxTime(1),
	minTime(0),
	curTime(0),
	isVisualizationStopped(false)
{
	loadedFieldPath = "?"; // This is an invalid path
}

#pragma region Callbacks
void VectorFieldsViewer::changedDrawStateCallback(int val)
{
	instance.setDrawState((DrawStateType) val);
}

void VectorFieldsViewer::setDrawState(DrawStateType ds)
{
	switch(ds)
	{
	case(DrawStateType::WIREFRAME):
	case(DrawStateType::SOLID_FLAT):
	case(DrawStateType::SOLID_SMOOTH):
	case(DrawStateType::FRONT_FIELD):
	case(DrawStateType::FIELD):
		drawState = ds;
		break;
	default:
		drawState = DrawStateType::SOLID_SMOOTH;
		std::cerr << "Unknown draw state: " << ds << std::endl;
		break;
	}
	UpdateDrawStateGui((int)drawState);
	if(redrawEvent != NULL)
	{
		redrawEvent();
	}
}

void VectorFieldsViewer::openMeshCallback(char* path)
{
	instance.openMesh(path);
	instance.loadedFieldPath = "?";
}

void VectorFieldsViewer::openMesh(char* path)
{
	fieldedMesh = FieldedMesh();
	if (fieldedMesh.load(path))
	{
		std::cout << fieldedMesh.n_vertices() << " vertices, " << fieldedMesh.n_faces()    << " faces\n";
		setDrawState(DrawStateType::SOLID_SMOOTH);
		if(resetSceneEvent != NULL)
		{
			(*resetSceneEvent)();
		}
	}
	else
	{
		std::cerr << "failed to load the mesh from: " << path << std::endl;
	}
	if(redrawEvent != NULL)
	{
		(*redrawEvent)();
	}
}

void VectorFieldsViewer::changedMeshColorCallback(float r, float g, float b, float a)
{
	instance.fieldedMesh.setMeshColor(Vec4f(r,g,b,a));
	if(instance.redrawEvent != NULL)
	{
		instance.redrawEvent();
	}
}

void VectorFieldsViewer::changedFieldColorCallback(float r, float g, float b, float a)
{
	instance.pathsMgr.ChangeBaseColor(Vec4f(r,g,b,a));
}

void VectorFieldsViewer::changedVisualizationCallback(int drawTimeout, double step, double window)
{
	if (drawTimeout < 0)
	{
		instance.isVisualizationStopped = true;
		return;
	}
	instance.isVisualizationStopped = false;

	drawingTimeout = drawTimeout;
	instance.pathsMgr.ChangePathWindow(window);
	instance.visualisationTimeInterval = step;
}

void VectorFieldsViewer::recomputePathsCallback(char* path, bool isConst, double step, double min, double max)
{
	string p(path);
	bool loaded = false;
	instance.maxTime = max;
	instance.minTime = min;
	instance.curTime = instance.minTime;

	if (p == instance.loadedFieldPath)
	{
		loaded = true;
	}
	else
	{
		if(p.length() == 0)
		{
			std::cout << "Assigning default rotating field" << path << std::endl;
			loaded = instance.fieldedMesh.assignDefaultField(min, max);
		}
		else
		{
			std::cout << "Opening Field File " << path << std::endl;
			loaded = instance.fieldedMesh.assignVectorField(path, isConst);
		}
		if (!loaded)
		{
			std::cout << "Failed to assign field" << std::endl;
		}
	}
	instance.loadedFieldPath = p;
	if (loaded)
	{
		instance.computePaths(step);
	}
}

void VectorFieldsViewer::onTimer(int val)
{
	if (!isVisualizationStopped)
	{
		evolvePaths();
	}
}

void VectorFieldsViewer::openParameterWindow()
{
	OpenWindow(&VectorFieldsViewer::changedDrawStateCallback,
		&VectorFieldsViewer::openMeshCallback, 
		&VectorFieldsViewer::changedMeshColorCallback, 
		&VectorFieldsViewer::changedFieldColorCallback,
		&VectorFieldsViewer::changedVisualizationCallback,
		&VectorFieldsViewer::recomputePathsCallback);
}

#pragma endregion

VectorFieldsViewer& VectorFieldsViewer::getInstance()
{
	return instance;
}

int VectorFieldsViewer::getDrawingTimeout()
{
	return drawingTimeout;
}

void VectorFieldsViewer::evolvePaths()
{
	DrawStateType ds = getDrawState();
	if(ds != DrawStateType::FIELD && ds != DrawStateType::FRONT_FIELD)
	{
		return;
	}
	curTime += visualisationTimeInterval;
	if (curTime > maxTime)
	{
		curTime = minTime;
		pathsMgr.SetTime(curTime);
	}
	else 
	{
		pathsMgr.Evolve(visualisationTimeInterval);
	}
	UpdateCurrentTimeGui(curTime);
}

void VectorFieldsViewer::computePaths(double step)
{
	vector<ParticlePath> particlePaths;
	pathsMgr.Clear();
	try
	{
		particlePaths = PathFinder().getParticlePaths(fieldedMesh, step, minTime, maxTime);
	}
	catch(exception e)
	{
		std::cout << "Exception thrown while computing particle paths: " << e.what() << std::endl;
	}
	pathsMgr.Configure(particlePaths);
}

void VectorFieldsViewer::AddRedrawHandler(void (*redrawCallback)(void))
{
	redrawEvent = redrawCallback;
}

void VectorFieldsViewer::AddResetSceneHandler(void (*resetSceneCallback)(void))
{
	resetSceneEvent = resetSceneCallback;
}

const FieldedMesh& VectorFieldsViewer::getMesh()
{
	return fieldedMesh;
}

DrawStateType VectorFieldsViewer::getDrawState()
{
	if(!fieldedMesh.isLoaded())
		return DrawStateType::SOLID_SMOOTH;
	switch(drawState)
	{
	case(DrawStateType::FIELD):
	case(DrawStateType::FRONT_FIELD):
		{
			if(!fieldedMesh.hasField())
			{
				return DrawStateType::SOLID_SMOOTH;
			}
		}
		break;
	}
	return drawState;
}

void VectorFieldsViewer::GetCurrentPaths(float*& dataArray, unsigned int*& starts, unsigned int*& counts, unsigned int& pathCount)
{
	return pathsMgr.GetCurrentPaths(dataArray, starts, counts, pathCount);
}