#pragma once

#pragma warning( push )
#pragma warning( disable : 4101)
#include "VectorFieldsUtils.h"
#include "MeshViewer.h"
#include "PathFinder.h"
#include "FieldedMesh.h"
#include "ParticlePath.h"
#pragma warning( pop ) 


class VectorFieldsViewer : public GlutExaminer
{
public:
	/*Default Constructor*/		VectorFieldsViewer(const char* _title, int _width, int _height);
	virtual bool				open_mesh(const char* _filename);
	virtual void				processmenu(int i);

protected:
#pragma region Methods
	static VectorFieldsViewer*	activeInstance;
	static void					onTimer(int val);
	virtual void				draw(const std::string& _draw_mode);
	virtual void				drawWireframe(Vec3f color = Vec3f(1,1,1)); 
	virtual void				drawSolid(bool isSmooth, bool useLighting, Vec3f color = Vec3f(0,0,0)); 
	virtual void				drawVectorField(); 
	virtual void				keyboard(int key, int x, int y);
#pragma endregion

private:
#pragma region Data
	bool					isParameterOpen;
	Vec3f					color;
	vector<ParticlePath>	particlePaths;
	FieldedMesh				fieldedMesh;
	PathFinder				pathFinder;
	float					fieldSimulationTimeInterval;
	vector<uint>			indices;
	vector<float>			colors;
	uint					maxActivePathLength;
	uint					timeout;
#pragma endregion

#pragma region Menu Keys
	int WIREFRAME_KEY;
	int HIDDEN_LINE_KEY;
	int SOLID_FLAT_KEY;
	int SOLID_SMOOTH_KEY;
	int VECTOR_FIELDS_KEY;
	int ONLY_LINES_KEY;
	int HIDDEN_FIELD_KEY;
	int LOAD_GEOMETRY_KEY;
	int LOAD_CONST_FIELD_KEY;
	int LOAD_VAR_FIELD_KEY;
#pragma endregion

#pragma region Methods
	static void				timeoutChanged(int newVal)
	{
		activeInstance->timeout = newVal;

	}
	static void				maxPathChanged(int newVal)
	{
		activeInstance->maxActivePathLength = newVal;
		activeInstance->resetColorAndIndices();
	}
	void					resetColorAndIndices()
	{
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
	static void				windowClosed()
	{
		activeInstance->isParameterOpen = false;
	}
	void					OpenParameterWindow();
	void					evolvePaths();
	void					resetTimer();
	void					initContextMenu();
	void					initDefaultData();
	void					computeVectorFieldLines();
#pragma endregion

};


