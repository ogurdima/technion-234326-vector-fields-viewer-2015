#include <windows.h>
#include <chrono>
#include <math.h>
#include "VectorFieldsWindow.h"
#include "VectorFieldsViewer.h"

VectorFieldsWindow* VectorFieldsWindow::instance = NULL;

VectorFieldsWindow::VectorFieldsWindow(const char* _title, int _width, int _height) : 
	GlutExaminer(_title, _width, _height)
{
	glutDetachMenu(GLUT_RIGHT_BUTTON);

	VectorFieldsViewer::getInstance().AddRedrawHandler(&VectorFieldsWindow::redrawHandler);
	VectorFieldsViewer::getInstance().AddResetSceneHandler(&VectorFieldsWindow::resetSceneHandler);
	VectorFieldsViewer::getInstance().openParameterWindow();

	resetTimer();
}

void VectorFieldsWindow::resetTimer()
{
	glutTimerFunc(60, &VectorFieldsWindow::timerCallback, 0);
}

void VectorFieldsWindow::timerCallback(int val)
{
	auto start_time = std::chrono::high_resolution_clock::now();
	VectorFieldsViewer::getInstance().onTimer(val);
	glutPostRedisplay();
	auto end_time = std::chrono::high_resolution_clock::now();
	auto time = end_time - start_time;
	resetTimer();
}

void VectorFieldsWindow::keyboard(int key, int x, int y)
{
	switch (key)
	{
	case 101:
		translate(Vec3f(0.0f, 0.05f, 0.f));
		return;
	case 100:
		translate(Vec3f(-0.05f, 0.0f, 0.0f));
		return;
	case 103:
		translate(Vec3f(0.0f, -0.05f, 0.f));
		return;
	case 102:
		translate(Vec3f(0.05f, 0.0f, 0.0f));
		return;
	case ',':
		translate(Vec3f(0.0f, 0.0f, 0.05f));
		return;
	case '.':
		translate(Vec3f(0.0f, 0.0f, -0.05f));
		return;
	default:
		{
			GlutExaminer::keyboard(key, x, y);
			break;
		}
	}
}

void VectorFieldsWindow::display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch(VectorFieldsViewer::getInstance().getDrawState())
	{
	case(DrawStateType::NONE):
		break;
	case(DrawStateType::WIREFRAME):
		drawWireframe(VectorFieldsViewer::getInstance().getMeshColor());
		break;
	case(DrawStateType::SOLID_FLAT):
		drawSolid(false, true, VectorFieldsViewer::getInstance().getMeshColor());
		break;
	case(DrawStateType::SOLID_SMOOTH):
		drawSolid(true, true, VectorFieldsViewer::getInstance().getMeshColor());
		break;
	case(DrawStateType::FRONT_FIELD):
		drawSolid(true, false, VectorFieldsViewer::getInstance().getMeshColor());
	case(DrawStateType::FIELD):
		drawVectorField();
		break;
	}
	glutSwapBuffers();
}

void VectorFieldsWindow::drawWireframe(const Vec4f& color)
{
	glDisable(GL_LIGHTING);
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	GL::glColor(color);

	const FieldedMesh& fieldedMesh = VectorFieldsViewer::getInstance().getMesh();
	
	glEnableClientState(GL_VERTEX_ARRAY);
	GL::glVertexPointer(fieldedMesh.points());
	glDrawElements(GL_TRIANGLES, fieldedMesh.getIndices().size(), GL_UNSIGNED_INT, &fieldedMesh.getIndices()[0]);
	glDisableClientState(GL_VERTEX_ARRAY);

}

void setShadeMode(bool isSmooth)
{
	if(isSmooth) 
	{
		glShadeModel(GL_SMOOTH);
	}
	else 
	{
		glShadeModel(GL_FLAT);
	}
}

void setLighting(bool useLighting)
{
	if (useLighting) 
	{
		glEnable(GL_LIGHTING);
	}
	else 
	{
		glDisable(GL_LIGHTING);
	}
}

void VectorFieldsWindow::drawSolid(bool isSmooth, bool useLighting, const Vec4f& color)
{
	glDepthRange(0.01, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	setShadeMode(isSmooth);
	setLighting(useLighting);
	
	const FieldedMesh& fieldedMesh = VectorFieldsViewer::getInstance().getMesh();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	GL::glVertexPointer(fieldedMesh.points());
	GL::glNormalPointer(fieldedMesh.vertex_normals());
	GL::glColorPointer(fieldedMesh.getVertexColors());
	glDrawElements(GL_TRIANGLES, fieldedMesh.getIndices().size(), GL_UNSIGNED_INT, &fieldedMesh.getIndices()[0]);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDepthRange(0, 1.0);
}

void VectorFieldsWindow::drawVectorField()
{
	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_BLEND );
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	float* dataArray;
	unsigned int** indices;
	unsigned int* counts;
	unsigned int* starts;
	unsigned int primCount;

	VectorFieldsViewer::getInstance().GetCurrentPaths(dataArray, starts, counts, primCount);
	glVertexPointer(3, GL_FLOAT, 8 * sizeof(float), dataArray);
	glColorPointer(4, GL_FLOAT, 8 * sizeof(float), dataArray + 4);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glMultiDrawArrays(GL_LINE_STRIP, (GLint*) starts, (GLint*) counts, primCount);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
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