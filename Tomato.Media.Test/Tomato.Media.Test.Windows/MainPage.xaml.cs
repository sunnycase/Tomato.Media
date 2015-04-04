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
            "09.つないだ手.APE",
            "- 雨降花 .mp3",
            "【這いよれ!ニャル子さんW】 Sister, Friend, Lover (FulI ver).flac",
            "凉宫春日 - 可爱清脆响铃.mp3"
        };

        DispatcherTimer timer = new DispatcherTimer() { Interval = TimeSpan.FromSeconds(1) };

        public MainPage()
        {
            this.InitializeComponent();
            timer.Tick += Timer_Tick;
        }

        private void Timer_Tick(object sender, object e)
        {
            System.Diagnostics.Debug.WriteLine(string.Format("CurrentTime: {0}", player.CurrentTime));
        }

        async void Play()
        {
            if (player == null)
            {
                player = new AudioPlayer();
                await player.Initialize();
                player.PauseButtonPressed += Player_OnPauseButtonPressed;
                player.PlayButtonPressed += Player_OnPlayButtonPressed;
                player.StopButtonPressed += Player_OnStopButtonPressed;
                player.MediaPlaybackStatusChanged += Player_MediaPlaybackStatusChanged;
                player.IsSystemMediaControlEnabled = true;
            }
            var mediaSource = await MediaSource.CreateFromFile(await Package.Current.InstalledLocation.GetFileAsync(files[7]));
            System.Diagnostics.Debug.WriteLine(string.Format("Duration: {0}", mediaSource.Duration));
            //var mediaSource = await MediaSource.CreateFromFile(await
            //    Windows.Storage.StorageFile.GetFileFromPathAsync(@"D:\Media\Music\Vocal\东方Project\-物凄い狂っとるフランちゃんが物凄いうた.mp3"));
            player.SetMediaSource(mediaSource);
            player.StartPlayback();
            timer.Start();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            Play();
        }

        private void Player_MediaPlaybackStatusChanged(object sender, MediaPlaybackStatus e)
        {
            if (e == MediaPlaybackStatus.Stopped)
                player.StartPlayback(TimeSpan.FromTicks(0));
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
