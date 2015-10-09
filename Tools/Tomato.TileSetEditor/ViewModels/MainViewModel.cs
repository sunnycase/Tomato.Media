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
        public TaskCommand SaveTileSetCommand { get; }
        
        public TileSetModel TileSet { get; private set; }

        public MainViewModel()
        {
            NewTileSetCommand = new Command(OnNewTileSetCommand);
            OpenTileSetCommand = new Command(OnOpenTileSetCommand);
            SaveTileSetCommand = new TaskCommand(OnSaveTileSetCommand, () => TileSet != null && !SaveTileSetCommand.IsExecuting);
        }

        private async Task OnSaveTileSetCommand()
        {
            await TileSet.SaveAsync();
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
            var model = new CreateTileSetModel();
            var uiVisualizerService = this.GetDependencyResolver().Resolve<IUIVisualizerService>();
            uiVisualizerService.ShowDialog<CreateTileSetViewModel>(model, async (s, e) =>
            {
                if (e.Result == true)
                    TileSet = await model.Create();
            });
        }

        private async void LoadTileSet(string fileName)
        {
            TileSet = await TileSetModel.Load(fileName);
        }
    }
}
