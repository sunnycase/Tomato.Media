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

        public MainMenuPage()
        {
            this.InitializeComponent();
            this.GetServiceLocator().RegisterInstance<IMainMenuViewService>(this);
            Loaded += MainMenuPage_Loaded;
            Unloaded += MainMenuPage_Unloaded;
            LoadFile();
        }

        XAudioSession xaudio;
        XAudioSound sound;
        XAudioChannel channel;
        private async void LoadFile()
        {
            var stream = await (await StorageFile.GetFileFromApplicationUriAsync(new Uri("ms-appx:///Assets/Audio/Sound/Yamaha-TG100-Whistle-C5.wav")))
                .OpenStreamForReadAsync();
            using (var reader = new BinaryReader(stream))
            {
                var data = new byte[stream.Length];
                reader.Read(data, 0, data.Length);
                xaudio = new XAudioSession();
                sound = new XAudioSound(data);
                channel = xaudio.AddChannel(sound.Format);
            }
            channel.Play(sound);
            channel.Play(sound);
        }

        private void MainMenuPage_Unloaded(object sender, RoutedEventArgs e)
        {
            this.GetServiceLocator().RemoveType<IMainMenuViewService>();
        }

        private void MainMenuPage_Loaded(object sender, RoutedEventArgs e)
        {
            var themeService = this.GetServiceLocator().ResolveType<IThemeService>();
            themeService.Start();
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

        public void Navigate(Type navigateType)
        {
            fr_Content.Navigate(navigateType);
        }
    }
}
