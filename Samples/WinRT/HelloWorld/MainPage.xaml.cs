using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using BackgroundMediaShared;
using Tomato.Media;
using Tomato.Media.Codec;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;

//“空白页”项模板在 http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409 上有介绍

namespace HelloWorld
{
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private BackgroundMediaPlayerClient playerClient;
        private VideoPresenter videoPresenter;
        private EffectMediaStreamSource streamSource;

        private static readonly string[] fileNames = new[]
        {
            "ms-appx:///Assets/04.花篝り.APE",
            "ms-appx:///Assets/04 - irony -TV Mix-.mp3",
            "ms-appx:///Assets/08. きらきらエブリディ.mp3"
        };
        private string fileName = fileNames[1];

        public MainPage()
        {
            this.InitializeComponent();

            Loaded += MainPage_Loaded;
        }

        private async void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            playerClient = new BackgroundMediaPlayerClient(typeof(BackgroundMediaPlayerHandler));
            playerClient.MessageReceived += PlayerClient_MessageReceived;

            //foreach (var fileName in fileNames)
            //{
            //    var file = await StorageFile.GetFileFromApplicationUriAsync(new Uri(fileNames[2]));
            //    var stream = await file.OpenReadAsync();
            //    var metadataProvider = await MediaMetadataProvider.CreateFromStream(stream, false);
            //    var pics = metadataProvider.Pictures;
            //}
            var file = await StorageFile.GetFileFromApplicationUriAsync(new Uri(fileNames[0]));
            var stream = await file.OpenReadAsync();
            var metadataProvider = await MediaMetadataProvider.CreateFromStream(stream, false);
            var pics = metadataProvider.Pictures;
            if(pics.Any())
            {
                var image = new BitmapImage();
                await image.SetSourceAsync((new MemoryStream(pics[0].Data)).AsRandomAccessStream());
                img_Video.Source = image;
            }
            var str = metadataProvider.Lyrics.ToString();

            //var mediaSource = await MediaSource.CreateFromStream(stream, file.Path);
            //streamSource = new EffectMediaStreamSource(mediaSource);
            //Media.SetMediaStreamSource(streamSource.Source);
            //GC.Collect();
            //GC.WaitForPendingFinalizers();
            //GC.Collect();
            //Debug.WriteLine($"Before: {GC.GetTotalMemory(true)} Bytes.");
            //for (int i = 0; i < 200; i++)
            //{
            //    foreach (var fileName in fileNames)
            //    {
            //        var file = await StorageFile.GetFileFromApplicationUriAsync(new Uri(fileName));
            //        var stream = await file.OpenReadAsync();
            //        var mediaSource = await MediaSource.CreateFromStream(stream, fileName);
            //        Debug.WriteLine($"Title: {mediaSource.Title}");
            //        Debug.WriteLine($"Album: {mediaSource.Album}");
            //        Debug.WriteLine($"Artist: {mediaSource.Artist}");
            //        Debug.WriteLine($"AlbumArtist: {mediaSource.AlbumArtist}");
            //        Debug.WriteLine($"Duration: {mediaSource.Duration}");
            //        Debug.WriteLine($"Lyrics: {mediaSource.Lyrics}");
            //    }
            //}
            //GC.Collect();
            //GC.WaitForPendingFinalizers();
            //GC.Collect();
            //Debug.WriteLine($"After: {GC.GetTotalMemory(true)} Bytes.");

            //var meta = await MediaMetadataProvider.CreateFromStream(stream, false);
            //Debug.WriteLine($"Title: {meta.Title}");
            //Debug.WriteLine($"Album: {meta.Album}");
            //Debug.WriteLine($"Artist: {meta.Artist}");
            //Debug.WriteLine($"AlbumArtist: {meta.AlbumArtist}");
            //Debug.WriteLine($"Duration: {meta.Duration}");
            //Debug.WriteLine($"Lyrics: {meta.Lyrics}");
            //var file = await StorageFile.GetFileFromApplicationUriAsync(new Uri(fileName));
            //var stream = await file.OpenReadAsync();
            //var mediaSource = await MediaSource.CreateFromStream(stream, fileName);
            //videoPresenter = new VideoPresenter();
            //videoPresenter.SetMediaSource(mediaSource);
            //img_Video.Source = videoPresenter.ImageSource;

            //videoPresenter.Play();
        }

        private void PlayerClient_MessageReceived(object sender, MessageReceivedEventArgs e)
        {
            if (e.Message == "Ready to Play")
                playerClient.SendMessage("M", "Play");
            Debug.WriteLine($"Player Message: {e.Tag}, {e.Message}");
        }

        private void MediaElement_MediaFailed(object sender, ExceptionRoutedEventArgs e)
        {

        }

        private void MediaElement_MediaOpened(object sender, RoutedEventArgs e)
        {

        }
    }
}
