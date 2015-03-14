// ManagedDll.h

#pragma once

#include <string>

using namespace System;
using namespace System::Reflection;

__declspec(dllexport) void OpenWindow(	void (*changedDrawingTimeout)(int),
										void (*changedDrawStateCallback)(int),
										void (*changedMeshColorCallback)(float,float,float,float),
										void (*changedFieldColorCallback)(float,float,float,float),
										void (*openMeshCallback)(char*),
										void (*openFieldCallback)(char*, bool),
										void (*changedPathWindowCallback)(double),
										void (*changedSimulationStepCallback)(double),
										void (*changedVisualizationStepCallback)(double),
										void (*recomputePathsCallback)(void))
{
	ParameterWindow::ParameterWindow::OpenParameterWindow(changedDrawingTimeout, 
					changedDrawStateCallback, 
					changedMeshColorCallback,
					changedFieldColorCallback,
					openMeshCallback,
					openFieldCallback,
					changedPathWindowCallback,
					changedSimulationStepCallback,
					changedVisualizationStepCallback,
					recomputePathsCallback);
}


__declspec(dllexport) void UpdatePathWindow(double pathWindow)
{
	ParameterWindow::ParameterWindow::Instance->UpdatePathWindow(pathWindow);
}

__declspec(dllexport) void UpdateSimulationStep(double simulationStep)
{
	ParameterWindow::ParameterWindow::Instance->UpdateSimulationStep(simulationStep);
}

__declspec(dllexport) void UpdateVisualizationStep(double visualizationStep)
{
	ParameterWindow::ParameterWindow::Instance->UpdateVisualizationStep(visualizationStep);
}



