using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Foundation.Collections;
using Windows.Media;

namespace Tomato.Media.Test.BackgroundAudioHandler
{
    public sealed class BackgroundAudioHandler : IBackgroundAudioHandler
    {
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

        private IAudioPlayer player;
        private int fileIdx = 0;
        private int nextIdx = 0;

        public BackgroundAudioHandler()
        {
        }

        public async void OnActivated(IAudioPlayer player)
        {
            this.player = player;
            player.IsMediaTransportControlsEnabled = true;
            player.IsPlayEnabled = player.IsPauseEnabled = player.IsStopEnabled =
                player.IsPreviousEnabled = player.IsNextEnabled = true;

            var file = await Package.Current.InstalledLocation.GetFileAsync(files[fileIdx]);
            player.SetMediaSource(await MediaSource.CreateFromFile(file));
        }

        public void OnError(Exception exception)
        {

        }

        public void OnMediaEnded()
        {
            if (nextIdx != -1)
            {
                if (fileIdx != nextIdx)
                    player.StopPlayback();
                else
                {
                    if (player.Position != TimeSpan.Zero)
                        player.Position = TimeSpan.Zero;
                    else
                        player.StartPlayback();
                }
            }
        }

        async void Switch()
        {
            if (nextIdx != -1)
            {
                if (fileIdx != nextIdx)
                {
                    fileIdx = nextIdx;
                    var file = await Package.Current.InstalledLocation.GetFileAsync(files[nextIdx]);
                    player.SetMediaSource(await MediaSource.CreateFromFile(file));
                }
                else
                    player.StartPlayback();
            }
        }

        public void OnMediaOpened()
        {
            //player.Position = TimeSpan.FromSeconds(280);
            player.StartPlayback();
        }

        public void OnMediaPlaybackStatusChanged(MediaPlaybackStatus newStatus)
        {
            switch (newStatus)
            {
                case MediaPlaybackStatus.Closed:
                    break;
                case MediaPlaybackStatus.Changing:
                    break;
                case MediaPlaybackStatus.Stopped:
                    Switch();
                    break;
                case MediaPlaybackStatus.Playing:
                    break;
                case MediaPlaybackStatus.Paused:
                    break;
                default:
                    break;
            }
            Debug.WriteLine(newStatus.ToString());
        }

        public void OnMediaTransportButtonPressed(SystemMediaTransportControlsButton button)
        {
            switch (button)
            {
                case SystemMediaTransportControlsButton.Play:
                    player.StartPlayback();
                    break;
                case SystemMediaTransportControlsButton.Pause:
                    player.PausePlayback();
                    break;
                case SystemMediaTransportControlsButton.Stop:
                    player.StopPlayback();
                    break;
                case SystemMediaTransportControlsButton.Record:
                    break;
                case SystemMediaTransportControlsButton.FastForward:
                    break;
                case SystemMediaTransportControlsButton.Rewind:
                    break;
                case SystemMediaTransportControlsButton.Next:
                    nextIdx = fileIdx == 0 ? 5 : 0;
                    if (player.CurrentStatus == MediaPlaybackStatus.Stopped)
                        Switch();
                    else
                        player.StopPlayback();
                    break;
                case SystemMediaTransportControlsButton.Previous:
                    nextIdx = fileIdx == 0 ? 5 : 0;
                    if (player.CurrentStatus == MediaPlaybackStatus.Stopped)
                        Switch();
                    else
                        player.StopPlayback();
                    break;
                case SystemMediaTransportControlsButton.ChannelUp:
                    break;
                case SystemMediaTransportControlsButton.ChannelDown:
                    break;
                default:
                    break;
            }
        }

        public void OnReceivedMessageFromForeground(object sender, ValueSet valueSet)
        {
            if (valueSet.ContainsKey("Stop"))
            {
                nextIdx = -1;
                player.StopPlayback();
            }
            else if (valueSet.ContainsKey("Play"))
                player.StartPlayback();
            else if (valueSet.ContainsKey("Pause"))
                player.PausePlayback();
            else if (valueSet.ContainsKey("Switch"))
            {
                nextIdx = fileIdx == 0 ? 5 : 0;
                if (player.CurrentStatus == MediaPlaybackStatus.Stopped)
                    Switch();
                else
                    player.StopPlayback();
            }
            else if (valueSet.ContainsKey("Position"))
                BackgroundAudioPlayerClient.SendMessageToForeground(new ValueSet
                {
                    { "Posision", player.Position }
                });
            else if (valueSet.ContainsKey("Seek"))
            {
                if(player.CurrentStatus != MediaPlaybackStatus.Changing &&
                    player.CurrentStatus != MediaPlaybackStatus.Closed)
                {
                    if (!isSeeking)
                    {
                        Debug.WriteLine("Call Seek {0}", (TimeSpan)valueSet["Seek"]);
                        isSeeking = true;
                        player.Position = (TimeSpan)valueSet["Seek"];
                    }
                }
            }
        }

        bool isSeeking = false;

        public void OnSeekCompleted()
        {
            if (player.CurrentStatus != MediaPlaybackStatus.Playing)
                player.StartPlayback();
            isSeeking = false;
        }
    }
}
