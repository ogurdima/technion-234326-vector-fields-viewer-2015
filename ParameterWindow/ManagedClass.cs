using System;
using System.Runtime.InteropServices;

namespace ParameterWindow
{
    public static class ManagedClass
    {
        
        public static unsafe void RegisterAndCall(void * cb)
        {
            var d = (MyDel)Marshal.GetDelegateForFunctionPointer((IntPtr)cb, typeof(MyDel));
            var window = new ParameterWindow();
            window.Callback += d;
            window.Show();
        }

        public delegate void MyDel(int i);
    }

}