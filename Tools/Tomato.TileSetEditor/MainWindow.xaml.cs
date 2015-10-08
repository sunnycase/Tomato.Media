using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Tomato.TileSetEditor.ViewModels;

namespace Tomato.TileSetEditor
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Catel.Windows.Window
    {
        new MainViewModel ViewModel
        {
            get { return (MainViewModel)base.ViewModel; }
        }

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Application_OpenCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = ViewModel.OpenTileSetCommand.CanExecute(e.Parameter);
        }

        private void Application_OpenCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            ViewModel.OpenTileSetCommand.Execute(e.Parameter);
        }

        private void Application_NewCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = ViewModel.NewTileSetCommand.CanExecute(e.Parameter);
        }

        private void Application_NewCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            ViewModel.NewTileSetCommand.Execute(e.Parameter);
        }
    }
}
