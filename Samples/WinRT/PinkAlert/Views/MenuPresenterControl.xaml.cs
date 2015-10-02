using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using PinkAlert.ViewModels;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace PinkAlert.Views
{
    public sealed partial class MenuPresenterControl : Catel.Windows.Controls.UserControl
    {
        new MenuPresenterViewModel ViewModel
        {
            get { return base.ViewModel as MenuPresenterViewModel; }
        }

        public MenuPresenterControl()
        {
            this.InitializeComponent();
            SizeChanged += MenuPresenterControl_SizeChanged;
            ViewModelChanged += MenuPresenterControl_ViewModelChanged;
        }

        public void ExitMenu()
        {
            ViewModel?.RequestNavigation(null);
        }

        private void MenuPresenterControl_ViewModelChanged(object sender, EventArgs e)
        {
            ViewModel?.OnSizeChanged(RenderSize);
        }

        private void MenuPresenterControl_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            ViewModel?.OnSizeChanged(e.NewSize);
        }
    }
}
