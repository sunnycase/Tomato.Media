using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace PinkAlert.Services
{
    class ThemeService : IThemeService
    {
        private MediaElement mediaElement = new MediaElement();

        public ThemeService()
        {
            mediaElement.AudioCategory = AudioCategory.GameMedia;
            mediaElement.AudioDeviceType = AudioDeviceType.Multimedia;
            mediaElement.IsLooping = true;
            mediaElement.AutoPlay = true;
            mediaElement.MediaFailed += MediaElement_MediaFailed;
        }

        private void MediaElement_MediaFailed(object sender, Windows.UI.Xaml.ExceptionRoutedEventArgs e)
        {

        }

        bool mediaSet = false;
        public async void Start()
        {
            if (!mediaSet)
            {
                var file = await StorageFile.GetFileFromApplicationUriAsync(new Uri("ms-appx:///Assets/Theme/drok.ogg"));
                var stream = await file.OpenReadAsync();
                mediaElement.SetSource(stream, stream.ContentType);
                mediaSet = true;
            }
            else
                mediaElement.Play();
        }
    }
}
