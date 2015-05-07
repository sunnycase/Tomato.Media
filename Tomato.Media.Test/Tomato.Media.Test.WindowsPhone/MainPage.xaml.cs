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
        bool initialized = false;
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

            this.NavigationCacheMode = NavigationCacheMode.Required;
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
            var value = sl_Time.ReadLocalValue(Slider.ValueProperty);
            if (value is double)
            {
                Seek();
            }
            isDraging = false;
        }

        private void Timer_Tick(object sender, object e)
        {
            //Model.CurrentTime = player.CurrentTime.TotalSeconds;
            //System.Diagnostics.Debug.WriteLine(string.Format("CurrentTime: {0}", player.CurrentTime));
        }

        async void Play(CoreDispatcher dispatcher, SystemMediaTransportControls controls)
        {
            if (!initialized)
            {
                await BackgroundAudioTask.Initialize();
                BackgroundAudioTask.IsPlayEnabled = true;
               BackgroundAudioTask.IsPauseEnabled = true;
               BackgroundAudioTask.PauseButtonPressed += Player_OnPauseButtonPressed;
               BackgroundAudioTask.PlayButtonPressed += Player_OnPlayButtonPressed;
               BackgroundAudioTask.StopButtonPressed += Player_OnStopButtonPressed;
               BackgroundAudioTask.MediaPlaybackStatusChanged += Player_MediaPlaybackStatusChanged;
                BackgroundAudioTask.IsSystemMediaControlEnabled = true;
            }
            for (int i = 0; i < 0; i++)
            {
                var s = await MediaSource.CreateFromFile(await Package.Current.InstalledLocation.GetFileAsync(files[0]));
                await s.InitializeFullMetadatas();
            }
            var file = await Package.Current.InstalledLocation.GetFileAsync(files[0]);
            //sl_Time.Maximum = mediaSource.Duration.TotalSeconds;
            //await mediaSource.InitializeFullMetadatas();
            //var lrc = mediaSource.Lyrics;
            //var lrcAna = new LyricsAnalyzer(lrc);
            //var rows = lrcAna.Rows.ToArray();
            //System.Diagnostics.Debug.WriteLine(string.Format("Duration: {0}", mediaSource.Duration));
            //var mediaSource = await MediaSource.CreateFromFile(await
            //    Windows.Storage.StorageFile.GetFileFromPathAsync(@"D:\Media\Music\Vocal\东方Project\-物凄い狂っとるフランちゃんが物凄いうた.mp3"));
            await BackgroundAudioTask.SetMediaSource(file.Path);
            BackgroundAudioTask.StartPlayback();
            timer.Start();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            Play(Dispatcher, SystemMediaTransportControls.GetForCurrentView());
        }

        private async void Player_MediaPlaybackStatusChanged(object sender, MediaPlaybackStatus e)
        {
            if (e == MediaPlaybackStatus.Stopped)
            {
                await BackgroundAudioTask.SetMediaSource(null);
                //player.StartPlayback(TimeSpan.FromTicks(0));
            }
            //else if (e == MediaPlaybackStatus.Playing && timer.IsEnabled == false)
            //    timer.Start();
        }

        private void Player_OnStopButtonPressed(object sender, object e)
        {
            //BackgroundAudioTask.StopPlayback();
        }

        private void Player_OnPlayButtonPressed(object sender, object e)
        {
            BackgroundAudioTask.StartPlayback();
        }

        private void Player_OnPauseButtonPressed(object sender, object e)
        {
            //player.PausePlayback();
        }

        void Seek()
        {
            var value = sl_Time.Value;
            sl_Time.ClearValue(Slider.ValueProperty);
            sl_Time.SetBinding(Slider.ValueProperty, new Binding
            {
                Source = Model,
                Path = new PropertyPath("CurrentTime")
            });
            Model.CurrentTime = value;
            timer.Stop();
            //BackgroundAudioTask.StartPlayback(TimeSpan.FromSeconds(value));
        }

        private void sl_Time_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            var value = sl_Time.ReadLocalValue(Slider.ValueProperty);
            if (value is double && !isDraging)
            {
                Seek();
            }
        }

        private async void Button_Click(object sender, RoutedEventArgs e)
        {
            //BackgroundMediaPlayer.Current.CurrentStateChanged += Current_CurrentStateChanged;
            await BackgroundAudioTask.Initialize();
            BackgroundAudioTask.IsPlayEnabled = true;
            BackgroundAudioTask.IsSystemMediaControlEnabled = true;
            var file = await Package.Current.InstalledLocation.GetFileAsync(files[0]);
            await BackgroundAudioTask.SetMediaSource(file.Path);
            BackgroundAudioTask.StartPlayback();

            Debug.WriteLine("Initialized.");
        }
    }
}
