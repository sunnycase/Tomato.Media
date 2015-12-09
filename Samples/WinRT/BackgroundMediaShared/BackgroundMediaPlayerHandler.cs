using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Media;
using Windows.Storage;

namespace BackgroundMediaShared
{
    public sealed class BackgroundMediaPlayerHandler : IBackgroundMediaPlayerHandler
    {
        private BackgroundMediaPlayer mediaPlayer;

        public async void OnActivated(BackgroundMediaPlayer mediaPlayer)
        {
            this.mediaPlayer = mediaPlayer;
            mediaPlayer.MediaOpened += MediaPlayer_MediaOpened;

            var file = await StorageFile.GetFileFromApplicationUriAsync(new Uri("ms-appx:///Assets/04 - irony -TV Mix-.mp3"));
            var stream = await file.OpenReadAsync();
            var mediaSource = await MediaSource.CreateFromStream(stream);
            Debug.WriteLine($"Title: {mediaSource.Title}");
            Debug.WriteLine($"Album: {mediaSource.Album}");

            mediaPlayer.SetMediaSource(mediaSource);
        }

        public void OnReceiveMessage(string message)
        {
            if (message == "Play")
            {
                mediaPlayer.Play();
                mediaPlayer.SendMessage("Playing");
            }
            Debug.WriteLine($"Client Message: {message}");
        }

        private void MediaPlayer_MediaOpened(IMediaPlayer sender, object args)
        {
            mediaPlayer.SendMessage("Ready to Play");
        }
    }
}
