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

#define MAX_VALENCE 10000

VectorFieldsViewer::VectorFieldsViewer(const char* _title, int _width, int _height) : MeshViewer(_title, _width, _height), 
	maxValence(1),
	minValence(0)
{ 
	mesh_.request_vertex_colors();
	add_draw_mode("Vector Field");
	add_draw_mode("Only Lines");
	// Add custom menu entries here
	glutAddMenuEntry("Load Geometry", LOAD_GEOMETRY);
	// Adding custom property: 
	mesh_.add_property(valence_vprop);
	mesh_.add_property(vfield_fprop);
}

// Overriden virtual method - fetch class specific IDs here
void VectorFieldsViewer::processmenu(int i) 
{
	switch (i) 
	{
	case LOAD_GEOMETRY: 
		{
			OPENFILENAME ofn={0};
			char szFileName[MAX_PATH]={0};
			ofn.lStructSize=sizeof(OPENFILENAME);
			ofn.Flags=OFN_ALLOWMULTISELECT|OFN_EXPLORER;
			ofn.lpstrFilter="All Files (*.*)\0*.*\0";
			ofn.lpstrFile=szFileName;
			ofn.nMaxFile=MAX_PATH;
			if(GetOpenFileName(&ofn)) {
				bool ok = true;
				open_mesh(szFileName);
			}
		}
		break;
	default: // default - let superclass handle this
		{
			MeshViewer::processmenu(i);
		}
	}
}

VectorFieldsViewer::~VectorFieldsViewer()
{
}

bool VectorFieldsViewer::open_mesh(const char* _filename)
{
	// load mesh
	if (MeshViewer::open_mesh(_filename))
	{
		assign_vector_field();
		compute_vf_lines();
		glutPostRedisplay();
		return true;
	}
	return false;
}

// Add random normalized Vec3f as vfield_fprop property for each face
// Color all certices appropriately
void VectorFieldsViewer::assign_vector_field()
{
	for(Mesh::FaceIter fit = mesh_.faces_begin(); fit != mesh_.faces_end(); ++fit) {
		double x = rand();
		double y = rand();
		double z = rand();
		Vec3f field = Vec3f(x,y,z).normalized();
		mesh_.property(vfield_fprop, fit.handle()) = field;

		for(Mesh::FaceVertexIter fvit = mesh_.fv_begin(fit.handle()); fvit != mesh_.fv_end(fit.handle()); ++fvit) {
			unsigned char r = floor((255 * field[0]));
			unsigned char g = floor((255 * field[1]));
			unsigned char b = floor((255 * field[2]));
			mesh_.set_color(fvit, Mesh::Color(r,g,b));
		}
	}
}

void VectorFieldsViewer::compute_vf_lines()
{
	PathFinder pathFinder = PathFinder(mesh_, vfield_fprop);
	particlePaths = pathFinder.getParticlePaths();
	bool ok = true;
}

void VectorFieldsViewer::draw(const std::string& _draw_mode)
{
	if (indices_.empty())
	{
		MeshViewer::draw(_draw_mode);
		return;
	}
	if (_draw_mode == "Vector Field")
	{
		//glDisable(GL_LIGHTING);
		//glShadeModel(GL_SMOOTH);
		//glEnableClientState(GL_VERTEX_ARRAY);
		//glEnableClientState(GL_NORMAL_ARRAY);
		//glEnableClientState(GL_COLOR_ARRAY);
		//GL::glVertexPointer(mesh_.points());
		//GL::glNormalPointer(mesh_.vertex_normals());
		//GL::glColorPointer(mesh_.vertex_colors());
		//glDepthRange(0.01, 1.0);
		//glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, &indices_[0]);
		//glDisableClientState(GL_VERTEX_ARRAY);
		//glDisableClientState(GL_NORMAL_ARRAY);
		//glDisableClientState(GL_COLOR_ARRAY);

		glColor3f(0.1, 0.1, 0.1);
		glEnableClientState(GL_VERTEX_ARRAY);
		GL::glVertexPointer(mesh_.points());
		glDrawBuffer(GL_BACK);
		glDepthRange(0.0, 1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDepthFunc(GL_LEQUAL);
		glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, &indices_[0]);
		glDisableClientState(GL_VERTEX_ARRAY);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDepthFunc(GL_LESS);

		unsigned int stamIndexes[2] = {0,1};
		glDisable(GL_LIGHTING);
		glColor3f(1.0, 1.0, 1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		for (int i = 0; i < particlePaths.size(); i++) 
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			GL::glVertexPointer(&(particlePaths[i][0]));
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, stamIndexes);
			glDisableClientState(GL_VERTEX_ARRAY);
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	}
	if (_draw_mode == "Only Lines")
	{
		unsigned int stamIndexes[2] = {0,1};
		glDisable(GL_LIGHTING);
		glColor3f(1.0, 1.0, 1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		for (int i = 0; i < particlePaths.size(); i++) 
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			GL::glVertexPointer(&(particlePaths[i][0]));
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, stamIndexes);
			glDisableClientState(GL_VERTEX_ARRAY);
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	// call parent method
	else 
	{
		MeshViewer::draw(_draw_mode);
	}
}


// Legacy

void VectorFieldsViewer::calc_valences()
{
	// Dont want to do it, so let's leave it linear as it is now
	int valenceBucket[MAX_VALENCE] = {};
	// Iterate over all vertices one-by-one
	for(Mesh::VertexIter vit = mesh_.vertices_begin(); vit != mesh_.vertices_end(); ++vit) {
		int valence = 0;
		// iterate over 1-ring neighbor vertices, basically just count them
		for(Mesh::VertexVertexIter vvit = mesh_.vv_iter(vit); vvit; ++vvit) {
			valence++;
		}
		// store this value as a custom property of the vertex
		mesh_.property(valence_vprop, vit) = valence;
		// storing maxValence and minValence for O(1) access for future color coding
		if ( valence > maxValence ) {
			maxValence = valence;
		}
		if ( minValence == 0 || minValence > valence ) {
			minValence = valence;
		}
	}
}

void VectorFieldsViewer::color_coding()
{
	int dValence = maxValence - minValence;
	for(Mesh::VertexIter vit = mesh_.vertices_begin(); vit != mesh_.vertices_end(); ++vit) {
		int curValence = mesh_.property(valence_vprop, vit);
		int r = (255 * (curValence - minValence)) / dValence;
		int g = (255 - r);
		// color change is linear on vetex valence: max is red, min is green.
		// Looks a bit weird on allmost-regular models with a singular high-valence vertex.
		// Can be fixed using lerp until median valence value but I am too lazy to do it.
		mesh_.set_color(vit, Mesh::Color(r, g, 0));
	}
}
