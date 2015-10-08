using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Catel.Services;
using Catel.IoC;
using System.IO;
using Tomato.Tools.Common.Gaming;
using Newtonsoft.Json;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Tomato.TileSetEditor.Models;

namespace Tomato.TileSetEditor.ViewModels
{
    class MainViewModel : ViewModelBase
    {
        public Command NewTileSetCommand { get; }
        public Command OpenTileSetCommand { get; }
        
        public TileSetModel TileSet { get; private set; }

        public MainViewModel()
        {
            NewTileSetCommand = new Command(OnNewTileSetCommand);
            OpenTileSetCommand = new Command(OnOpenTileSetCommand);
        }

        private void OnOpenTileSetCommand()
        {
            var openFileService = this.GetDependencyResolver().Resolve<IOpenFileService>();
            openFileService.Filter = Constants.TileSetFilter;
            openFileService.CheckFileExists = true;
            if (openFileService.DetermineFile())
                LoadTileSet(openFileService.FileName);
        }

        private void OnNewTileSetCommand()
        {
            var uiVisualizerService = this.GetDependencyResolver().Resolve<IUIVisualizerService>();
            uiVisualizerService.ShowDialog<CreateTileSetViewModel>(completedProc: OnNewTileSetCompleted);
        }

        private async void OnNewTileSetCompleted(object sender, UICompletedEventArgs e)
        {
            if (e.Result == true)
            {
                var viewModel = (CreateTileSetViewModel)e.DataContext;
                TileSet = await viewModel.Model.Create();
            }
        }

        private async void LoadTileSet(string fileName)
        {
            TileSet = await TileSetModel.Load(fileName);
        }
    }
}
