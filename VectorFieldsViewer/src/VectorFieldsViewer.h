#pragma once

#include <string>
#include "FieldedMesh.h"
#include "PathFinder.h"

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

_declspec(dllexport) void OpenWindow(
	void (*changedRangeCallback)(double),
	void (*changedDrawStateCallback)(int),
	void (*changedMeshColorCallback)(float,float,float,float),
	void (*changedFieldColorCallback)(float,float,float,float),
	void (*openMeshCallback)(std::string),
	void (*openFieldCallback)(std::string, bool));

class VectorFieldsViewer
{
#pragma region Singleton
private:
	VectorFieldsViewer(void);
	~VectorFieldsViewer(void);

	static VectorFieldsViewer instance;
#pragma endregion

#pragma region Callbacks
private:
	static void				changedRangeCallback(double range);
	static void				openMeshCallback(string path);
	static void				openFieldCallback(string path, bool isConst);
	static void				changedDrawStateCallback(int);
	static void				changedMeshColorCallback(float,float,float,float);
	static void				changedFieldColorCallback(float,float,float,float);
	// callback handlers
	void					openField(string path, bool isConst);
	void					openMesh(string path);
	void					changeRange(double range);
	// events
	void					(*redrawEvent)(void);
	void					(*resetSceneEvent)(void);
#pragma endregion

#pragma region Fields
private:
	DrawStateType			drawState;
	Vec3f					color;
	double					fieldSimulationTimeInterval;

	FieldedMesh				fieldedMesh;
	PathFinder				pathFinder;
	vector<ParticlePath>	particlePaths;

	vector<uint>			indices;
	vector<float>			colors;
#pragma endregion

private:
	void					computePaths();
	void					evolvePaths();
	void					resetColorAndIndices();
	void					openParameterWindow();


public:
	// singleton
	static VectorFieldsViewer&		getInstance();

	// events
	void							AddRedrawHandler(void (*redrawCallback)(void));
	void							AddResetSceneHandler(void (*resetSceneCallback)(void));

	// api
	const FieldedMesh&				getMesh();
	void							onTimer(int val);
	DrawStateType					getDrawState();

};

