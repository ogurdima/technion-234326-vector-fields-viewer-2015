#pragma once

#include <OpenMesh/Core/IO/MeshIO.hh>
#include "MeshViewer.h"

class VectorFieldsWindow : public GlutExaminer
{
private:
	static VectorFieldsWindow*			instance;
public:
	static const VectorFieldsWindow*	getInstance();
	static void							initInstance(const char* title, int w, int h);

private:
										VectorFieldsWindow(const char* _title, int _width, int _height);

	static void							resetTimer();
	static void							timerCallback(int val);

	static void							redrawHandler();
	static void							resetSceneHandler();

	virtual void						drawWireframe(const Vec4f& color = Vec4f(1,1,1,1)); 
	virtual void						drawSolid(bool isSmooth, bool useLighting, const Vec4f& color = Vec4f(0,0,0)); 
	virtual void						drawVectorField(); 
	virtual void						keyboard(int key, int x, int y);

protected:
	virtual void						display(void);

};
