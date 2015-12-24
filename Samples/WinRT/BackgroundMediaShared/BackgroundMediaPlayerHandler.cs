using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Media;
using Windows.Media;
using Windows.Storage;

namespace BackgroundMediaShared
{
    public sealed class BackgroundMediaPlayerHandler : IBackgroundMediaPlayerHandler
    {
        private BackgroundMediaPlayer mediaPlayer;
        private SystemMediaTransportControls smtc;

        public async void OnActivated(BackgroundMediaPlayer mediaPlayer)
        {
            this.mediaPlayer = mediaPlayer;
            smtc = mediaPlayer.SystemMediaTransportControls;
            ConfigureSystemMediaTransportControls();
            mediaPlayer.MediaOpened += MediaPlayer_MediaOpened;
            mediaPlayer.MediaEnded += MediaPlayer_MediaEnded;
            mediaPlayer.CurrentStateChanged += MediaPlayer_CurrentStateChanged;

            var file = await StorageFile.GetFileFromApplicationUriAsync(new Uri("ms-appx:///Assets/04 - irony -TV Mix-.mp3"));
            var stream = await file.OpenReadAsync();
            var mediaSource = await MediaSource.CreateFromStream(stream);
            Debug.WriteLine($"Title: {mediaSource.Title}");
            Debug.WriteLine($"Album: {mediaSource.Album}");
            Debug.WriteLine($"Artist: {mediaSource.Artist}");
            Debug.WriteLine($"AlbumArtist: {mediaSource.AlbumArtist}");
            Debug.WriteLine($"Duration: {mediaSource.Duration}");
            Debug.WriteLine($"Lyrics: {mediaSource.Lyrics}");

            smtc.DisplayUpdater.MusicProperties.Title = mediaSource.Title;
            smtc.DisplayUpdater.Update();

            mediaPlayer.SetMediaSource(mediaSource);
        }

        private void MediaPlayer_MediaEnded(IMediaPlayer sender, object args)
        {
            Debug.WriteLine("Media ended.");
            mediaPlayer.Play();
        }

        private void MediaPlayer_CurrentStateChanged(IMediaPlayer sender, object args)
        {
            switch (mediaPlayer.State)
            {
                case Windows.Media.Playback.MediaPlayerState.Closed:
                    smtc.PlaybackStatus = MediaPlaybackStatus.Closed;
                    break;
                case Windows.Media.Playback.MediaPlayerState.Opening:
                    smtc.PlaybackStatus = MediaPlaybackStatus.Changing;
                    break;
                case Windows.Media.Playback.MediaPlayerState.Buffering:
                    smtc.PlaybackStatus = MediaPlaybackStatus.Changing;
                    break;
                case Windows.Media.Playback.MediaPlayerState.Playing:
                    smtc.PlaybackStatus = MediaPlaybackStatus.Playing;
                    break;
                case Windows.Media.Playback.MediaPlayerState.Paused:
                    smtc.PlaybackStatus = MediaPlaybackStatus.Paused;
                    break;
                case Windows.Media.Playback.MediaPlayerState.Stopped:
                    smtc.PlaybackStatus = MediaPlaybackStatus.Stopped;
                    break;
                default:
                    break;
            }
        }

        private void ConfigureSystemMediaTransportControls()
        {
            smtc.ButtonPressed += Smtc_ButtonPressed;
            smtc.IsPlayEnabled = smtc.IsPauseEnabled = true;
            smtc.IsEnabled = true;
            smtc.PlaybackStatus = MediaPlaybackStatus.Closed;
            smtc.DisplayUpdater.Type = MediaPlaybackType.Music;
            smtc.DisplayUpdater.Update();
        }

        private void Smtc_ButtonPressed(SystemMediaTransportControls sender, SystemMediaTransportControlsButtonPressedEventArgs args)
        {
            switch (args.Button)
            {
                case SystemMediaTransportControlsButton.Play:
                    mediaPlayer.Play();
                    break;
                case SystemMediaTransportControlsButton.Pause:
                    mediaPlayer.Pause();
                    break;
                case SystemMediaTransportControlsButton.Stop:
                    mediaPlayer.Pause();
                    break;
                case SystemMediaTransportControlsButton.Record:
                    break;
                case SystemMediaTransportControlsButton.FastForward:
                    break;
                case SystemMediaTransportControlsButton.Rewind:
                    break;
                case SystemMediaTransportControlsButton.Next:
                    break;
                case SystemMediaTransportControlsButton.Previous:
                    break;
                case SystemMediaTransportControlsButton.ChannelUp:
                    break;
                case SystemMediaTransportControlsButton.ChannelDown:
                    break;
                default:
                    break;
            }
        }

        public void OnReceiveMessage(string tag, string message)
        {
            if (message == "Play")
            {
                mediaPlayer.Play();
                mediaPlayer.SendMessage("M", "Playing");
            }
            Debug.WriteLine($"Client Message: {tag}, {message}");
        }

        private void MediaPlayer_MediaOpened(IMediaPlayer sender, object args)
        {
            mediaPlayer.SendMessage("M", "Ready to Play");
        }
    }
}
