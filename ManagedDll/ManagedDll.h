// ManagedDll.h

#pragma once

using namespace System;
using namespace System::Reflection;


namespace ManagedDll {	

	public ref class DoWork
	{
	public:

		void OpenWindow(void (*timeoutChanged)(int),
						void (*pathLengthChanged)(int),
						void (*closedCallback)(void))
		{
			ParameterWindow::ManagedClass::OpenParameterWindow(timeoutChanged, pathLengthChanged, closedCallback);
		}
	};
}


__declspec(dllexport) void OpenWindow(void (*timeoutChanged)(int),
				void (*pathLengthChanged)(int),
				void (*closedCallback)(void))
{
	ManagedDll::DoWork work;	
	work.OpenWindow(timeoutChanged, pathLengthChanged, closedCallback);	
}






