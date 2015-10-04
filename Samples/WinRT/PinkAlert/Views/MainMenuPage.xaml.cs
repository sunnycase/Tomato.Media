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
using Windows.UI.Xaml.Media.Animation;
using PinkAlert.ViewModels;
using Tomato.Media;
using Windows.Storage;
using Tomato.Media.Gaming;
using Windows.Storage.Streams;
using System.Threading.Tasks;

// “空白页”项模板在 http://go.microsoft.com/fwlink/?LinkId=234238 上提供

namespace PinkAlert.Views
{
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class MainMenuPage : Catel.Windows.Controls.Page, IMainMenuViewService
    {
        new MainMenuViewModel ViewModel
        {
            get { return base.ViewModel as MainMenuViewModel; }
        }

        private SwapChainPanel _swapPanel = new SwapChainPanel();
        private Game _game = new Game();

        public MainMenuPage()
        {
            this.InitializeComponent();
            this.GetServiceLocator().RegisterInstance<IMainMenuViewService>(this);
            Loaded += MainMenuPage_Loaded;
            Unloaded += MainMenuPage_Unloaded;
        }

        private void MainMenuPage_Unloaded(object sender, RoutedEventArgs e)
        {
            this.GetServiceLocator().RemoveType<IMainMenuViewService>();
        }

        private void MainMenuPage_Loaded(object sender, RoutedEventArgs e)
        {
            var themeService = this.GetServiceLocator().ResolveType<IThemeService>();
            themeService.Start(new Uri("ms-appx:///Assets/Theme/drok.ogg"));
            _game.SetPresenter(_swapPanel, 800, 600);
            Test();
        }

        public sealed class ResourceResolver : ITiledMapResourceResolver
        {
            public IAsyncOperation<IRandomAccessStream> OnResolveImage(string name)
            {
                return OnResolveImageIntern(name).AsAsyncOperation();
            }

            public IAsyncOperation<IRandomAccessStream> OnResolveTileSet(string name)
            {
                return OnResolveTileSetIntern(name).AsAsyncOperation();
            }

            private async Task<IRandomAccessStream> OnResolveTileSetIntern(string name)
            {
                var file = await StorageFile.GetFileFromApplicationUriAsync(new Uri("ms-appx:///Assets/Map/" + name));
                return await file.OpenReadAsync();
            }

            private async Task<IRandomAccessStream> OnResolveImageIntern(string name)
            {
                var file = await StorageFile.GetFileFromApplicationUriAsync(new Uri("ms-appx:///Assets/Map/" + name));
                return await file.OpenReadAsync();
            }
        }

        private async void Test()
        {
            var reader = new TiledMapReader(_game, new ResourceResolver());
            var file = await StorageFile.GetFileFromApplicationUriAsync(new Uri("ms-appx:///Assets/Map/isometric_grass_and_water.json"));
            var stream = await file.OpenReadAsync();
            await reader.Parse(stream);
        }

        private void fa_AlterMeter_AnimationEnded(object sender, RoutedEventArgs e)
        {
            ViewModel?.OnAlterMeterEnteringAnimationEnded();
        }

        public void PlayAlterMeterPointerAnimation()
        {
            fa_AlterMeterPointer.Visibility = Visibility.Visible;
            fa_AlterMeterPointer.CurrentFrameIndex = 0;
            fa_AlterMeterPointer.Play();
        }

        public void NavigateContent(Type navigateType)
        {
            fr_Content.Navigate(navigateType);
        }

        public void ExitMenu()
        {
            mpc_Menu.ExitMenu();
        }

        public void NavigateScene(Type navigateType)
        {
            Frame.Navigate(navigateType);
        }
    }
}
