using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace ParameterWindow
{
    /// <summary>
    /// Interaction logic for ParameterWindow.xaml
    /// </summary>
    public partial class ParameterWindow : Window
    {


        public ParameterWindow()
        {
            InitializeComponent();
            Closed += OnClosed;
        }

        private void OnClosed(object sender, EventArgs eventArgs)
        {
            WindowClosed();
        }


        public event IntParameterCallback TimeoutChanged = delegate { };

        private int _timeout = 100;

        public int Timeout
        {
            get { return _timeout; }
            set
            {
                _timeout = value;
                TimeoutChanged(_timeout);
            }
        }

        public event IntParameterCallback PathLengthChanged = delegate { }; 

        private int _maxLength = 10;

        public int MaxLength
        {
            get { return _maxLength; }
            set
            {
                _maxLength = value;
                PathLengthChanged(_maxLength);
            }
        }


        public event VoidParameterCallback WindowClosed = delegate { };
    }
}
