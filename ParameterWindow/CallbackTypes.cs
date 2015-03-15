using System.Runtime.InteropServices;
using System.Text;

namespace Parameters
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void IntParameterCallback(int i);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void VoidParameterCallback();

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void DoubleParameterCallback(double d);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ColorParameterCallback(float r, float g, float b, float a);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void MeshPathParameterCallback(StringBuilder s);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void FieldPathParameterCallback(StringBuilder s, bool isConst);

    

}