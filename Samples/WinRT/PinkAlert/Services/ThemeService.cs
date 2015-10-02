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
        }


        public async void Start(Uri source)
        {
            if (mediaElement.Source != source)
            {
                mediaElement.Source = source;
                var file = await StorageFile.GetFileFromApplicationUriAsync(source);
                var stream = await file.OpenReadAsync();
                mediaElement.SetSource(stream, stream.ContentType);
            }
            mediaElement.Play();
        }
    }
}
