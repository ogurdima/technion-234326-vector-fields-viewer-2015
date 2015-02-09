using System;
using System.Runtime.InteropServices;

namespace ParameterWindow
{
    public static class ManagedClass
    {

        public static unsafe void OpenParameterWindow(void* timeoutChanged,
            void* pathLengthChanged,
            void* closedCallback)
        {
            var window = new ParameterWindow();
            window.TimeoutChanged +=
                (IntParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr) timeoutChanged, typeof (IntParameterCallback));
            window.PathLengthChanged +=
                (IntParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr) pathLengthChanged, typeof (IntParameterCallback));
            window.WindowClosed +=
                (VoidParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr) closedCallback, typeof (VoidParameterCallback));
            
            window.Show();
        }
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void IntParameterCallback(int i);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void VoidParameterCallback();

}