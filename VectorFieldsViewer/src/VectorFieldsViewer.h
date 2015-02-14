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

#include "VectorFieldsUtils.h"
#include "MeshViewer.h"
#include "PathFinder.h"
#include "FieldedMesh.h"
#include "ParticlePath.h"




class VectorFieldsViewer : public GlutExaminer
{

public:

	/// default constructor
	VectorFieldsViewer(const char* _title, int _width, int _height);
	/// open mesh
	virtual bool open_mesh(const char* _filename);
	virtual void processmenu(int i);
	int LOAD_GEOMETRY_KEY;
	int LOAD_FIELD_KEY;
	
	

protected:
	static VectorFieldsViewer* activeInstance;
	static void onTimer(int val);

	virtual void draw(const std::string& _draw_mode);

	void drawWireframe(Vec3f color = Vec3f(1,1,1)); 
	void drawSolid(bool isSmooth, bool useLighting, Vec3f color = Vec3f(0,0,0)); 
	void drawVectorField(); 

private:
	bool isParameterOpen;
	Vec3f color;

	static void timeoutChanged(int newVal)
	{
		activeInstance->timeout = newVal;

	}

	static void maxPathChanged(int newVal)
	{
		activeInstance->maxActivePathLength = newVal;
		activeInstance->resetColorAndIndices();
	}

	void resetColorAndIndices()
	{
		colors.resize(maxActivePathLength * 4);
		indices.resize(maxActivePathLength);
		for(uint i = 0; i < maxActivePathLength; ++i)
		{
			indices[i] = i;
			colors[i * 4] = color[0];
			colors[i * 4 + 1] = color[1];
			colors[i * 4 + 2] = color[2];
			colors[i * 4 + 3] = std::sqrt(std::sqrt( ((float) i) / maxActivePathLength)) / 3;
		}
	}

	static void windowClosed()
	{
		activeInstance->isParameterOpen = false;
	}

	void OpenParameterWindow();

	void evolvePaths();
	void resetTimer();

	vector<ParticlePath> particlePaths;

	FieldedMesh		fieldedMesh;
	PathFinder		pathFinder;

	double			fieldSimulationTimeInterval;
	double			fieldSimulationMinTime;
	double			fieldSimulationMaxTime;

	void computeVectorFieldLines();

	vector<uint> indices;
	vector<float> colors;

	uint maxActivePathLength;
	uint timeout;
};

#endif

