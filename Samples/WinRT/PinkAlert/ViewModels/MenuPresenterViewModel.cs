using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Windows.Foundation;

namespace PinkAlert.ViewModels
{
    class MenuPresenterViewModel : ViewModelBase
    {
        private static readonly Size ButtonSize = new Size(168.0f, 42.0f);

        public ObservableCollection<MenuButtonViewModel> Buttons { get; private set; }

        private Size _oldSize;

        public MenuPresenterViewModel()
        {
            Buttons = new ObservableCollection<MenuButtonViewModel>();
        }

        public void OnSizeChanged(Size size)
        {
            if(_oldSize != size)
            {
                Buttons.Clear();
                // 测算需要容纳的 Button 数目，向下取证
                var buttonsCount = (int)Math.Floor(size.Height / ButtonSize.Height);
                for (int i = 0; i < buttonsCount; i++)
                {
                    Buttons.Add(new MenuButtonViewModel());
                }
                _oldSize = size;
            }
        }
    }
}
