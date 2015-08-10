using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using BackgroundMediaShared;
using Tomato.Media;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
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

        public MainPage()
        {
            this.InitializeComponent();

            Loaded += MainPage_Loaded;
        }

        private async void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            //playerClient = new BackgroundMediaPlayerClient(typeof(BackgroundMediaPlayerHandler).FullName);


            var file = await StorageFile.GetFileFromApplicationUriAsync(new Uri("ms-appx:///Assets/Ore no Imouto ga Konna ni Kawaii Wake ga Nai Opening.avi"));
            var stream = await file.OpenReadAsync();
            var mediaSource = await MediaSource.CreateFromStream(stream);
            videoPresenter = new VideoPresenter();
            videoPresenter.SetMediaSource(mediaSource);
            img_Video.Source = videoPresenter.ImageSource;

            videoPresenter.Play();
        }
    }
}
