using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Microsoft.Graphics.Canvas;
using Windows.Foundation;
using Windows.UI.Xaml;

namespace PinkAlert.ViewModels
{
    class MenuPresenterViewModel : ViewModelBase
    {
        private static readonly Size ButtonSize = new Size(168.0f, 42.0f);

        public ObservableCollection<MenuButtonViewModel> Buttons { get; private set; }


        private static readonly Uri MenuButtonAnimationSourceUri = new Uri("ms-appx:///Assets/Art/sdbtnanm.png");
        public CanvasBitmap MenuButtonAnimationSource { get; set; }

        private Size _oldSize;

        public MenuPresenterViewModel()
        {
            Buttons = new ObservableCollection<MenuButtonViewModel>();
            buttonsDelayTimer.Tick += ButtonsDelayTimer_Tick;
            LoadResources();
        }

        private async void LoadResources()
        {
            var sharedDevice = CanvasDevice.GetSharedDevice();
            MenuButtonAnimationSource = await CanvasBitmap.LoadAsync(sharedDevice, MenuButtonAnimationSourceUri);
        }

        public void OnSizeChanged(Size size)
        {
            if (_oldSize != size)
            {
                Buttons.Clear();
                // 测算需要容纳的 Button 数目，向下取证
                var buttonsCount = (int)Math.Floor(size.Height / ButtonSize.Height);
                for (int i = 0; i < buttonsCount; i++)
                {
                    Buttons.Add(new MenuButtonViewModel(this)
                    {
                        HasContent = i % 2 == 0
                    });
                }
                _oldSize = size;
            }
        }

        private DispatcherTimer buttonsDelayTimer = new DispatcherTimer()
        {
            Interval = TimeSpan.FromMilliseconds(20)
        };
        private int _loaded = 0;
        private IEnumerator buttonsEnteringEnum;
        public void ReportLoaded()
        {
            if (++_loaded == Buttons.Count)
            {
                buttonsEnteringEnum = EnteringButtons().GetEnumerator();
                buttonsDelayTimer.Start();
            }
        }

        private void ButtonsDelayTimer_Tick(object sender, object e)
        {
            buttonsEnteringEnum.MoveNext();
        }

        private IEnumerable EnteringButtons()
        {
            foreach (var button in Buttons)
            {
                button.GoToEnteringState();
                yield return null;
            }
            buttonsDelayTimer.Stop();
        }

        private int _entered = 0;
        public void ReportEntered()
        {
            if (++_entered == Buttons.Count)
            {
                ShowContent();
            }
        }

        private void ShowContent()
        {
            foreach (var button in Buttons)
                button.GoToShowContentState();
        }
    }
}
