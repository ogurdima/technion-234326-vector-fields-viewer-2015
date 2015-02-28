#pragma once

#include <OpenMesh/Core/IO/MeshIO.hh>
#include "MeshViewer.h"

class VectorFieldsWindow : public GlutExaminer
{
public:
	static const VectorFieldsWindow* getInstance();

	static void initInstance(const char* title, int w, int h);

private:
	static VectorFieldsWindow* instance;

	VectorFieldsWindow(const char* _title, int _width, int _height);

	void resetTimer(int timeout);
	static void onTimer(int val);

	static void redrawHandler();
	static void resetSceneHandler();

	//virtual bool open_mesh(const char* _filename);
	virtual void processmenu(int i);

	int LOAD_GEOMETRY_KEY;
	int LOAD_CONST_FIELD_KEY;
	int LOAD_VAR_FIELD_KEY;

protected:
	
	virtual void draw(const std::string& _draw_mode);

	void drawWireframe(Vec3f color = Vec3f(1,1,1)); 
	void drawSolid(bool isSmooth, bool useLighting, Vec3f color = Vec3f(0,0,0)); 
	void drawVectorField(); 

	virtual void keyboard(int key, int x, int y);
	
};
