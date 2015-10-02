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
    class MainMenuViewModel : ViewModelBase, IMainMenuService
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
            this.GetServiceLocator().RegisterInstance<IMainMenuService>(this);
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

        private void OnNavigateToMenu(Uri uri)
        {
            if (uri == null)
            {
                OnNavigateScene();
            }
            else
            {
                var navigation = new MenuNavigationModel();
                App.LoadComponent(navigation, uri);

                _mainMenuService.NavigateContent(navigation.ViewType);
                MenuConfig = navigation.MenuConfig;
            }
        }

        public void NavigateToMenu(Uri uri)
        {
            OnNavigateToMenu(uri);
        }

        protected override Task OnClosingAsync()
        {
            this.GetServiceLocator().RemoveType<IMainMenuService>();
            return base.OnClosingAsync();
        }

        private Uri _requestedSceneNavigation;
        public void NavigateScene(Uri uri)
        {
            _requestedSceneNavigation = uri;
            _mainMenuService.ExitMenu();
        }

        private void OnNavigateScene()
        {
            var navigation = new SceneNavigationModel();
            App.LoadComponent(navigation, _requestedSceneNavigation);
            _mainMenuService.NavigateScene(navigation.ViewType);
        }
    }
}
