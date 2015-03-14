#pragma once

#include <string>
#include "FieldedMesh.h"
#include "PathFinder.h"
#include "PathsManager.h"

using std::string;

typedef enum DrawStateType
{
	NONE			= 0,
	WIREFRAME		= 1,
	SOLID_FLAT		= 2,
	SOLID_SMOOTH	= 3,
	FRONT_FIELD		= 4,
	FIELD			= 5,
} DrawStateType;

_declspec(dllexport) void OpenWindow(	void (*changedDrawingTimeout)(int),
										void (*changedDrawStateCallback)(int),
										void (*changedMeshColorCallback)(float,float,float,float),
										void (*changedFieldColorCallback)(float,float,float,float),
										void (*openMeshCallback)(char*),
										void (*openFieldCallback)(char*, bool),
										void (*changedPathWindowCallback)(double),
										void (*changedSimulationStepCallback)(double),
										void (*changedVisualizationStepCallback)(double),
										void (*recomputePathsCallback)(void));

_declspec(dllexport) void UpdatePathWindow(double pathWindow);
_declspec(dllexport) void UpdateSimulationStep(double simulationStep);
_declspec(dllexport) void UpdateVisualizationStep(double visualizationStep);

class VectorFieldsViewer
{
#pragma region Singleton
private:
	VectorFieldsViewer(void);
	~VectorFieldsViewer(void);

	static VectorFieldsViewer		instance;
	static int						drawingTimeout;
#pragma endregion

#pragma region Callbacks
private:
	static void						changeDrawingTimeout(int timeout);
	static void						openMeshCallback(char* path);
	static void						openFieldCallback(char* path, bool isConst);
	static void						changedDrawStateCallback(int);
	static void						changedMeshColorCallback(float,float,float,float);
	static void						changedFieldColorCallback(float,float,float,float);
	static void						changedPathWindowCallback(double val);
	static void						changedSimulationStepCallback(double val);
	static void						changedVisualizationStepCallback(double val);
	static void						recomputePathsCallback();
	// callback handlers
	void							openField(char* path, bool isConst);
	void							openMesh(char* path);
	// events
	void							(*redrawEvent)(void);
	void							(*resetSceneEvent)(void);
#pragma endregion

#pragma region Fields
private:
	DrawStateType					drawState;
	Vec4f							meshColor;
	Vec4f							fieldColor;

	Time							curTime;
	Time							maxTime;
	Time							minTime;
	Time							fieldSimulationTimeInterval;
	Time							visualisationTimeInterval;

	FieldedMesh						fieldedMesh;
	PathFinder						pathFinder;
	PathsManager					pathsMgr;

	vector<uint>					indices;
	vector<float>					colors;
#pragma endregion

private:
	void							computePaths();
	void							evolvePaths();
	void							resetColorAndIndices();
	


public:
	// singleton
	static VectorFieldsViewer&		getInstance();

	// events
	void							AddRedrawHandler(void (*redrawCallback)(void));
	void							AddResetSceneHandler(void (*resetSceneCallback)(void));

	// api
	static int						getDrawingTimeout();

	void							GetCurrentPaths(float*& dataArray, unsigned int*& starts, unsigned int*& counts, unsigned int& pathCount);
	const FieldedMesh&				getMesh();
	const Vec4f&					getMeshColor();
	const Vec4f&					getFieldColor();
	void							onTimer(int val);
	DrawStateType					getDrawState();
	void							openParameterWindow();

};

