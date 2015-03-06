#pragma once

#include <OpenMesh/Core/IO/MeshIO.hh>
#include "MeshViewer.h"

class VectorFieldsWindow : public GlutExaminer
{
private:
	static VectorFieldsWindow* instance;
public:
	static const VectorFieldsWindow* getInstance();

	static void initInstance(const char* title, int w, int h);

private:
	VectorFieldsWindow(const char* _title, int _width, int _height);

	static void resetTimer();
	static void timerCallback(int val);

	static void redrawHandler();
	static void resetSceneHandler();
protected:
	virtual void				draw(const std::string& _draw_mode);
	virtual void				display(void);
private:
	virtual void				drawWireframe(const Vec4f color = Vec4f(1,1,1,1)); 
	virtual void				drawSolid(bool isSmooth, bool useLighting, const Vec4f color = Vec4f(0,0,0)); 
	virtual void				drawVectorField(); 
	virtual void				keyboard(int key, int x, int y);

	//virtual bool open_mesh(const char* _filename);
	//virtual void processmenu(int i);
private:
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

protected:
#pragma region Methods
	void					initContextMenu();
	void					initDefaultData();
#pragma endregion
	
};
