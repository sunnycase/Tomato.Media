using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Tomato.Media;
using Tomato.Media.Toolkit;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Media.Playback;
using Windows.Storage.Pickers;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

//“空白页”项模板在 http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409 上有介绍

namespace BackgroundMediaPlayback
{
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private static readonly MediaEnvironment _mediaEnvironment = new MediaEnvironment();

        static MainPage()
        {
            _mediaEnvironment.RegisterDefaultCodecs();
        }

        public MainPage()
        {
            this.InitializeComponent();
        }

        private async void Button_Click(object sender, RoutedEventArgs e)
        {
            var dlg = new FileOpenPicker();
            dlg.FileTypeFilter.Add(".ape");
            dlg.FileTypeFilter.Add(".flac");
            dlg.FileTypeFilter.Add(".ogg");
            dlg.FileTypeFilter.Add(".m4a");
            dlg.FileTypeFilter.Add(".aac");
            dlg.FileTypeFilter.Add(".alac");
            dlg.FileTypeFilter.Add(".dff");
            dlg.FileTypeFilter.Add(".dsf");
            dlg.FileTypeFilter.Add(".tak");
            var file = await dlg.PickSingleFileAsync();
            if (file != null)
            {
                me.SetSource(await file.OpenReadAsync(), file.ContentType);
                var metadataProvider = await MediaMetadataProvider.CreateFromStream(await file.OpenReadAsync(), file.Path, false);
                metadataProvider.Album.ToString();
            }
        }
    }
}
