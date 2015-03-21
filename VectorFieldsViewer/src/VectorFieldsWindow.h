#pragma once

#include <OpenMesh/Core/IO/MeshIO.hh>
#include "MeshViewer.h"

class VectorFieldsWindow : public GlutExaminer
{
	VectorFieldsWindow(const char* _title, int _width, int _height);
	static VectorFieldsWindow*			instance;

	static void							resetTimer();
	static void							timerCallback(int val);

	static void							redrawHandler();
	static void							resetSceneHandler();

	void								drawWireframe();
	void								drawVectorField();
	void								drawSolid(bool isSmooth, bool useLighting); 

protected:
	virtual void						display(void);
	virtual void						keyboard(int key, int x, int y);

public:
	static const VectorFieldsWindow*	getInstance();
	static void							initInstance(const char* title, int w, int h);
};
