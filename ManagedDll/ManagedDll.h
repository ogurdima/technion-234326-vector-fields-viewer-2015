// ManagedDll.h

#pragma once

#include <string>

using namespace System;
using namespace System::Reflection;


namespace ManagedDll {	

	public ref class DoWork
	{
	public:

		void OpenWindow(void (*changedDrawingTimeout)(int),
						void (*changedDrawStateCallback)(int),
						void (*changedMeshColorCallback)(float,float,float,float),
						void (*changedFieldColorCallback)(float,float,float,float),
						void (*openMeshCallback)(char*),
						void (*openFieldCallback)(char*, bool),
						void (*changedPathWindow)(double))
		{
			ParameterWindow::ManagedClass::OpenParameterWindow(	changedDrawingTimeout, 
																changedDrawStateCallback, 
																changedMeshColorCallback,
																changedFieldColorCallback,
																openMeshCallback,
																openFieldCallback,
																changedPathWindow);
		}
	};
}


__declspec(dllexport) void OpenWindow(	void (*changedDrawingTimeout)(int),
										void (*changedDrawStateCallback)(int),
										void (*changedMeshColorCallback)(float,float,float,float),
										void (*changedFieldColorCallback)(float,float,float,float),
										void (*openMeshCallback)(char*),
										void (*openFieldCallback)(char*, bool),
										void (*changedPathWindow)(double))
{
	ManagedDll::DoWork work;
	work.OpenWindow(changedDrawingTimeout, 
					changedDrawStateCallback, 
					changedMeshColorCallback,
					changedFieldColorCallback,
					openMeshCallback,
					openFieldCallback,
					changedPathWindow);
}






