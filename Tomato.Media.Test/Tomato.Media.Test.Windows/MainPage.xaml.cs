using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.ApplicationModel;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Media;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using WinRTXamlToolkit.Controls.Extensions;

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
            "【這いよれ!ニャル子さんW】 Sister, Friend, Lover (FulI ver).flac"
        };

        DispatcherTimer timer = new DispatcherTimer() { Interval = TimeSpan.FromSeconds(1) };

        ViewModel Model
        {
            get { return Resources["ViewModel"] as ViewModel; }
        }

        public MainPage()
        {
            this.InitializeComponent();
            Loaded += MainPage_Loaded;
            timer.Tick += Timer_Tick;
        }

        private void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            var thumb = sl_Time.GetFirstDescendantOfType<Thumb>();
            thumb.DragCompleted += Thumb_DragCompleted;
            thumb.DragStarted += Thumb_DragStarted;
        }

        bool isDraging = false;

        private void Thumb_DragStarted(object sender, DragStartedEventArgs e)
        {
            isDraging = true;
        }

        private void Thumb_DragCompleted(object sender, DragCompletedEventArgs e)
        {
            var binding = sl_Time.GetBindingExpression(Slider.ValueProperty);
            if (binding == null)
            {
                Seek();
            }
            isDraging = false;
        }

        private void Timer_Tick(object sender, object e)
        {
            Model.CurrentTime = player.Position.TotalSeconds;
            sl_Time.Maximum = player.Duration.TotalSeconds;
            System.Diagnostics.Debug.WriteLine(string.Format("CurrentTime: {0}", player.Position));
        }

        async void Play(CoreDispatcher dispatcher, SystemMediaTransportControls controls)
        {
            if (player == null)
            {
                player = new AudioPlayer(dispatcher, controls);
                player.IsPlayEnabled = player.IsPauseEnabled = true;
                player.MediaTransportButtonPressed += Player_MediaTransportButtonPressed;
                player.MediaPlaybackStatusChanged += Player_MediaPlaybackStatusChanged;
                player.MediaOpened += Player_MediaOpened;
                player.MediaEnded += Player_MediaEnded;
                player.IsMediaTransportControlsEnabled = true;
            }
            for (int i = 0; i < 0; i++)
            {
                var s = await MediaSource.CreateFromFile(await Package.Current.InstalledLocation.GetFileAsync(files[0]));
                await s.InitializeFullMetadatas();
            }
            var mediaSource = await MediaSource.CreateFromFile(await Package.Current.InstalledLocation.GetFileAsync(files[0]));
            sl_Time.Maximum = mediaSource.Duration.TotalSeconds;
            //await mediaSource.InitializeFullMetadatas();
            //var lrc = mediaSource.Lyrics;
            //var lrcAna = new LyricsAnalyzer(lrc);
            //var rows = lrcAna.Rows.ToArray();
            //System.Diagnostics.Debug.WriteLine(string.Format("Duration: {0}", mediaSource.Duration));
            //var mediaSource = await MediaSource.CreateFromFile(await
            //    Windows.Storage.StorageFile.GetFileFromPathAsync(@"D:\Media\Music\Vocal\东方Project\-物凄い狂っとるフランちゃんが物凄いうた.mp3"));
            player.SetMediaSource(mediaSource);
            //player.StartPlayback();
            //timer.Start();
        }

        private void Player_MediaOpened(object sender, object e)
        {
            player.StartPlayback();
        }

        private void Player_MediaTransportButtonPressed(object sender, SystemMediaTransportControlsButton e)
        {

        }

        private void Player_MediaEnded(object sender, object e)
        {
            //player.StartPlayback(TimeSpan.FromTicks(0));
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            
            Play(Dispatcher, SystemMediaTransportControls.GetForCurrentView());
        }

        private void Player_MediaPlaybackStatusChanged(object sender, MediaPlaybackStatus e)
        {
            switch (e)
            {
                case MediaPlaybackStatus.Closed:
                    timer.Stop();
                    break;
                case MediaPlaybackStatus.Changing:
                    break;
                case MediaPlaybackStatus.Stopped:
                    Model.CurrentTime = 0;
                    timer.Stop();
                    break;
                case MediaPlaybackStatus.Playing:
                    timer.Start();
                    break;
                case MediaPlaybackStatus.Paused:
                    timer.Stop();
                    break;
                default:
                    break;
            }
            Debug.WriteLine(e.ToString());
        }


        bool seeking = false;
        void Seek()
        {
            if (!seeking)
            {
                seeking = true;
                var value = sl_Time.Value;
                sl_Time.ClearValue(Slider.ValueProperty);
                sl_Time.SetBinding(Slider.ValueProperty, new Binding
                {
                    Source = Model,
                    Path = new PropertyPath("CurrentTime")
                });
                Model.CurrentTime = value;
                player.Position = TimeSpan.FromSeconds(value);
                seeking = false;
            }
        }

        private void sl_Time_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            var binding = sl_Time.GetBindingExpression(Slider.ValueProperty);
            if (binding == null && !isDraging)
            {
                Seek();
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            if (player.CurrentStatus == MediaPlaybackStatus.Paused ||
                player.CurrentStatus == MediaPlaybackStatus.Stopped)
                player.StartPlayback();
        }

        private void PauseButton_Click(object sender, RoutedEventArgs e)
        {
            if (player.CurrentStatus == MediaPlaybackStatus.Playing)
                player.PausePlayback();
        }

        private void StopButton_Click(object sender, RoutedEventArgs e)
        {
            if (player.CurrentStatus == MediaPlaybackStatus.Playing ||
                player.CurrentStatus == MediaPlaybackStatus.Paused)
                player.StopPlayback();
        }

        private void SwitchButton_Click(object sender, RoutedEventArgs e)
        {

        }
    }
}
