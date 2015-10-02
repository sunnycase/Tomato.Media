using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Microsoft.Graphics.Canvas;
using Catel.IoC;
using PinkAlert.Services;

namespace PinkAlert.Areas.MainMenu.ViewModels
{
    class NewCampaignViewModel : ViewModelBase
    {
        private static readonly Uri AlliedCampaignAnimationSourceUri = new Uri("ms-appx:///Assets/Art/fsalg.png");

        public CanvasBitmap AlliedCampaignAnimationSource { get; set; }
        public Command NewAlliedCampaignCommand { get; private set; }

        public NewCampaignViewModel()
        {
            NewAlliedCampaignCommand = new Command(OnNewAlliedCampaignCommand);
            LoadResources();
        }

        private void OnNewAlliedCampaignCommand()
        {
            var service = this.GetDependencyResolver().Resolve<IMainMenuService>();
            service.NavigateScene(new Uri("ms-appx:///Areas/MainMenu/Assets/LoadCampaignAtNewCampaignNavigation.xaml"));
        }

        private async void LoadResources()
        {
            var device = CanvasDevice.GetSharedDevice();
            AlliedCampaignAnimationSource = await CanvasBitmap.LoadAsync(device, AlliedCampaignAnimationSourceUri);
        }
    }
}
