#pragma once

#include <string>
#include "FieldedMesh.h"
#include "PathFinder.h"
#include "PathsManager.h"
#include "External.h"

using std::string;

typedef enum DrawStateType
{
	WIREFRAME		= 1,
	SOLID_FLAT		= 2,
	SOLID_SMOOTH	= 3,
	FRONT_FIELD		= 4,
	FIELD			= 5,
} DrawStateType;

class VectorFieldsViewer
{
#pragma region Singleton
private:
	VectorFieldsViewer();
	static VectorFieldsViewer		instance;
	static int						drawingTimeout;
#pragma endregion

#pragma region Callbacks
private:
	static void						changedDrawStateCallback(int);
	static void						openMeshCallback(char* path);
	static void						changedMeshColorCallback(float,float,float,float);
	static void						changedFieldColorCallback(float,float,float,float);
	static void						changedVisualizationCallback(int drawTimeout, double step, double window);
	static void						recomputePathsCallback(char* path, bool isConst, bool normalize, double step, double min, double max);
	static void						takeScreenshotsCallback(int howMany);
	// callback handlers
	void							openField(char* path, bool isConst);
	void							openMesh(char* path);
	// events
	void							(*redrawEvent)(void);
	void							(*resetSceneEvent)(void);
	void							(*printScreenEvent)(string);
#pragma endregion

#pragma region Fields
private:
	DrawStateType					drawState;
	Time							curTime;
	Time							maxTime;
	Time							minTime;
	Time							visualisationTimeInterval;
	FieldedMesh						fieldedMesh;
	PathsManager					pathsMgr;
	vector<uint>					indices;
	vector<float>					colors;
	string							loadedFieldPath;
	bool							isVisualizationStopped;
#pragma endregion

private:
	void							computePaths(double step);
	void							evolvePaths();
	void							setDrawState(DrawStateType ds);

public:
#pragma region Static
	static VectorFieldsViewer&		getInstance();
	static int						getDrawingTimeout();
#pragma endregion

#pragma region Handlers
	void							AddRedrawHandler(void (*redrawCallback)(void));
	void							AddResetSceneHandler(void (*resetSceneCallback)(void));  
	void							AddPrintScreenHandler(void (*printScreenCallback)(std::string));
#pragma endregion

#pragma region API
	void							GetCurrentPaths(float*& dataArray, unsigned int*& starts, unsigned int*& counts, unsigned int& pathCount);
	const FieldedMesh&				getMesh();
	void							onTimer(int val);
	void							openParameterWindow();
	DrawStateType					getDrawState();  
	void							takeScreenshots(int howMany);
#pragma endregion

};
