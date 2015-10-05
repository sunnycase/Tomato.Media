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

namespace Tomato.TileSetEditor.ViewModels
{
    class TileSetContext
    {
        public TileSet TileSet { get; set; }
        public ImageSource Image { get; set; }
        public ImageSource ExtraImage { get; set; }
    }

    class MainViewModel : ViewModelBase
    {
        public Command NewTileSetCommand { get; }
        public Command OpenTileSetCommand { get; }
        public Command BrowseImageCommand { get; }
        public Command AddTileUnitCommand { get; }

        private TileSet _tileSet;
        public TileSetContext TileSet { get; private set; }

        public MainViewModel()
        {
            NewTileSetCommand = new Command(OnNewTileSetCommand);
            OpenTileSetCommand = new Command(OnOpenTileSetCommand);
            BrowseImageCommand = new Command(OnBrowseImageCommand);
            AddTileUnitCommand = new Command(OnAddTileUnitCommand);
        }

        private void OnAddTileUnitCommand()
        {
            var uiVisualizerService = this.GetDependencyResolver().Resolve<IUIVisualizerService>();
            uiVisualizerService.ShowDialog<AddTileUnitViewModel>(completedProc: OnAddTileUnitCompleted);
        }

        private void OnAddTileUnitCompleted(object sender, UICompletedEventArgs e)
        {
            if(e.Result == true)
            {
                _tileSet.TileUnits.Add(((AddTileUnitViewModel)e.DataContext).TileUnit);
            }
        }

        private void OnBrowseImageCommand()
        {

        }

        private void OnOpenTileSetCommand()
        {
            var openFileService = this.GetDependencyResolver().Resolve<IOpenFileService>();
            openFileService.Filter = "TileSet (*.tts)|*.tts";
            openFileService.CheckFileExists = true;
            if (openFileService.DetermineFile())
                LoadTileSet(openFileService.FileName);
        }

        private void OnNewTileSetCommand()
        {
            _tileSet = new TileSet();
            TileSet = new TileSetContext
            {
                TileSet = _tileSet
            };
        }

        private void LoadTileSet(string path)
        {
            var dir = Path.GetDirectoryName(path);
            _tileSet = JsonConvert.DeserializeObject<TileSet>(File.ReadAllText(path));
            LoadTileSetContext(dir);
        }

        private void LoadTileSetContext(string directory)
        {
            TileSet = new TileSetContext
            {
                TileSet = _tileSet,
                Image = string.IsNullOrEmpty(_tileSet.Image) ?
                    new BitmapImage(new Uri(Path.Combine(directory, _tileSet.Image))) : null
            };
        }
    }
}
