using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.ApplicationModel;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Media;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// “空白页”项模板在 http://go.microsoft.com/fwlink/?LinkId=234238 上提供

namespace Tomato.Media.Test
{
    /// <summary>
    /// 可独立使用或用于导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class MainPage : Page
    {
        AudioPlayer player;
        string[] files =
        {
            "04.花篝り.APE",
            "03.ずるいよ….mp3",
            "04. 全天候型いらっしゃいませ.flac",
            "02. 涙色の翼.flac",
            "09.つないだ手.APE"
        };

        public MainPage()
        {
            this.InitializeComponent();
        }

        async void Play()
        {
            await player.Initialize();
            player.IsSystemMediaControlEnabled = true;
            var mediaSource = await MediaSource.CreateFromFile(await Package.Current.InstalledLocation.GetFileAsync(files[0]));
            player.SetMediaSource(mediaSource);
            player.StartPlayback();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            player = new AudioPlayer();
            player.OnPauseButtonPressed += Player_OnPauseButtonPressed;
            player.OnPlayButtonPressed += Player_OnPlayButtonPressed;
            player.OnStopButtonPressed += Player_OnStopButtonPressed;

            Play();
        }

        private void Player_OnStopButtonPressed(object sender, object e)
        {
            player.StopPlayback();
        }

        private void Player_OnPlayButtonPressed(object sender, object e)
        {
            player.StartPlayback();
        }

        private void Player_OnPauseButtonPressed(object sender, object e)
        {
            player.PausePlayback();
        }
    }
}
