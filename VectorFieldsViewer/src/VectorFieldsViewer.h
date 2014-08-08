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
//  CLASS VectorFieldsViewer
//
//=============================================================================


#ifndef VECTORFIELDSVIEWERWIDGET_H
#define VECTORFIELDSVIEWERWIDGET_H

#include "MeshViewer.h"
#include "PathFinder.h"

class VectorFieldsViewer : public MeshViewer
{
public:
	/// default constructor
	VectorFieldsViewer(const char* _title, int _width, int _height);
	// destructor
	~VectorFieldsViewer();
	/// open mesh
	virtual bool open_mesh(const char* _filename);
	virtual void processmenu(int i);
	static const int LOAD_GEOMETRY = 6001;

protected:
	virtual void draw(const std::string& _draw_mode);
	/// calculate vertex valences
	void calc_valences();
	/// set vertex color from vertex valence
	void color_coding();
	OpenMesh::VPropHandleT<int> valence_vprop;
	int maxValence;
	int minValence;

	OpenMesh::FPropHandleT<Vec3f> vfield_fprop;
	void assign_vector_field();

	void compute_vf_lines();

	
	std::vector<std::vector<Vec3f>> particlePaths;



private:
};

#endif

