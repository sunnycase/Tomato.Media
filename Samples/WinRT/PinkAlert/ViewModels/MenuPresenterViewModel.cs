using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Microsoft.Graphics.Canvas;
using PinkAlert.Models;
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
            buttonsEnteringDelayTimer.Tick += ButtonsEnteringDelayTimer_Tick;
            buttonsLeavingDelayTimer.Tick += ButtonsLeavingDelayTimer_Tick;
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
                    Buttons.Add(new MenuButtonViewModel(this, new MenuButtonModel
                    {
                        IsEnabled = true,
                        Text = "主选单"
                    }));
                }
                _oldSize = size;
            }
        }

        private DispatcherTimer buttonsEnteringDelayTimer = new DispatcherTimer()
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
                buttonsEnteringDelayTimer.Start();
            }
        }

        private void ButtonsEnteringDelayTimer_Tick(object sender, object e)
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
            buttonsEnteringDelayTimer.Stop();
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

        public void ReportNavigate(Type navigationType)
        {
            buttonsLeavingEnum = LeavingButtons().GetEnumerator();
            buttonsLeavingDelayTimer.Start();
        }


        private DispatcherTimer buttonsLeavingDelayTimer = new DispatcherTimer()
        {
            Interval = TimeSpan.FromMilliseconds(20)
        };

        private IEnumerator buttonsLeavingEnum;
        private void ButtonsLeavingDelayTimer_Tick(object sender, object e)
        {
            buttonsLeavingEnum.MoveNext();
        }

        private IEnumerable LeavingButtons()
        {
            foreach (var button in Buttons)
            {
                button.GoToLeavingState();
                yield return null;
            }
            buttonsLeavingDelayTimer.Stop();
        }
    }
}
