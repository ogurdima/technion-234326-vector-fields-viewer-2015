using System;
using System.ComponentModel;
using System.IO;
using System.Linq;
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

        public static unsafe void OpenParameterWindow(void* changedDrawStateCallback,
                                                        void* openMeshCallback,
                                                        void* changedMeshColorCallback,
                                                        void* changedFieldColorCallback,
                                                        void* changedVisualizationCallback,
                                                        void* recomputePathsCallback)
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
            Instance.MeshColorChanged +=
                (ColorParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)changedMeshColorCallback,
                        typeof(ColorParameterCallback));
            Instance.FieldColorChanged +=
                (ColorParameterCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)changedFieldColorCallback,
                        typeof(ColorParameterCallback));
            Instance.VisualizationChanged +=
                (VisualizationChangedCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)changedVisualizationCallback,
                        typeof(VisualizationChangedCallback));
            Instance.RecomputePaths +=
                (RecomputePathsCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)recomputePathsCallback, typeof(RecomputePathsCallback));

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

        #region C'tor
        public ParameterWindow()
        {
            InitializeComponent();
            IsVisualizationStopped = false;
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

        #region Events
        public event IntParameterCallback DrawStateChanged;
        public event MeshPathParameterCallback OpenMesh;
        public event ColorParameterCallback MeshColorChanged;
        public event ColorParameterCallback FieldColorChanged;
        public event VisualizationChangedCallback VisualizationChanged;
        private void OnVisualizationChanged()
        {
            if (VisualizationChanged != null)
            {
                int actualTimeout = _timeout;
                if (IsVisualizationStopped)
                {
                    actualTimeout = -1; // A long time
                }
                VisualizationChanged(actualTimeout, _visualizationStep, _pathWindow);
            }
        }
        public event RecomputePathsCallback RecomputePaths;
        #endregion

        #region Properties
        private DrawState _selectedDrawState;
        public DrawState SelectedDrawState
        {
            get { return _selectedDrawState; }
            set
            {
                SetDrawState(value, true);
            }
        }

        private void SetDrawState(DrawState value, bool riseEvent)
        {
            if (value == null)
            {
                return;
            }
            _selectedDrawState = value;
            if (riseEvent && null != DrawStateChanged)
            {
                DrawStateChanged(_selectedDrawState.Value);
            }
            OnPropertyChanged("SelectedDrawState");
        }
        public void SetDrawState(int val)
        {
            SetDrawState(DrawState.Values.FirstOrDefault(v => v.Value == val), false);
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

        private string _fieldPath;
        public string FieldPath
        {
            get { return _fieldPath; }
            set
            {
                if (_fieldPath == value)
                    return;
                _fieldPath = value;
                OnPropertyChanged("FieldPath");
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
                OnVisualizationChanged();
            }
        }

        private bool _isVisualizationStopped;
        public bool IsVisualizationStopped
        {
            get { return _isVisualizationStopped; }
            set
            {
                _isVisualizationStopped = value;
                OnPropertyChanged("IsVisualizationStopped");
                OnVisualizationChanged();
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
                OnVisualizationChanged();
            }
        }

        private double _visualizationStep = 0.002;
        public double VisualizationStep
        {
            get { return _visualizationStep; }
            set
            {
                _visualizationStep = value;
                OnPropertyChanged("VisualizationStep");
                OnVisualizationChanged();
            }
        }

        private double _currentTime;
        public double CurrentTime
        {
            get { return Math.Round(_currentTime, 5); }
            set
            {
                _currentTime = value;
                OnPropertyChanged("CurrentTime");
            }
        }
        public void SetCurrentTime(double time)
        {
            _currentTime = time;
            OnPropertyChanged("CurrentTime");
        }

        private double _simulationStep = 0.005;
        public double SimulationStep
        {
            get { return _simulationStep; }
            set
            {
                _simulationStep = value;
                OnPropertyChanged("SimulationStep");
            }
        }

        private double _maxTime = 1;
        public double MaxTime
        {
            get { return _maxTime; }
            set
            {
                _maxTime = value;
                OnPropertyChanged("MaxTime");
            }
        }

        private double _minTime;
        public double MinTime
        {
            get { return _minTime; }
            set
            {
                _minTime = value;
                OnPropertyChanged("MinTime");
            }
        } 
        #endregion

        #region Handlers
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
            FieldPath = string.Empty;
            OpenMesh(new StringBuilder(fileName));
        }

        private void LoadField(object sender, RoutedEventArgs e)
        {
            var fd = new OpenFileDialog { CheckFileExists = true, CheckPathExists = true };
            if (fd.ShowDialog() != true)
            {
                return;
            }
            FieldPath = fd.FileName;
        }

        private void LoadedWindow(object sender, RoutedEventArgs e)
        {
            SelectedDrawState = DrawState.SolidSmooth;
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
            var fileInfo = new FileInfo(@"..\Data\old\Horse.off");
            CallOpenMesh(fileInfo.FullName);
        }

        private void RecomputeClick(object sender, RoutedEventArgs e)
        {
            if (RecomputePaths == null)
                return;
            var path = FieldPath ?? string.Empty;
            RecomputePaths(new StringBuilder(path), !path.EndsWith(".txt"), SimulationStep, MinTime, MaxTime);
        }

        private void StopClick(object sender, RoutedEventArgs e)
        {
            IsVisualizationStopped = true;
        }

        private void PlayClick(object sender, RoutedEventArgs e)
        {
            IsVisualizationStopped = false;
        }

        private void WindowClosing(object sender, CancelEventArgs e)
        {
            e.Cancel = true;
        } 
        #endregion

       

        

    }
}