using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using PinkAlert.Services;
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
using PinkAlert.Areas.Game;

// “空白页”项模板在 http://go.microsoft.com/fwlink/?LinkId=234238 上提供

namespace PinkAlert.Areas.Campaign.Views
{
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class LoadCampaignPage : Catel.Windows.Controls.Page, ILoadCampaignViewService
    {
        public LoadCampaignPage()
        {
            this.InitializeComponent();
            Loaded += LoadCampaignPage_Loaded;
            Unloaded += LoadCampaignPage_Unloaded;
        }

        private void LoadCampaignPage_Unloaded(object sender, RoutedEventArgs e)
        {
            this.GetServiceLocator().RemoveType<ILoadCampaignViewService>();
        }

        public void NaviagteToGame()
        {
            Frame.Navigate(typeof(BattleControlPage));
        }

        private void LoadCampaignPage_Loaded(object sender, RoutedEventArgs e)
        {
            this.GetServiceLocator().RegisterInstance<ILoadCampaignViewService>(this);

            var themeService = this.GetServiceLocator().ResolveType<IThemeService>();
            themeService.Start(new Uri("ms-appx:///Assets/Theme/bully.ogg"));
        }
    }
}
