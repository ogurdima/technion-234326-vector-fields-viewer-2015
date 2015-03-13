using System;
using System.Runtime.InteropServices;
using System.Text;

namespace ParameterWindow
{
    public static class ManagedClass
    {

        public static unsafe void OpenParameterWindow(  void* changedDrawingTimeoutCallback, 
                                                        void* changedDrawStateCallback, 
                                                        void* changedMeshColorCallback,
                                                        void* changedFieldColorCallback,
                                                        void* openMeshCallback,
                                                        void* openFieldCallback,
                                                        void* changedPathWindow)
        {
            var window = new ParameterWindow();

            window.DrawStateChanged +=
                (IntParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr) changedDrawStateCallback,
                        typeof (IntParameterCallback));
            window.OpenMesh += (MeshPathParameterCallback)
                Marshal.GetDelegateForFunctionPointer((IntPtr) openMeshCallback, typeof (MeshPathParameterCallback));
            window.OpenField += (FieldPathParameterCallback)
                Marshal.GetDelegateForFunctionPointer((IntPtr) openFieldCallback, typeof (FieldPathParameterCallback));
            window.MeshColorChanged += (ColorParameterCallback)
                Marshal.GetDelegateForFunctionPointer((IntPtr) changedMeshColorCallback, typeof (ColorParameterCallback));
            window.FieldColorChanged += (ColorParameterCallback)
                Marshal.GetDelegateForFunctionPointer((IntPtr) changedFieldColorCallback,
                    typeof (ColorParameterCallback));
            window.TimeoutChanged +=
                (IntParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr) changedDrawingTimeoutCallback,
                        typeof (IntParameterCallback));
            window.PathWindowChanged +=
                (DoubleParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr) changedPathWindow, typeof (DoubleParameterCallback));

            window.Show();
        }
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void IntParameterCallback(int i);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void DoubleParameterCallback(double d);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ColorParameterCallback(float r, float g, float b, float a);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void MeshPathParameterCallback(StringBuilder s);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void FieldPathParameterCallback(StringBuilder s, bool isConst);

}