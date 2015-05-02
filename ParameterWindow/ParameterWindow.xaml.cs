using System;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Media;
using Microsoft.Win32;
using WPFFolderBrowser;

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
                                                        void* recomputePathsCallback,
                                                        void* takeScreenshotsCallback)
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
                (ColorParameterCallbackArr)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)changedFieldColorCallback,
                        typeof(ColorParameterCallbackArr));
            Instance.VisualizationChanged +=
                (VisualizationChangedCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)changedVisualizationCallback,
                        typeof(VisualizationChangedCallback));
            Instance.RecomputePaths +=
                (RecomputePathsCallback)
                    Marshal.GetDelegateForFunctionPointer((IntPtr)recomputePathsCallback, typeof(RecomputePathsCallback));
            Instance.TakeScreenshots +=
                 (TakeScreenshotCallback)
                     Marshal.GetDelegateForFunctionPointer((IntPtr)takeScreenshotsCallback,
                            typeof(TakeScreenshotCallback));


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
            NormalizeField = false;
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
        public event ColorParameterCallbackArr FieldColorChanged;
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
        public event TakeScreenshotCallback TakeScreenshots;
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

        private bool _normalizeField;
        public bool NormalizeField
        {
            get { return _normalizeField; }
            set
            {
                _normalizeField = value;
                OnPropertyChanged("NormalizeField");
            }
        }

        private Color _fieldHeadColor;
        public Color FieldHeadColor
        {
            get { return _fieldHeadColor; }
            set
            {
                _fieldHeadColor = value;
                if (FieldColorChanged != null)
                {
                    FieldColorChanged(ColorToFloatArray(_fieldHeadColor), ColorToFloatArray(_fieldTailColor));
                }
                OnPropertyChanged("FieldHeadColor");
            }
        }

        private Color _fieldTailColor;
        public Color FieldTailColor
        {
            get { return _fieldTailColor; }
            set
            {
                _fieldTailColor = value;
                if (FieldColorChanged != null)
                {
                    FieldColorChanged(ColorToFloatArray(_fieldHeadColor), ColorToFloatArray(_fieldTailColor));
                }
                OnPropertyChanged("FieldTailColor");
            }
        }

        private static float[] ColorToFloatArray(Color c)
        {
            var f = new float[4];
            f[0] = c.ScR;
            f[1] = c.ScG;
            f[2] = c.ScB;
            f[3] = c.ScA;
            return f;
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

        private double _simulationStep = 0.05;
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

        private int _numberOfScreenshots = 1;
        public int NumberOfScreenshots
        {
            get { return _numberOfScreenshots; }
            set
            {
                _numberOfScreenshots = value;
                OnPropertyChanged("NumberOfScreenshots");
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
            Timeout = 60;
            PathWindow = 0.1;
            var fileInfo = new FileInfo(@"..\Data\old\Horse.off");
            if (!fileInfo.Exists)
            {
                fileInfo = new FileInfo(@"Horse.off");
            }
            if (fileInfo.Exists)
            {
                CallOpenMesh(fileInfo.FullName);
            }
            MeshColor = new Color
            {
                ScR = 0.1f,
                ScG = 0.1f,
                ScB = 0.1f,
                ScA = 1f
            };
            FieldHeadColor = new Color
            {
                ScR = 0f,
                ScG = 1f,
                ScB = 0f,
                ScA = 1f
            };
            FieldTailColor = new Color
            {
                ScR = 0f,
                ScG = 1f,
                ScB = 0f,
                ScA = 0f
            };
        }

        private void RecomputeClick(object sender, RoutedEventArgs e)
        {
            if (RecomputePaths == null)
                return;
            var path = FieldPath ?? string.Empty;
            RecomputePaths(new StringBuilder(path), !path.EndsWith(".txt"), NormalizeField, SimulationStep, MinTime, MaxTime);
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

        private void CaptureScreenshotsClick(object sender, RoutedEventArgs e)
        {
            if (TakeScreenshots == null)
            {
                return;
            }
            var fDialog = new WPFFolderBrowserDialog("Select folder");
            if (fDialog.ShowDialog() != true)
            {
                return;
            }
            TakeScreenshots(new StringBuilder(fDialog.FileName), NumberOfScreenshots);
        }
        #endregion
    }
}