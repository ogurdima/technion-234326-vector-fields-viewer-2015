#include <windows.h>
#include <chrono>
#include <math.h>
#include "VectorFieldsWindow.h"
#include "VectorFieldsViewer.h"

VectorFieldsWindow* VectorFieldsWindow::instance = NULL;

VectorFieldsWindow::VectorFieldsWindow(const char* _title, int _width, int _height) : 
	GlutExaminer(_title, _width, _height)
{
	VectorFieldsViewer::getInstance().AddRedrawHandler(&VectorFieldsWindow::redrawHandler);
	
	clear_draw_modes();
	add_draw_mode("Wireframe");
	add_draw_mode("Hidden Line");
	add_draw_mode("Solid Flat");
	add_draw_mode("Solid Smooth");
	int vfDrawModeId = add_draw_mode("Vector Field");
	add_draw_mode("Only Lines");
	add_draw_mode("Hidden Field");

	set_draw_mode(vfDrawModeId);

	LOAD_GEOMETRY_KEY = add_draw_mode("Load Geometry");
	LOAD_CONST_FIELD_KEY = add_draw_mode("Load Constant Field");
	LOAD_VAR_FIELD_KEY = add_draw_mode("Load Variable Field");

	//const char initPath[] = "..\\Data\\miri\\teddy171.off";
	const char initPath[] = "..\\Data\\miri\\frog\\frog_s5.off";
	//const char initPath[] = "..\\Data\\old\\Horse.off";
	//open_mesh(initPath);

	resetTimer(60);
}

void VectorFieldsWindow::resetTimer(int timeout)
{
	glutTimerFunc(timeout, &VectorFieldsWindow::onTimer, 0);
}

void VectorFieldsWindow::onTimer(int val)
{
	auto start_time = std::chrono::high_resolution_clock::now();
	VectorFieldsViewer::getInstance().onTimer(val);
	glutPostRedisplay();
	auto end_time = std::chrono::high_resolution_clock::now();
	auto time = end_time - start_time;
	instance->resetTimer(40);
}

void VectorFieldsWindow::processmenu(int i) 
{
	/*if(LOAD_GEOMETRY_KEY == i)
	{
		OPENFILENAME ofn={0};
		char szFileName[MAX_PATH]={0};
		ofn.lStructSize=sizeof(OPENFILENAME);
		ofn.Flags=OFN_ALLOWMULTISELECT|OFN_EXPLORER;
		ofn.lpstrFilter="OFF Files (*.off)\0*.off\0";
		ofn.lpstrFile=szFileName;
		ofn.nMaxFile=MAX_PATH;
		if(GetOpenFileName(&ofn)) {
			particlePaths.clear();
			std::cout << "Opening Mesh File " << ofn.lpstrFile << std::endl;
			bool success = open_mesh(szFileName);

			if (!success)
			{
				std::cout << "Failed to read mesh" << std::endl;
			}
			else
			{
				computeVectorFieldLines();
			}
		}
	}
	else if (LOAD_CONST_FIELD_KEY == i || LOAD_VAR_FIELD_KEY == i)
	{
		bool isConstField = (LOAD_CONST_FIELD_KEY == i);
		OPENFILENAME ofn = {0};
		char szFileName[MAX_PATH] = {0};
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.Flags = OFN_EXPLORER;
		ofn.lpstrFilter = isConstField ? "VF Files (*.vf)\0*.vf\0" : "TXT Files (*.txt)\0*.txt\0";

		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		if(GetOpenFileName(&ofn)) 
		{
			std::cout << "Opening Field File " << ofn.lpstrFile << std::endl;
			bool success = fieldedMesh.assignVectorField(szFileName, isConstField);
			if (!success) 
			{
				std::cout << "Failed to read field" << std::endl;
			}
			else
			{
				computeVectorFieldLines();
			}
		}
	}
	else 
	{
		set_draw_mode(i);
	}*/
}

void VectorFieldsWindow::keyboard(int key, int x, int y)
{
	switch (key)
	{
	case 101:
		translate(Vec3f(0.0, 0.05, 0.));
		return;
	case 100:
		translate(Vec3f(-0.05, 0.0, 0.0));
		return;
	case 103:
		translate(Vec3f(0.0, -0.05, 0.));
		return;
	case 102:
		translate(Vec3f(0.05, 0.0, 0.0));
		return;
	case ',':
		translate(Vec3f(0.0, 0.0, 0.05));
		return;
	case '.':
		translate(Vec3f(0.0, 0.0, -0.05));
		return;
	default:
		{

			GlutExaminer::keyboard(key, x, y);
			break;
		}
	}
}

