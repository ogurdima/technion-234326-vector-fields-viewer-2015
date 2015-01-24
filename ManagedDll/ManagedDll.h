// ManagedDll.h

#pragma once

using namespace System;
using namespace System::Reflection;


namespace ManagedDll {	

	public ref class DoWork
	{
	public:

		void RegisterAndCall(void (* cb)(int))
		{
			ParameterWindow::ManagedClass::RegisterAndCall(cb);
		}
	};
}


__declspec(dllexport) void RegisterAndCall(void (* cb)(int))
{
	ManagedDll::DoWork work;	
	work.RegisterAndCall(cb);	
}






