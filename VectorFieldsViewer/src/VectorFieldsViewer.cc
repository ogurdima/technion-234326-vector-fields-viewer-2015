//=============================================================================
//                                                
//   Code framework for the lecture
//
//   "Surface Representation and Geometric Modeling"
//
//   Mark Pauly, Mario Botsch, Balint Miklos, and Hao Li
//
//   Copyright (C) 2007 by  Applied Geometry Group and 
//							Computer Graphics Laboratory, ETH Zurich
//                                                                         
//-----------------------------------------------------------------------------
//                                                                            
//                                License                                     
//                                                                            
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//   
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//   
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 51 Franklin Street, Fifth Floor, 
//   Boston, MA  02110-1301, USA.
//                                                                            
//=============================================================================
//=============================================================================
//
//  CLASS VectorFieldsViewer - IMPLEMENTATION
//
//=============================================================================


#include <windows.h>
#include "VectorFieldsViewer.h"
#include <vector>
#include <float.h>

VectorFieldsViewer::VectorFieldsViewer(const char* _title, int _width, int _height) : 
GlutExaminer(_title, _width, _height),
fieldSimulationTimeInterval(0.5),
fieldSimulationMinTime(0),
fieldSimulationMaxTime(10)
{
	clear_draw_modes();
	add_draw_mode("Wireframe");
	add_draw_mode("Hidden Line");
	add_draw_mode("Solid Flat");
	add_draw_mode("Solid Smooth");
	add_draw_mode("Vector Field");
	add_draw_mode("Only Lines");
	
	LOAD_GEOMETRY_KEY = add_draw_mode("Load Geometry");

	const char initPath[] = "..\\Data\\Horse.off";
	open_mesh(initPath);
	set_draw_mode(4);
}

// Overriden virtual method - fetch class specific IDs here
void VectorFieldsViewer::processmenu(int i) 
{
	if(i != LOAD_GEOMETRY_KEY)
	{
		set_draw_mode(i);
	}
	else
	{
		OPENFILENAME ofn={0};
		char szFileName[MAX_PATH]={0};
		ofn.lStructSize=sizeof(OPENFILENAME);
		ofn.Flags=OFN_ALLOWMULTISELECT|OFN_EXPLORER;
		ofn.lpstrFilter="All Files (*.*)\0*.*\0";
		ofn.lpstrFile=szFileName;
		ofn.nMaxFile=MAX_PATH;
		if(GetOpenFileName(&ofn)) {
			open_mesh(szFileName);
		}
	}
}

VectorFieldsViewer::~VectorFieldsViewer()
{
}

bool VectorFieldsViewer::open_mesh(const char* fileName)
{
	if (fieldedMesh.load(fileName))
	{
		computeVectorFieldLines();

		set_scene( (Vec3f)(fieldedMesh.boundingBoxMin() + fieldedMesh.boundingBoxMax())*0.5,
			0.5*(fieldedMesh.boundingBoxMin() - fieldedMesh.boundingBoxMax()).norm());
		glutPostRedisplay();
		return true;
	}

	// info
	std::cerr << fieldedMesh.n_vertices() << " vertices, " << fieldedMesh.n_faces()    << " faces\n";
	return false;
}

void VectorFieldsViewer::computeVectorFieldLines()
{
	bool success = pathFinder.configure(fieldedMesh, fieldSimulationTimeInterval, fieldSimulationMinTime, fieldSimulationMaxTime);
	if (success) {
		particlePaths = pathFinder.getParticlePaths();
	}
	else {
		std::cerr << "Failed to properly configure PathFinder" << std::endl;
		particlePaths = vector<vector<Vec3f>>();
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

		vector<unsigned int> vIndexes;
		unsigned int stamIndexes[2] = {0,1};
		glClearColor(0.0,0.0,0.0,0.0);
		glColor4f(1.0, 1.0, 1.0, 0.5);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for (int i = 0; i < particlePaths.size(); i++) 
		{
			vIndexes.clear();
			for (int j = 0; j < particlePaths[i].size(); j++) {
				vIndexes.push_back(j);
			}
			glEnableClientState(GL_VERTEX_ARRAY);
			GL::glVertexPointer(&(particlePaths[i][0]));
			glDrawElements(GL_LINE_STRIP, particlePaths[i].size(), GL_UNSIGNED_INT, &vIndexes[0]);
			glDisableClientState(GL_VERTEX_ARRAY);
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	// call parent method
	else 
	{
		GlutExaminer::draw(_draw_mode);
	}
}

