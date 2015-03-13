#include "VectorFieldsViewer.h"

VectorFieldsViewer VectorFieldsViewer::instance;
int VectorFieldsViewer::drawingTimeout;


VectorFieldsViewer::VectorFieldsViewer(void) :
	fieldSimulationTimeInterval(0.001),
	visualisationTimeInterval(0.0001),
	drawState(DrawStateType::NONE),
	fieldColor(1,1,1,0),
	meshColor(0.1,0.1,0.1,1),
	resetSceneEvent(NULL),
	redrawEvent(NULL),
	maxTime(1),
	minTime(0),
	curTime(0)
{
}

VectorFieldsViewer::~VectorFieldsViewer(void)
{
	resetColorAndIndices();
}

void VectorFieldsViewer::openMeshCallback(char* path)
{
	instance.openMesh(path);
}

void VectorFieldsViewer::changedDrawStateCallback(int val)
{
	DrawStateType ds =(DrawStateType) val;
	switch(ds )
	{
	case(DrawStateType::NONE):
	case(DrawStateType::WIREFRAME):
	case(DrawStateType::SOLID_FLAT):
	case(DrawStateType::SOLID_SMOOTH):
	case(DrawStateType::FRONT_FIELD):
	case(DrawStateType::FIELD):
		instance.drawState = ds;
		break;
	default:
		instance.drawState = DrawStateType::NONE;
		std::cerr << "Unknown draw state: " << val << std::endl;
		break;
	}
	if(instance.redrawEvent != NULL)
	{
		instance.redrawEvent();
	}
}

void VectorFieldsViewer::changedMeshColorCallback(float r, float g, float b, float a)
{
	instance.meshColor = Vec4f(r,g,b,a);
	getInstance().fieldedMesh.setMeshColor(Vec4f(r,g,b,a));
	if(getInstance().redrawEvent != NULL)
	{
		instance.redrawEvent();
	}
}

void VectorFieldsViewer::changedFieldColorCallback(float r, float g, float b, float a)
{
	instance.fieldColor = Vec4f(r,g,b,a);
	instance.pathsMgr.ChangeBaseColor(instance.fieldColor);
}

void VectorFieldsViewer::openMesh(char* path)
{
	if (fieldedMesh.load(path))
	{
		std::cout << fieldedMesh.n_vertices() << " vertices, " << fieldedMesh.n_faces()    << " faces\n";
		maxTime = fieldedMesh.maxTime();
		minTime = fieldedMesh.minTime();
		fieldSimulationTimeInterval = (maxTime - minTime) / 100.f;
		visualisationTimeInterval = fieldSimulationTimeInterval / 10.f;
		curTime = minTime;

		if(resetSceneEvent != NULL)
		{
			(*resetSceneEvent)();
		}
		computePaths();
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

void VectorFieldsViewer::onTimer(int val)
{
	evolvePaths();
}

void VectorFieldsViewer::openFieldCallback(char* path, bool isConst)
{
	instance.openField(path, isConst);
}

void VectorFieldsViewer::openField(char* path, bool isConst)
{
	std::cout << "Opening Field File " << path << std::endl;
	bool success = fieldedMesh.assignVectorField(path, isConst);
	if (!success) 
	{
		std::cout << "Failed to read field" << std::endl;
	}
	else
	{
		maxTime = fieldedMesh.maxTime();
		minTime = fieldedMesh.minTime();
		fieldSimulationTimeInterval = (maxTime - minTime) / 100.f;
		visualisationTimeInterval = fieldSimulationTimeInterval / 10.f;
		curTime = minTime;
		computePaths();
	}
}

void VectorFieldsViewer::changeDrawingTimeout(int range)
{
	drawingTimeout = range;
}

void VectorFieldsViewer::openParameterWindow()
{
	OpenWindow(&VectorFieldsViewer::changeDrawingTimeout,
		&VectorFieldsViewer::changedDrawStateCallback, 
		&VectorFieldsViewer::changedMeshColorCallback, 
		&VectorFieldsViewer::changedFieldColorCallback, 
		&VectorFieldsViewer::openMeshCallback, 
		&VectorFieldsViewer::openFieldCallback,
		&VectorFieldsViewer::changedPathWindowCallback);
}

void VectorFieldsViewer::changedPathWindowCallback(double val)
{

}

void VectorFieldsViewer::resetColorAndIndices()
{
	int maxActivePathLength = 30;
	colors.resize(maxActivePathLength * 4);
	indices.resize(maxActivePathLength);
	for(uint i = 0; i < maxActivePathLength; ++i)
	{
		indices[i] = i;
		colors[i * 4] = fieldColor[0];
		colors[i * 4 + 1] = fieldColor[1];
		colors[i * 4 + 2] = fieldColor[2];
		colors[i * 4 + 3] = fieldColor[3] * std::sqrt(std::sqrt( ((float) i) / maxActivePathLength)) / 3;
	}
}

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
}

void VectorFieldsViewer::computePaths()
{
	vector<ParticlePath> particlePaths;
	bool success = pathFinder.configure(fieldedMesh, fieldSimulationTimeInterval);
	if (success) 
	{
		particlePaths = pathFinder.getParticlePaths();
	}
	else 
	{
		std::cerr << "Failed to properly configure PathFinder" << std::endl;
		particlePaths = vector<ParticlePath>();
	}
	pathsMgr.Configure(fieldColor, particlePaths, (maxTime - minTime) / 5 );
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

const Vec4f& VectorFieldsViewer::getMeshColor()
{
	return meshColor;
}

const Vec4f& VectorFieldsViewer::getFieldColor()
{
	return fieldColor;
}

DrawStateType VectorFieldsViewer::getDrawState()
{
	if(!fieldedMesh.isLoaded())
		return DrawStateType::NONE;
	switch(drawState)
	{
	case(DrawStateType::FIELD):
	case(DrawStateType::FRONT_FIELD):
		{
			if(!fieldedMesh.hasField())
			{
				return DrawStateType::NONE;
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