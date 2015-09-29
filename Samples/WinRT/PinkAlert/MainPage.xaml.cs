using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Tomato.Media.Gaming;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

//“空白页”项模板在 http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409 上有介绍

namespace PinkAlert
{
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private Game game = new Game();

        public MainPage()
        {
            this.InitializeComponent();

            scp_Game.SizeChanged += Scp_Game_SizeChanged;
        }

        private void Scp_Game_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            game.SetPresenter(scp_Game, (uint)scp_Game.ActualWidth, (uint)scp_Game.ActualHeight);
        }
    }
}
