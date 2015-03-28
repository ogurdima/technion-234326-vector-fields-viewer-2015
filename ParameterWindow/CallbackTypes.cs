using System.Runtime.InteropServices;
using System.Text;

namespace Parameters
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void IntParameterCallback(int i);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void RecomputePathsCallback(StringBuilder s, bool isConst, double step, double min, double max);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void VisualizationChangedCallback(int timeout, double step, double window);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ColorParameterCallback(float r, float g, float b, float a);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void MeshPathParameterCallback(StringBuilder s);

}