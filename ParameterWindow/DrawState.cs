using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace ParameterWindow
{

    public sealed class DrawState
    {
        static DrawState()
        {
            Values = new ObservableCollection<DrawState>
            {
                Wireframe,
                SolidFlat,
                SolidSmooth,
                FrontField,
                Field
            };
        }

        // Define values here.
        public static readonly DrawState None = new DrawState {Name = null, Value = 0};
        public static readonly DrawState Wireframe = new DrawState {Name = "Wireframe", Value = 1};
        public static readonly DrawState SolidFlat = new DrawState {Name = "Solid Flat", Value = 2};
        public static readonly DrawState SolidSmooth = new DrawState {Name = "Solid Smooth", Value = 3};
        public static readonly DrawState FrontField = new DrawState {Name = "Front Field", Value = 4};
        public static readonly DrawState Field = new DrawState {Name = "Field", Value = 5};
     
        public int Value { get; private set; }
        public string Name { get; private set; }

        public static IEnumerable<DrawState> Values
        {
            get;
            private set;
        }
    }

}
