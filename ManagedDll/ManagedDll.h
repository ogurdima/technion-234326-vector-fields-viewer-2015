// ManagedDll.h

#pragma once

#include <string>

using namespace System;
using namespace System::Reflection;


namespace ManagedDll {	

	public ref class DoWork
	{
	public:

		void OpenWindow(void (*changedRangeCallback)(double),
						void (*changedDrawStateCallback)(int),
						void (*changedMeshColorCallback)(float,float,float,float),
						void (*changedFieldColorCallback)(float,float,float,float),
						void (*openMeshCallback)(char*),
						void (*openFieldCallback)(char*, bool))
		{
			ParameterWindow::ManagedClass::OpenParameterWindow(	changedRangeCallback, 
																changedDrawStateCallback, 
																changedMeshColorCallback,
																changedFieldColorCallback,
																openMeshCallback,
																openFieldCallback);
		}
	};
}


__declspec(dllexport) void OpenWindow(	void (*changedRangeCallback)(double),
										void (*changedDrawStateCallback)(int),
										void (*changedMeshColorCallback)(float,float,float,float),
										void (*changedFieldColorCallback)(float,float,float,float),
										void (*openMeshCallback)(char*),
										void (*openFieldCallback)(char*, bool))
{
	ManagedDll::DoWork work;
	work.OpenWindow(changedRangeCallback, 
					changedDrawStateCallback, 
					changedMeshColorCallback,
					changedFieldColorCallback,
					openMeshCallback,
					openFieldCallback);
}






