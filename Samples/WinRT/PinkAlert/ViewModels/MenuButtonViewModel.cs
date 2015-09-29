using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.Data;
using Catel.MVVM;
using Microsoft.Graphics.Canvas;
using PinkAlert.Models;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace PinkAlert.ViewModels
{
    class MenuButtonViewModel : ViewModelBase
    {
        [Model]
        public MenuButtonModel Model { get; private set; }

        [ViewModelToModel("Model")]
        public bool IsEnabled { get; set; }

        public bool HasContent { get { return Model != null; } }

        [ViewModelToModel("Model")]
        public string Text { get; set; }

        public bool IsAnimationPlaying { get; set; }

        public CanvasBitmap MenuButtonAnimationSource { get; set; }

        public ListBoxItem ButtonItem { get; set; }

        public Command ClickCommand { get; private set; }

        private static readonly Uri MenuButtonAnimationSourceUri = new Uri("ms-appx:///Assets/Art/sdbtnanm.png");

        enum State
        {
            Stable,
            Entering,
            Leaving
        }

        readonly MenuPresenterViewModel _parent;
        State _state = State.Stable;

        public MenuButtonViewModel(MenuPresenterViewModel parent, MenuButtonModel model)
        {
            _parent = parent;
            Model = model;
            ClickCommand = new Command(OnClickCommand);
            LoadResources();
        }

        private void OnClickCommand()
        {
            _parent.RequestNavigation(new Uri(Model.NavigationUri));
        }

        private async void LoadResources()
        {
            var sharedDevice = CanvasDevice.GetSharedDevice();
            MenuButtonAnimationSource = await CanvasBitmap.LoadAsync(sharedDevice, MenuButtonAnimationSourceUri);
        }

        public void OnButtonItemChanged()
        {
            if (ButtonItem != null)
                ButtonItem.Loaded += ButtonItem_Loaded;
        }

        private void ButtonItem_Loaded(object sender, RoutedEventArgs e)
        {
            _parent.ReportLoaded();
        }

        public void GoToStableState()
        {
            _state = State.Stable;
            VisualStateManager.GoToState(ButtonItem, nameof(State.Stable) +
                (HasContent ? string.Empty : "NoContent"), true);
        }

        public void GoToEnteringState()
        {
            _state = State.Entering;
            VisualStateManager.GoToState(ButtonItem, nameof(State.Entering) +
                (HasContent ? string.Empty : "NoContent"), true);
        }

        public void OnIsAnimationPlayingChanged()
        {
            if (!IsAnimationPlaying)
            {
                switch (_state)
                {
                    case State.Stable:
                        break;
                    case State.Entering:
                        GoToStableState();
                        _parent.ReportEntered();
                        break;
                    case State.Leaving:
                        _parent.ReportLeaved();
                        break;
                    default:
                        break;
                }
            }
        }

        public void GoToShowContentState()
        {
            if (HasContent)
                VisualStateManager.GoToState(ButtonItem, "ShowContent", true);
        }

        public void GoToNoContentState()
        {
            if (HasContent)
                VisualStateManager.GoToState(ButtonItem, "NoContent", true);
        }

        public void GoToLeavingState()
        {
            GoToNoContentState();

            _state = State.Leaving;
            VisualStateManager.GoToState(ButtonItem, nameof(State.Leaving) +
                (HasContent ? string.Empty : "NoContent"), true);
        }
    }
}
