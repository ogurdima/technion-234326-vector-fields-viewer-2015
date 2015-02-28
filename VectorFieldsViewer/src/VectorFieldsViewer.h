#pragma once

#include <string>
#include "FieldedMesh.h"
#include "PathFinder.h"

using std::string;

_declspec(dllexport) void OpenWindow(
	void (*changedRangeCallback)(double),				// 
	void (*openMeshCallback)(std::string),				// 
	void (*openFieldCallback)(std::string, bool));		// 

class VectorFieldsViewer
{
private:
	VectorFieldsViewer(void);
	~VectorFieldsViewer(void);

	static VectorFieldsViewer instance;
	// callbacks
public:
	static void				onTimer(int val);
private:
	static void				changedRangeCallback(double range);
	static void				openMeshCallback(string path);
	static void				openFieldCallback(string path, bool isConst);
	// callback handlers
	void					openField(string path, bool isConst);
	void					openMesh(string path);
	void					changeRange(double range);

	// events
	void					(*redrawEvent)(void);
	void					(*resetSceneEvent)(void);

	// members
	Vec3f					color;
	double					fieldSimulationTimeInterval;

	FieldedMesh				fieldedMesh;
	PathFinder				pathFinder;

	vector<uint>			indices;
	vector<float>			colors;

	vector<ParticlePath>	particlePaths;

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
	const FieldedMesh&				mesh();

};

