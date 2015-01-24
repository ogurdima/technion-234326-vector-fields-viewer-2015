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
        public event ManagedClass.MyDel Callback;


        public ParameterWindow()
        {
            InitializeComponent();
        }

        private int i = 0;
        private void ButtonBase_OnClick(object sender, RoutedEventArgs e)
        {
            if (Callback != null)
                Callback(i++);
        }
    }
}
