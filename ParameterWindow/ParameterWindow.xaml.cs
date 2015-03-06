using System;
using System.ComponentModel;
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
            _selectedDrawState = DrawState.Field;
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

        public event IntParameterCallback DrawStateChanged;
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

        private Color _meshColor = new Color
        {
            ScR = 0.1f,
            ScG = 0.1f,
            ScB = 0.1f,
            ScA = 1f
        };
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
            }
        }

        private Color _fieldColor = new Color
        {
            ScR = 0f,
            ScG = 1f,
            ScB = 0f,
            ScA = 1f
        };
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
            }
        }

        private void LoadMesh(object sender, RoutedEventArgs e)
        {
            var fd = new OpenFileDialog {CheckFileExists = true, CheckPathExists = true};
            if (fd.ShowDialog() != true)
            {
                return;
            }
            if (OpenMesh == null)
                return;
            MeshPath.Text = fd.FileName;
            OpenMesh(new StringBuilder(fd.FileName));
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

        
    }
}