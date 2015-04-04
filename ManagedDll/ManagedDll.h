// ManagedDll.h

#pragma once

#include <string>

using namespace System;
using namespace System::Reflection;

__declspec(dllexport) void OpenWindow(void (*changedDrawStateCallback)(int),
									 void (*openMeshCallback)(char*),
									 void (*changedMeshColorCallback)(float,float,float,float),
									 void (*changedFieldColorCallback)(float,float,float,float),
									 void (*changedVisualizationCallback)(int, double, double),
									 void (*recomputePathsCallback)(char* path, bool isConst, double step, double min, double max))
{
	Parameters::ParameterWindow::OpenParameterWindow( changedDrawStateCallback, 
					openMeshCallback,
					changedMeshColorCallback,
					changedFieldColorCallback,
					changedVisualizationCallback,
					recomputePathsCallback);
}


_declspec(dllexport) void UpdateDrawStateGui(int drawState)
{
	Parameters::ParameterWindow::Instance->SetDrawState(drawState);
}

_declspec(dllexport) void UpdateCurrentTimeGui(double currentTime)
{
	Parameters::ParameterWindow::Instance->SetCurrentTime(currentTime);
}