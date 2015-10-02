using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.Fody;
using Catel.MVVM;
using Microsoft.Graphics.Canvas;
using PinkAlert.Models;
using Windows.Foundation;
using Windows.UI.Xaml;
using Catel.IoC;
using PinkAlert.Services;

namespace PinkAlert.ViewModels
{
    class MenuPresenterViewModel : ViewModelBase
    {
        private static readonly Size ButtonSize = new Size(168.0f, 42.0f);

        public ObservableCollection<MenuButtonViewModel> Buttons { get; private set; }


        private static readonly Uri MenuButtonAnimationSourceUri = new Uri("ms-appx:///Assets/Art/sdbtnanm.png");
        public CanvasBitmap MenuButtonAnimationSource { get; set; }

        private int _oldButtonsCount;
        private readonly MenuConfig _config;
        private readonly ISoundService _soundService;

        public MenuPresenterViewModel([NotNull] MenuConfig config, [NotNull]ISoundService soundService)
        {
            _config = config;
            _soundService = soundService;
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
            // 测算需要容纳的 Button 数目，向下取证
            var buttonsCount = (int)Math.Floor(size.Height / ButtonSize.Height);
            if (_oldButtonsCount != buttonsCount)
            {
                Buttons.Clear();
                // 顶部按钮
                foreach (var button in _config.TopButtons)
                    Buttons.Add(new MenuButtonViewModel(this, button));
                // padding 按钮
                var paddingCount = Math.Max(0, buttonsCount - _config.TopButtons.Count - _config.BottomButtons.Count);
                for (int i = 0; i < paddingCount; i++)
                    Buttons.Add(new MenuButtonViewModel(this, null));
                // 底部按钮
                foreach (var button in _config.BottomButtons)
                    Buttons.Add(new MenuButtonViewModel(this, button));

                _loaded = _entered = _leaved = 0;
                _oldButtonsCount = buttonsCount;
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
            _soundService.PlaySound(SoundConstants.SlideIn);
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
            _soundService.PlaySound(SoundConstants.SlideOut);
            foreach (var button in Buttons)
            {
                button.GoToLeavingState();
                yield return null;
            }
            buttonsLeavingDelayTimer.Stop();
        }

        private int _leaved = 0;
        public void ReportLeaved()
        {
            if (++_leaved == Buttons.Count)
                DoNavigation();
        }

        private Uri _requestedNavigation;
        public void RequestNavigation(Uri uri)
        {
            _requestedNavigation = uri;
            buttonsLeavingEnum = LeavingButtons().GetEnumerator();
            buttonsLeavingDelayTimer.Start();
        }

        private void DoNavigation()
        {
            var service = this.GetDependencyResolver().Resolve<IMainMenuService>();
            service.NavigateToMenu(_requestedNavigation);
        }
    }
}