void VectorFieldsWindow::draw(const std::string& _draw_mode)
{
	if (_draw_mode == "Wireframe")
	{
		drawWireframe();
	}
	else if (_draw_mode == "Hidden Line")
	{
		drawSolid(true, false, Vec3f(0,0,0));
		drawWireframe();
	}
	else if (_draw_mode == "Solid Flat")
	{
		drawSolid(false, true);
	}
	else if (_draw_mode == "Solid Smooth")
	{
		drawSolid(true, true);
	} 
	else if (_draw_mode == "Only Lines" || _draw_mode == "Vector Field" || _draw_mode == "Hidden Field" )
	{
		if (_draw_mode == "Vector Field")
		{
			drawSolid(true, false, Vec3f(0.3,0.3,0.3));
			drawWireframe(Vec3f(0.5,0.5,0.2));
		}
		else if (_draw_mode == "Hidden Field")
		{
			drawSolid(true, false, Vec3f(0.05, 0.05, 0.05));
		}
		drawVectorField();
	}
	else // call parent method
	{
		GlutExaminer::draw(_draw_mode);
	}
}

void VectorFieldsWindow::drawWireframe(Vec3f color)
{
	glDisable(GL_LIGHTING);
	glColor3f(color[0], color[1], color[2]);
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	const FieldedMesh& fieldedMesh = VectorFieldsViewer::getInstance().getMesh();
	glEnableClientState(GL_VERTEX_ARRAY);
	GL::glVertexPointer(fieldedMesh.points());
	glDrawElements(GL_TRIANGLES, fieldedMesh.getIndices().size(), GL_UNSIGNED_INT, &fieldedMesh.getIndices()[0]);
	glDisableClientState(GL_VERTEX_ARRAY);

}

void VectorFieldsWindow::drawSolid(bool isSmooth, bool useLighting, Vec3f color)
{
	glDepthRange(0.01, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(isSmooth) {
		glShadeModel(GL_SMOOTH);
	}
	else {
		glShadeModel(GL_FLAT);
	}

	if (useLighting) {
		glEnable(GL_LIGHTING);
	}
	else {
		glDisable(GL_LIGHTING);
		glColor3f(color[0], color[1], color[2]);
	}
	const FieldedMesh& fieldedMesh = VectorFieldsViewer::getInstance().getMesh();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	GL::glVertexPointer(fieldedMesh.points());
	GL::glNormalPointer(fieldedMesh.vertex_normals());
	glDrawElements(GL_TRIANGLES, fieldedMesh.getIndices().size(), GL_UNSIGNED_INT, &fieldedMesh.getIndices()[0]);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDepthRange(0, 1.0);
}

void VectorFieldsWindow::drawVectorField()
{
	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_BLEND );
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	Point ** points;
	int ** indices;
	int * sizes;
	float ** colors;
	int pathNumber;
	//VectorFieldsViewer::getInstance().paths(points, indices, colors, sizes, pathNumber);
	for (uint i = 0; i < pathNumber; i++) 
	{
		GL::glVertexPointer(points[i]);
		GL::glColorPointer(4, GL_FLOAT, 0, colors[i]);
		glDrawElements(GL_LINE_STRIP, sizes[i], GL_UNSIGNED_INT, indices[i]);
	}
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void VectorFieldsWindow::initInstance(const char* title, int w, int h)
{
	if(instance != NULL)
	{
		delete instance;
	}
	instance = new VectorFieldsWindow(title, w, h);
}

void VectorFieldsWindow::redrawHandler()
{
	glutPostRedisplay();
}

void VectorFieldsWindow::resetSceneHandler()
{
	if(instance == NULL)
	{
		return;
	}
	Vec3f l(-1.), r(1.);
	VectorFieldsWindow::instance->set_scene( (l + r) * 0.5, (l - r).norm() * 0.5);
}

const VectorFieldsWindow* VectorFieldsWindow::getInstance()
{
	return instance;
}
