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
using Windows.Media.Playback;
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

        BackgroundAudioPlayerClient audioClient;
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
            Model.CurrentTime = audioClient.Position.TotalSeconds;
            sl_Time.Maximum = audioClient.Duration.TotalSeconds;
            System.Diagnostics.Debug.WriteLine(string.Format("CurrentTime: {0}", audioClient.Position));
        }

        async void Play()
        {
            if (!initialized)
            {
                initialized = true;
                var type = typeof(BackgroundAudioHandler.BackgroundAudioHandler);
                audioClient = new BackgroundAudioPlayerClient(type.FullName);
                audioClient.MessageReceivedFromBackground += AudioClient_MessageReceivedFromBackground;
            }
            for (int i = 0; i < 0; i++)
            {
                var s = await MediaSource.CreateFromFile(await Package.Current.InstalledLocation.GetFileAsync(files[0]));
                await s.InitializeFullMetadatas();
            }
            var file = await Package.Current.InstalledLocation.GetFileAsync(files[0]);
            var mediaSource = await MediaSource.CreateFromFile(file);
            await mediaSource.InitializeFullMetadatas();
            //var lrc = mediaSource.Lyrics;
            //var lrcAna = new LyricsAnalyzer(lrc);
            timer.Start();
        }

        private void AudioClient_MessageReceivedFromBackground(object sender, ValueSet e)
        {
        }

        private void Controls_ButtonPressed(SystemMediaTransportControls sender, SystemMediaTransportControlsButtonPressedEventArgs args)
        {
            throw new NotImplementedException();
        }

        private void Controls_PropertyChanged(SystemMediaTransportControls sender, SystemMediaTransportControlsPropertyChangedEventArgs args)
        {

        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            Play();
        }

        private async void Player_MediaPlaybackStatusChanged(object sender, MediaPlaybackStatus e)
        {

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
                audioClient.SendMessageToBackground(new ValueSet
                {
                    { "Seek", TimeSpan.FromSeconds(value) }
                });
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

        private async void Button_Click(object sender, RoutedEventArgs e)
        {
            audioClient.SendMessageToBackground(new ValueSet
            {
                { "Play", null }
            });
        }

        private void StopButton_Click(object sender, RoutedEventArgs e)
        {
            audioClient.SendMessageToBackground(new ValueSet
            {
                { "Stop", null }
            });
        }

        private void PauseButton_Click(object sender, RoutedEventArgs e)
        {
            audioClient.SendMessageToBackground(new ValueSet
            {
                { "Pause", null }
            });
        }

        private void SwitchButton_Click(object sender, RoutedEventArgs e)
        {
            audioClient.SendMessageToBackground(new ValueSet
            {
                { "Switch", null }
            });
        }
    }
}
