#include <windows.h>
#include "VectorFieldsViewer.h"
#include <vector>
#include <float.h>


_declspec(dllexport) void OpenWindow(void (*timeoutChanged)(int),
									 void (*pathLengthChanged)(int),
									 void (*closedCallback)(void));

VectorFieldsViewer* VectorFieldsViewer::activeInstance = NULL;

void VectorFieldsViewer::OpenParameterWindow()
{
	if(isParameterOpen)
	{
		return;
	}
	OpenWindow(&VectorFieldsViewer::timeoutChanged, &VectorFieldsViewer::maxPathChanged, &VectorFieldsViewer::windowClosed);
	isParameterOpen = true;
}

VectorFieldsViewer::VectorFieldsViewer(const char* _title, int _width, int _height) : 
	GlutExaminer(_title, _width, _height),
	fieldSimulationTimeInterval(0.001),
	maxActivePathLength(10),
	timeout(200),
	isParameterOpen(false),
	color(0.1,1,0.1)
{

	resetColorAndIndices();

	clear_draw_modes();
	add_draw_mode("Wireframe");
	add_draw_mode("Hidden Line");
	add_draw_mode("Solid Flat");
	add_draw_mode("Solid Smooth");
	int vfDrawModeId = add_draw_mode("Vector Field");
	add_draw_mode("Only Lines");
	add_draw_mode("Hidden Field");

	LOAD_GEOMETRY_KEY = add_draw_mode("Load Geometry");
	LOAD_CONST_FIELD_KEY = add_draw_mode("Load Constant Field");
	LOAD_VAR_FIELD_KEY = add_draw_mode("Load Variable Field");

	const char initPath[] = "..\\Data\\miri\\frog\\frog_s5.off";
	//const char initPath[] = "..\\Data\\old\\Horse.off";
	open_mesh(initPath);
	//fieldedMesh.assignVectorField("..\\Data\\miri\\teddy171.vf");
	set_draw_mode(vfDrawModeId);
	VectorFieldsViewer::activeInstance = this;
	//computeVectorFieldLines();
	resetTimer();

	OpenParameterWindow();
}

void VectorFieldsViewer::resetTimer()
{
	glutTimerFunc(timeout, &VectorFieldsViewer::onTimer, 0);
}

void VectorFieldsViewer::evolvePaths()
{
	double dt = fieldSimulationTimeInterval;
	int s = particlePaths.size();
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < s; i++) 
	{
		particlePaths[i].evolveParticleLoc(dt);
	}
}

void VectorFieldsViewer::onTimer(int val)
{
	VectorFieldsViewer::activeInstance->evolvePaths();
	glutPostRedisplay();
	VectorFieldsViewer::activeInstance->resetTimer();
}

// Overriden virtual method - fetch class specific IDs here
void VectorFieldsViewer::processmenu(int i) 
{
	if(LOAD_GEOMETRY_KEY == i)
	{
		OPENFILENAME ofn={0};
		char szFileName[MAX_PATH]={0};
		ofn.lStructSize=sizeof(OPENFILENAME);
		ofn.Flags=OFN_ALLOWMULTISELECT|OFN_EXPLORER;
		ofn.lpstrFilter="OFF Files (*.off)\0*.off\0";
		ofn.lpstrFile=szFileName;
		ofn.nMaxFile=MAX_PATH;
		if(GetOpenFileName(&ofn)) {
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
		OPENFILENAME ofn={0};
		char szFileName[MAX_PATH]={0};
		ofn.lStructSize=sizeof(OPENFILENAME);
		ofn.Flags=OFN_EXPLORER;
		ofn.lpstrFilter="VF Files (*.vf)\0*.vf\0";
		if (!isConstField) 
		{
			ofn.lpstrFilter="TXT Files (*.txt)\0*.txt\0";
		}
		ofn.lpstrFile=szFileName;
		ofn.nMaxFile=MAX_PATH;
		if(GetOpenFileName(&ofn)) {
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
	}
}

bool VectorFieldsViewer::open_mesh(const char* fileName)
{
	if (fieldedMesh.load(fileName))
	{
		std::cout << fieldedMesh.n_vertices() << " vertices, " << fieldedMesh.n_faces()    << " faces\n";
		set_scene( (Vec3f)(fieldedMesh.boundingBoxMin() + fieldedMesh.boundingBoxMax())*0.5,
			0.5*(fieldedMesh.boundingBoxMin() - fieldedMesh.boundingBoxMax()).norm());

		fieldSimulationTimeInterval = (fieldedMesh.maxTime() - fieldedMesh.minTime()) / 100.;

		glutPostRedisplay();
		return true;
	}
	return false;
}

void VectorFieldsViewer::computeVectorFieldLines()
{
	particlePaths.clear();
	bool success = pathFinder.configure(fieldedMesh, fieldSimulationTimeInterval);
	if (success) 
	{
		particlePaths = pathFinder.getParticlePaths();
	}
	else 
	{
		std::cerr << "Failed to properly configure PathFinder" << std::endl;
		particlePaths = vector<ParticlePath>();
	}
}

void VectorFieldsViewer::draw(const std::string& _draw_mode)
{
	if (!fieldedMesh.isLoaded())
	{
		GlutExaminer::draw(_draw_mode);
		return;
	}
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

void VectorFieldsViewer::drawWireframe(Vec3f color)
{
	glDisable(GL_LIGHTING);
	glColor3f(color[0], color[1], color[2]);
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	

	glEnableClientState(GL_VERTEX_ARRAY);
	GL::glVertexPointer(fieldedMesh.points());
	glDrawElements(GL_TRIANGLES, fieldedMesh.getIndices().size(), GL_UNSIGNED_INT, &fieldedMesh.getIndices()[0]);
	glDisableClientState(GL_VERTEX_ARRAY);
	
}

void VectorFieldsViewer::drawSolid(bool isSmooth, bool useLighting, Vec3f color)
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
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	GL::glVertexPointer(fieldedMesh.points());
	GL::glNormalPointer(fieldedMesh.vertex_normals());
	glDrawElements(GL_TRIANGLES, fieldedMesh.getIndices().size(), GL_UNSIGNED_INT, &fieldedMesh.getIndices()[0]);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDepthRange(0, 1.0);
}

void VectorFieldsViewer::drawVectorField()
{
	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_BLEND );
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	for (uint i = 0; i < particlePaths.size(); i++) 
	{
		int visiblePathLen = 0;
		const Point* first = particlePaths[i].getActivePathPoints(maxActivePathLength, &visiblePathLen);
		if(visiblePathLen <= 0)
		{
			continue;
		}
		GL::glVertexPointer(first);
		GL::glColorPointer(4, GL_FLOAT, 0, &colors[0]);
		glDrawElements(GL_LINE_STRIP, visiblePathLen, GL_UNSIGNED_INT, &indices[0]);
	}
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

