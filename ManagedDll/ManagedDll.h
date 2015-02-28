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
						void (*openMeshCallback)(std::string),
						void (*openFieldCallback)(std::string, bool))
		{
			//ParameterWindow::ManagedClass::OpenParameterWindow(timeoutChanged, pathLengthChanged, closedCallback);
		}
	};
}


__declspec(dllexport) void OpenWindow(void (*changedRangeCallback)(double), 
									  void (*openMeshCallback)(std::string),
									  void (*openFieldCallback)(std::string, bool))

{
	ManagedDll::DoWork work;	
	work.OpenWindow(changedRangeCallback, openMeshCallback, openFieldCallback);
}






