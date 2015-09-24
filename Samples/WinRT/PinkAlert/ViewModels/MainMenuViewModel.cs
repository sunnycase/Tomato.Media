using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Catel.IoC;
using Microsoft.Graphics.Canvas;
using PinkAlert.Services;
using PinkAlert.Models;
using Windows.UI.Xaml.Controls;
using Catel.Fody;

namespace PinkAlert.ViewModels
{
    class MainMenuViewModel : ViewModelBase
    {
        public string HelpDescription { get; set; }

        private static readonly Uri AlertMeterEnteringAnimationSourceUri = new Uri("ms-appx:///Assets/Art/sdwrntmp.png");
        private static readonly Uri AlertMeterPointerAnimationSourceUri = new Uri("ms-appx:///Assets/Art/sdwrnanm.png");
        /// <summary>
        /// 警报度量表进入动画 ImageSource
        /// </summary>
        public CanvasBitmap AlertMeterEnteringAnimationSource { get; set; }
        /// <summary>
        /// 警报度量表指针动画
        /// </summary>
        public CanvasBitmap AlertMeterPointerAnimationSource { get; set; }

        public MenuConfig MenuConfig { get; set; }

        public readonly IMainMenuViewService _mainMenuService;

        public MainMenuViewModel([NotNull] IMainMenuViewService mainMenuService)
        {
            _mainMenuService = mainMenuService;
            LoadResources();
        }

        private async void LoadResources()
        {
            var sharedDevice = CanvasDevice.GetSharedDevice();
            AlertMeterEnteringAnimationSource = await CanvasBitmap.LoadAsync(sharedDevice, AlertMeterEnteringAnimationSourceUri);
            AlertMeterPointerAnimationSource = await CanvasBitmap.LoadAsync(sharedDevice, AlertMeterPointerAnimationSourceUri);

            OnNavigateToMenu(MainMenuConstants.MainMenuUri);
        }

        public void OnAlterMeterEnteringAnimationEnded()
        {
            this.GetDependencyResolver().Resolve<IMainMenuViewService>()?.PlayAlterMeterPointerAnimation();
        }

        public void OnNavigateToMenu(Uri uri)
        {
            var navigation = new MenuNavigationModel();
            App.LoadComponent(navigation, uri);

            _mainMenuService.Navigate(navigation.ViewType);
            MenuConfig = navigation.MenuConfig;
        }
    }
}
