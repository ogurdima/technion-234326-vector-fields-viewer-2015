#include <windows.h>
#include "VectorFieldsViewer.h"
#include <vector>
#include <float.h>

VectorFieldsViewer* VectorFieldsViewer::activeInstance = NULL;

VectorFieldsViewer::VectorFieldsViewer(const char* _title, int _width, int _height) : 
GlutExaminer(_title, _width, _height),
	fieldSimulationTimeInterval(0.002),
fieldSimulationMinTime(0),
	fieldSimulationMaxTime(0.02),
maxActivePathLength(10),
timeout(200)
{
	float r = 1;
	float g = 1;
	float b = 1;
	colors.reserve(maxActivePathLength * 4);
	indices.reserve(maxActivePathLength);
	for(uint i = 0; i < maxActivePathLength; ++i)
	{
		indices.push_back(i);
		colors.push_back(r);
		colors.push_back(g);
		colors.push_back(b);
		colors.push_back( std::sqrt(std::sqrt( ((float) i) / maxActivePathLength)) / 3);
	}

	clear_draw_modes();
	add_draw_mode("Wireframe");
	add_draw_mode("Hidden Line");
	add_draw_mode("Solid Flat");
	add_draw_mode("Solid Smooth");
	int vfDrawModeId = add_draw_mode("Vector Field");
	add_draw_mode("Only Lines");
	
	LOAD_GEOMETRY_KEY = add_draw_mode("Load Geometry");
	LOAD_FIELD_KEY = add_draw_mode("Load Field");

	//const char initPath[] = "..\\Data\\miri\\cat.off";
	const char initPath[] = "..\\Data\\miri\\teddy171.off";
	open_mesh(initPath);
	set_draw_mode(vfDrawModeId);
	VectorFieldsViewer::activeInstance = this;
	resetTimer();
}

void VectorFieldsViewer::resetTimer()
{
	glutTimerFunc(timeout, &VectorFieldsViewer::onTimer, 0);
}

void VectorFieldsViewer::evolvePaths()
{
	double dt = fieldSimulationTimeInterval * 2;
	int s = particlePaths.size();
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
			if (!success) {
				std::cout << "Failed to read mesh" << std::endl;
			}
		}
	}
	else if (LOAD_FIELD_KEY == i)
	{
		OPENFILENAME ofn={0};
		char szFileName[MAX_PATH]={0};
		ofn.lStructSize=sizeof(OPENFILENAME);
		ofn.Flags=OFN_ALLOWMULTISELECT|OFN_EXPLORER;
		ofn.lpstrFilter="VF Files (*.vf)\0*.vf\0";
		ofn.lpstrFile=szFileName;
		ofn.nMaxFile=MAX_PATH;
		if(GetOpenFileName(&ofn)) {
			std::cout << "Opening Field File " << ofn.lpstrFile << std::endl;
			bool success = fieldedMesh.assignVectorField(szFileName);
			if (!success) {
				std::cout << "Failed to read field" << std::endl;
			}
			else {
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
		computeVectorFieldLines();
		glutPostRedisplay();
		return true;
	}
	return false;
}

void VectorFieldsViewer::computeVectorFieldLines()
{
	bool success = pathFinder.configure(fieldedMesh, fieldSimulationTimeInterval, fieldSimulationMinTime, fieldSimulationMaxTime);
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
		glDisable(GL_LIGHTING);
		glColor3f(1.0, 1.0, 1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glEnableClientState(GL_VERTEX_ARRAY);
		GL::glVertexPointer(fieldedMesh.points());

		glDrawElements(GL_TRIANGLES, fieldedMesh.getIndices().size(), GL_UNSIGNED_INT, &fieldedMesh.getIndices()[0]);

		glDisableClientState(GL_VERTEX_ARRAY);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else if (_draw_mode == "Hidden Line")
	{

		glDisable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
		glColor3f(0.0, 0.0, 0.0);

		glEnableClientState(GL_VERTEX_ARRAY);
		GL::glVertexPointer(fieldedMesh.points());

		glDepthRange(0.01, 1.0);
		glDrawElements(GL_TRIANGLES, fieldedMesh.getIndices().size(), GL_UNSIGNED_INT, &fieldedMesh.getIndices()[0]);
		glDisableClientState(GL_VERTEX_ARRAY);
		glColor3f(1.0, 1.0, 1.0);

		glEnableClientState(GL_VERTEX_ARRAY);
		GL::glVertexPointer(fieldedMesh.points());

		glDrawBuffer(GL_BACK);
		glDepthRange(0.0, 1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDepthFunc(GL_LEQUAL);
		glDrawElements(GL_TRIANGLES, fieldedMesh.getIndices().size(), GL_UNSIGNED_INT, &fieldedMesh.getIndices()[0]);

		glDisableClientState(GL_VERTEX_ARRAY);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDepthFunc(GL_LESS);

	}
	else if (_draw_mode == "Solid Flat")
	{
		Mesh::ConstFaceIter        f_it(fieldedMesh.faces_begin()),  f_end(fieldedMesh.faces_end());
		Mesh::ConstFaceVertexIter  fv_it;

		glEnable(GL_LIGHTING);
		glShadeModel(GL_FLAT);

		glBegin(GL_TRIANGLES);
		for (; f_it!=f_end; ++f_it)
		{
			GL::glNormal(fieldedMesh.normal(f_it));
			fv_it = fieldedMesh.cfv_iter(f_it.handle()); 
			GL::glVertex(fieldedMesh.point(fv_it));
			++fv_it;
			GL::glVertex(fieldedMesh.point(fv_it));
			++fv_it;
			GL::glVertex(fieldedMesh.point(fv_it));
		}
		glEnd();
	}
	else if (_draw_mode == "Solid Smooth")
	{
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		GL::glVertexPointer(fieldedMesh.points());
		GL::glNormalPointer(fieldedMesh.vertex_normals());

		glDrawElements(GL_TRIANGLES, fieldedMesh.getIndices().size(), GL_UNSIGNED_INT, &fieldedMesh.getIndices()[0]);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
	} else
	if (_draw_mode == "Only Lines" || _draw_mode == "Vector Field")
	{
		glDisable(GL_LIGHTING);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable( GL_BLEND );

		glClearColor(0.0,0.0,0.0,0.0);

		if (_draw_mode == "Vector Field")
		{
			glDisable(GL_LIGHTING);
			glShadeModel(GL_SMOOTH);
			glColor3f(0.3, 0.3, 0.3);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			GL::glVertexPointer(fieldedMesh.points());
			GL::glNormalPointer(fieldedMesh.vertex_normals());
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthRange(0.01, 1.0);
			glDrawElements(GL_TRIANGLES, fieldedMesh.getIndices().size(), GL_UNSIGNED_INT, &fieldedMesh.getIndices()[0]);
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);

			glColor3f(0.5, 0.2, 0.2);
			glEnableClientState(GL_VERTEX_ARRAY);
			GL::glVertexPointer(fieldedMesh.points());
			glDrawBuffer(GL_BACK);
			glDepthRange(0.0, 1.0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDepthFunc(GL_LEQUAL);
			glDrawElements(GL_TRIANGLES, fieldedMesh.getIndices().size(), GL_UNSIGNED_INT, &fieldedMesh.getIndices()[0]);
			glDisableClientState(GL_VERTEX_ARRAY);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthFunc(GL_LESS);
		}

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

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	// call parent method
	else 
	{
		GlutExaminer::draw(_draw_mode);
	}
}

