using System;
using System.ComponentModel;
using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Media;
using Microsoft.Win32;

namespace ParameterWindow
{
    /// <summary>
    /// Interaction logic for ParameterWindow.xaml
    /// </summary>
    public sealed partial class ParameterWindow : Window, INotifyPropertyChanged
    {
        public ParameterWindow()
        {
            InitializeComponent();
        }

        #region INotifyPropertyChanged
        public event PropertyChangedEventHandler PropertyChanged;

        private void OnPropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler != null)
                handler(this, new PropertyChangedEventArgs(propertyName));
        } 
        #endregion

        public event IntParameterCallback TimeoutChanged;
        public event IntParameterCallback DrawStateChanged;
        public event DoubleParameterCallback PathWindowChanged;
        public event MeshPathParameterCallback OpenMesh;
        public event FieldPathParameterCallback OpenField;
        public event ColorParameterCallback MeshColorChanged;
        public event ColorParameterCallback FieldColorChanged;

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
        private double _pathWindow;

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

        private void LoadMesh(object sender, RoutedEventArgs e)
        {
            var fd = new OpenFileDialog {CheckFileExists = true, CheckPathExists = true};
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
            OpenMesh(new StringBuilder(fileName));
        }

        private void LoadField(object sender, RoutedEventArgs e)
        {
            var fd = new OpenFileDialog {CheckFileExists = true, CheckPathExists = true};
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

            var fileInfo = new FileInfo(@"..\Data\old\Horse.off");
            CallOpenMesh(fileInfo.FullName);
        }

        
    }
}