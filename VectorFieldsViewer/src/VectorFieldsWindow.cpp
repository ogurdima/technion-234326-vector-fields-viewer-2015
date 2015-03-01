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
	clear_draw_modes();
	initDefaultData();
	//set_draw_mode(HIDDEN_FIELD_KEY);

	resetTimer();
}

void VectorFieldsWindow::initContextMenu()
{
	clear_draw_modes();
	/*WIREFRAME_KEY			= add_draw_mode("Wireframe");
	HIDDEN_LINE_KEY			= add_draw_mode("Hidden Line");
	SOLID_FLAT_KEY			= add_draw_mode("Solid Flat");
	SOLID_SMOOTH_KEY		= add_draw_mode("Solid Smooth");
	VECTOR_FIELDS_KEY		= add_draw_mode("Vector Field");
	ONLY_LINES_KEY			= add_draw_mode("Only Lines");
	HIDDEN_FIELD_KEY		= add_draw_mode("Hidden Field");
	LOAD_GEOMETRY_KEY		= add_draw_mode("Load Geometry");
	LOAD_CONST_FIELD_KEY	= add_draw_mode("Load Constant Field");
	LOAD_VAR_FIELD_KEY		= add_draw_mode("Load Variable Field");*/
}

void VectorFieldsWindow::initDefaultData()
{
	//const char initPath[] = "..\\Data\\miri\\teddy171.off";
	//const char initPath[] = "..\\Data\\miri\\frog\\frog_s5.off";
	const char initPath[] = "..\\Data\\old\\Horse.off";
	//open_mesh(initPath);
	//fieldedMesh.assignVectorField("..\\Data\\miri\\frog\\frog_s5_times.txt", false);

}

void VectorFieldsWindow::resetTimer()
{
	glutTimerFunc(40, &VectorFieldsWindow::timerCallback, 0);
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

//void VectorFieldsWindow::processmenu(int i) 
//{
//	if(LOAD_GEOMETRY_KEY == i)
//	{
//		OPENFILENAME ofn={0};
//		char szFileName[MAX_PATH]={0};
//		ofn.lStructSize=sizeof(OPENFILENAME);
//		ofn.Flags=OFN_ALLOWMULTISELECT|OFN_EXPLORER;
//		ofn.lpstrFilter="OFF Files (*.off)\0*.off\0";
//		ofn.lpstrFile=szFileName;
//		ofn.nMaxFile=MAX_PATH;
//		if(GetOpenFileName(&ofn)) {
//			particlePaths.clear();
//			std::cout << "Opening Mesh File " << ofn.lpstrFile << std::endl;
//			bool success = open_mesh(szFileName);
//			
//			if (!success)
//			{
//				std::cout << "Failed to read mesh" << std::endl;
//			}
//			else
//			{
//				computeVectorFieldLines();
//			}
//		}
//	}
//	else if (LOAD_CONST_FIELD_KEY == i || LOAD_VAR_FIELD_KEY == i)
//	{
//		bool isConstField = (LOAD_CONST_FIELD_KEY == i);
//		OPENFILENAME ofn = {0};
//		char szFileName[MAX_PATH] = {0};
//		ofn.lStructSize = sizeof(OPENFILENAME);
//		ofn.Flags = OFN_EXPLORER;
//		ofn.lpstrFilter = isConstField ? "VF Files (*.vf)\0*.vf\0" : "TXT Files (*.txt)\0*.txt\0";
//		
//		ofn.lpstrFile = szFileName;
//		ofn.nMaxFile = MAX_PATH;
//		if(GetOpenFileName(&ofn)) 
//		{
//			std::cout << "Opening Field File " << ofn.lpstrFile << std::endl;
//			bool success = fieldedMesh.assignVectorField(szFileName, isConstField);
//			if (!success) 
//			{
//				std::cout << "Failed to read field" << std::endl;
//			}
//			else
//			{
//				computeVectorFieldLines();
//			}
//		}
//	}
//	else 
//	{
//		set_draw_mode(i);
//	}
//}

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
			drawSolid(true, false, Vec3f(0.3f,0.3f,0.3f));
			drawWireframe(Vec3f(0.5f,0.5f,0.2f));
		}
		else if (_draw_mode == "Hidden Field")
		{
			drawSolid(true, false, Vec3f(0.05f, 0.05f, 0.05f));
		}
		drawVectorField();
	}
	else // call parent method
	{
		GlutExaminer::draw(_draw_mode);
	}
}

void VectorFieldsWindow::display()
{
	switch(VectorFieldsViewer::getInstance().getDrawState())
	{
	case(DrawStateType::NONE):
		return;
	case(DrawStateType::WIREFRAME):
		drawWireframe();
		return;
	case(DrawStateType::SOLID_FLAT):
		drawSolid(false, true);
		return;
	case(DrawStateType::SOLID_SMOOTH):
		drawSolid(true, true);
		return;
	case(DrawStateType::FRONT_FIELD):
		drawSolid(true, false, Vec3f(0.3f,0.3f,0.3f));
	case(DrawStateType::FIELD):
		drawVectorField();
		return;
	}

	//else if (_draw_mode == "Hidden Line")
	//{
	//	drawSolid(true, false, Vec3f(0,0,0));
	//	drawWireframe();
	//}

	//else if (_draw_mode == "Only Lines" || _draw_mode == "Vector Field" || _draw_mode == "Hidden Field" )
	//{
	//	if (_draw_mode == "Vector Field")
	//	{
	//		drawSolid(true, false, Vec3f(0.3f,0.3f,0.3f));
	//		drawWireframe(Vec3f(0.5f,0.5f,0.2f));
	//	}
	//	else if (_draw_mode == "Hidden Field")
	//	{
	//		drawSolid(true, false, Vec3f(0.05f, 0.05f, 0.05f));
	//	}
	//	drawVectorField();
	//}
	//else // call parent method
	//{
	//	GlutExaminer::draw(_draw_mode);
	//}
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

	vector<int> startIndices;
	vector<int> pathLenghts;
	vector<Point> paths;
	vector<float> tmpColors;
	//paths.resize(particlePaths.size());
	vector<ParticlePath> particlePaths;
	VectorFieldsViewer::getInstance();
	int pathIdx = 0;
	for (uint i = 0; i < particlePaths.size(); i++) 
	{
		int visiblePathLen = 0;
		const Point* first = particlePaths[i].getActivePathPoints(20, &visiblePathLen);
		if(visiblePathLen <= 0)
		{
			continue;
		}
		startIndices.push_back(pathIdx);
		for (int j = 0; j < visiblePathLen; j++)
		{
			paths.push_back(first[j]);
			pathIdx++;
			tmpColors.push_back(1); tmpColors.push_back(1); tmpColors.push_back(1); tmpColors.push_back(1);
		}
		pathLenghts.push_back(visiblePathLen);
	}
	
	assert(pathLenghts.size() == startIndices.size());
	assert(4 * paths.size() == tmpColors.size());

	glColor3f(1,1,1);
	GL::glVertexPointer(&paths[0]);
	GL::glColorPointer(4, GL_FLOAT, 0, &tmpColors[0]);
	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_COLOR_ARRAY);
	
	glMultiDrawArrays(GL_LINE_STRIP, &startIndices[0], &pathLenghts[0], pathLenghts.size());
	//glMultiDrawArrays(
	//glDisableClientState(GL_COLOR_ARRAY);
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