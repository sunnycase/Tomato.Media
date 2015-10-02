using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Catel.IoC;
using PinkAlert.Services;

namespace PinkAlert.Areas.Campaign.ViewModels
{
    class LoadCampaignViewModel : ViewModelBase
    {
        public float LoadingProgress { get; private set; }

        public LoadCampaignViewModel()
        {
            LoadGame();
        }

        private async void LoadGame()
        {
            while (LoadingProgress <= 100.0f)
            {
                await Task.Delay(20);
                LoadingProgress += 1.0f;
            }
            this.GetDependencyResolver().Resolve<ILoadCampaignViewService>().NaviagteToGame();
        }
    }
}
