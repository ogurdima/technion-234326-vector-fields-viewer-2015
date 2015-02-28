#include "VectorFieldsViewer.h"

VectorFieldsViewer VectorFieldsViewer::instance;

VectorFieldsViewer::VectorFieldsViewer(void) :
	fieldSimulationTimeInterval(0.001),
	color(0.1,1,0.1),
	resetSceneEvent(NULL),
	redrawEvent(NULL)
{
}

VectorFieldsViewer::~VectorFieldsViewer(void)
{
	resetColorAndIndices();
}

void VectorFieldsViewer::openMeshCallback(string path)
{
	instance.openMesh(path);
}

void VectorFieldsViewer::openMesh(string path)
{
	if (fieldedMesh.load(path.c_str()))
	{
		std::cout << fieldedMesh.n_vertices() << " vertices, " << fieldedMesh.n_faces()    << " faces\n";
		fieldSimulationTimeInterval = (fieldedMesh.maxTime() - fieldedMesh.minTime()) / 1000.;
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

void VectorFieldsViewer::onTimer(int val)
{
	evolvePaths();
}

void VectorFieldsViewer::openFieldCallback(string path, bool isConst)
{
	instance.openField(path, isConst);
}

void VectorFieldsViewer::openField(string path, bool isConst)
{
	std::cout << "Opening Field File " << path << std::endl;
	bool success = fieldedMesh.assignVectorField(path.c_str(), isConst);
	if (!success) 
	{
		std::cout << "Failed to read field" << std::endl;
	}
	else
	{
		computePaths();
	}
}

void VectorFieldsViewer::changedRangeCallback(double range)
{
	instance.changeRange(range);
}

void VectorFieldsViewer::changeRange(double range)
{
	throw std::exception();
}

void VectorFieldsViewer::openParameterWindow()
{
	OpenWindow(&VectorFieldsViewer::changedRangeCallback, 
		&VectorFieldsViewer::openMeshCallback, 
		&VectorFieldsViewer::openFieldCallback);
}

void VectorFieldsViewer::resetColorAndIndices()
{
	int maxActivePathLength = 30;
	colors.resize(maxActivePathLength * 4);
	indices.resize(maxActivePathLength);
	for(uint i = 0; i < maxActivePathLength; ++i)
	{
		indices[i] = i;
		colors[i * 4] = color[0];
		colors[i * 4 + 1] = color[1];
		colors[i * 4 + 2] = color[2];
		colors[i * 4 + 3] = std::sqrt(std::sqrt( ((float) i) / maxActivePathLength)) / 3;
	}
}

VectorFieldsViewer& VectorFieldsViewer::getInstance()
{
	return instance;
}

void VectorFieldsViewer::evolvePaths()
{
	double dt = fieldSimulationTimeInterval;
	int s = particlePaths.size();
#pragma omp parallel for schedule(dynamic, 500)
	for (int i = 0; i < s; i++) 
	{
		particlePaths[i].evolveParticleLoc(dt);
	}
}

void VectorFieldsViewer::computePaths()
{
	particlePaths.clear();
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
