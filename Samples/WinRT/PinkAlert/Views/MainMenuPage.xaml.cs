using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Catel.IoC;
using PinkAlert.Services;

// “空白页”项模板在 http://go.microsoft.com/fwlink/?LinkId=234238 上提供

namespace PinkAlert.Views
{
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class MainMenuPage : Page
    {
        public MainMenuPage()
        {
            this.InitializeComponent();
            Loaded += MainMenuPage_Loaded;
        }

        private void MainMenuPage_Loaded(object sender, RoutedEventArgs e)
        {
            me_logo.MediaFailed += Me_logo_MediaFailed;
            me_logo.MediaOpened += Me_logo_MediaOpened;
            me_logo.MediaEnded += Me_logo_MediaEnded;

            var themeService = this.GetServiceLocator().ResolveType<IThemeService>();
            //themeService.Start();
        }

        private void Me_logo_MediaEnded(object sender, RoutedEventArgs e)
        {

        }

        private void Me_logo_MediaOpened(object sender, RoutedEventArgs e)
        {

        }

        private void Me_logo_MediaFailed(object sender, ExceptionRoutedEventArgs e)
        {

        }
    }
}
