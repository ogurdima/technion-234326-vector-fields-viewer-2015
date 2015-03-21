using System;
using System.ComponentModel;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Media;
using Microsoft.Win32;

namespace Parameters
{
    /// <summary>
    /// Interaction logic for ParameterWindow.xaml
    /// </summary>
    public sealed partial class ParameterWindow : Window, INotifyPropertyChanged
    {
        #region Static
        private static bool _opened;
        private static readonly ParameterWindow _parameterWindow;
        static ParameterWindow()
        {
            _parameterWindow = new ParameterWindow();
        }

        public static unsafe void OpenParameterWindow(void* changedDrawingTimeoutCallback,
                                                        void* changedDrawStateCallback,
                                                        void* changedMeshColorCallback,
                                                        void* changedFieldColorCallback,
                                                        void* openMeshCallback,
                                                        void* openFieldCallback,
                                                        void* changedPathWindowCallback,
                                                        void* changedSimulationStepCallback,
                                                        void* changedVisualizationStepCallback,
                                                        void* recomputeCallback)
        {
            if (_opened)
            {
                return;
            }
            Instance.DrawStateChanged +=
                (IntParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)changedDrawStateCallback,
                        typeof(IntParameterCallback));
            Instance.OpenMesh +=
                (MeshPathParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)openMeshCallback, typeof(MeshPathParameterCallback));
            Instance.OpenField +=
                (FieldPathParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)openFieldCallback,
                        typeof(FieldPathParameterCallback));
            Instance.MeshColorChanged +=
                (ColorParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)changedMeshColorCallback,
                        typeof(ColorParameterCallback));
            Instance.FieldColorChanged +=
                (ColorParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)changedFieldColorCallback,
                        typeof(ColorParameterCallback));
            Instance.TimeoutChanged +=
                (IntParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)changedDrawingTimeoutCallback,
                        typeof(IntParameterCallback));
            Instance.PathWindowChanged +=
                (DoubleParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)changedPathWindowCallback,
                        typeof(DoubleParameterCallback));
            Instance.SimulationStepChanged +=
                (DoubleParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)changedSimulationStepCallback,
                        typeof(DoubleParameterCallback));
            Instance.VisualizationStepChanged +=
                (DoubleParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)changedVisualizationStepCallback,
                        typeof(DoubleParameterCallback));
            Instance.RecomputePaths +=
                (VoidParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)recomputeCallback, typeof(VoidParameterCallback));

            Instance.Show();
            _opened = true;
        }

        public static ParameterWindow Instance
        {
            get
            {
                return _parameterWindow;
            }
        }
        #endregion

        #region INotifyPropertyChanged
        public event PropertyChangedEventHandler PropertyChanged;

        private void OnPropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler != null)
                handler(this, new PropertyChangedEventArgs(propertyName));
        }
        #endregion

        public ParameterWindow()
        {
            InitializeComponent();
            PropertyChanged += PropertyChangedCallback;
        }

        private void PropertyChangedCallback(object sender, PropertyChangedEventArgs propertyChangedEventArgs)
        {
            NeedRecompute = _oldSimulationStep != _simulationStep;
        }

        public event IntParameterCallback TimeoutChanged;
        public event IntParameterCallback DrawStateChanged;
        public event DoubleParameterCallback PathWindowChanged;
        public event MeshPathParameterCallback OpenMesh;
        public event FieldPathParameterCallback OpenField;
        public event ColorParameterCallback MeshColorChanged;
        public event ColorParameterCallback FieldColorChanged;
        public event DoubleParameterCallback SimulationStepChanged;
        public event DoubleParameterCallback VisualizationStepChanged;
        public event VoidParameterCallback RecomputePaths;

        private DrawState _selectedDrawState;
        public DrawState SelectedDrawState
        {
            get { return _selectedDrawState; }
            set
            {
                if (value == null)
                {
                    return;
                }
                _selectedDrawState = value;
                if (DrawStateChanged != null)
                {
                    DrawStateChanged(_selectedDrawState.Value);
                }
                OnPropertyChanged("SelectedDrawState");
            }
        }

        private Color _meshColor;
        public Color MeshColor
        {
            get { return _meshColor; }
            set
            {
                _meshColor = value;
                if (MeshColorChanged != null)
                {
                    MeshColorChanged(_meshColor.ScR, _meshColor.ScG, _meshColor.ScB, _meshColor.ScA);
                }
                OnPropertyChanged("MeshColor");
            }
        }

        private Color _fieldColor;
        public Color FieldColor
        {
            get { return _fieldColor; }
            set
            {
                _fieldColor = value;
                if (FieldColorChanged != null)
                {
                    FieldColorChanged(_fieldColor.ScR, _fieldColor.ScG, _fieldColor.ScB, _fieldColor.ScA);
                }
                OnPropertyChanged("FieldColor");
            }
        }

        private int _timeout;
        public int Timeout
        {
            get { return _timeout; }
            set
            {
                _timeout = value;
                OnPropertyChanged("Timeout");
                if (TimeoutChanged != null)
                {
                    TimeoutChanged(_timeout);
                }
            }
        }

        private double _pathWindow;
        public double PathWindow
        {
            get { return _pathWindow; }
            set
            {
                _pathWindow = value;
                OnPropertyChanged("PathWindow");
                if (PathWindowChanged != null)
                {
                    PathWindowChanged(_pathWindow);
                }
            }
        }
        public void UpdatePathWindow(double value)
        {
            _pathWindow = value;
            OnPropertyChanged("PathWindow");
        }

        private double _visualizationStep;
        public double VisualizationStep
        {
            get { return _visualizationStep; }
            set
            {
                _visualizationStep = value;
                OnPropertyChanged("VisualizationStep");
                if (VisualizationStepChanged != null)
                {
                    VisualizationStepChanged(_visualizationStep);
                }
            }
        }
        public void UpdateVisualizationStep(double step)
        {
            _visualizationStep = step;
            OnPropertyChanged("VisualizationStep");
        }

        private double _oldSimulationStep;
        private double _simulationStep;
        public double SimulationStep
        {
            get { return _simulationStep; }
            set
            {
                _simulationStep = value;
                OnPropertyChanged("SimulationStep");
                if (SimulationStepChanged != null)
                {
                    SimulationStepChanged(_simulationStep);
                }
            }
        }
        public void UpdateSimulationStep(double step)
        {
            _oldSimulationStep = _simulationStep = step;
            OnPropertyChanged("SimulationStep");
        }

        private bool _needRecompute;
        public bool NeedRecompute
        {
            get { return _needRecompute; }
            set
            {
                if (_needRecompute == value)
                    return;
                _needRecompute = value;
                OnPropertyChanged("NeedRecompute");
            }
        }

        private void LoadMesh(object sender, RoutedEventArgs e)
        {
            var fd = new OpenFileDialog { CheckFileExists = true, CheckPathExists = true };
            if (fd.ShowDialog() != true)
            {
                return;
            }
            CallOpenMesh(fd.FileName);
        }

        private void CallOpenMesh(string fileName)
        {
            if (OpenMesh == null)
                return;
            MeshPath.Text = fileName;
            FieldPath.Text = string.Empty;
            OpenMesh(new StringBuilder(fileName));
        }

        private void LoadField(object sender, RoutedEventArgs e)
        {
            var fd = new OpenFileDialog { CheckFileExists = true, CheckPathExists = true };
            if (fd.ShowDialog() != true)
            {
                return;
            }
            if (OpenField == null)
                return;
            FieldPath.Text = fd.FileName;
            OpenField(new StringBuilder(fd.FileName), !fd.FileName.EndsWith(".txt"));
        }

        private void LoadedWindow(object sender, RoutedEventArgs e)
        {
            SelectedDrawState = DrawState.Field;
            MeshColor = new Color
            {
                ScR = 0.1f,
                ScG = 0.1f,
                ScB = 0.1f,
                ScA = 1f
            };
            FieldColor = new Color
            {
                ScR = 0f,
                ScG = 1f,
                ScB = 0f,
                ScA = 1f
            };
            Timeout = 60;
            PathWindow = 0.1;
            var fileInfo = new FileInfo(@"..\Data\miri\teddy171.off");
            CallOpenMesh(fileInfo.FullName);
        }

        private void RecomputeClick(object sender, RoutedEventArgs e)
        {
            if (RecomputePaths != null)
            {
                RecomputePaths();
            }
        }

        private void WindowClosing(object sender, CancelEventArgs e)
        {
            e.Cancel = true;
        }

    }
}