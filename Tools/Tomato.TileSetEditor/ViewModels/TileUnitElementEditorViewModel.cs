using Catel.IoC;
using Catel.MVVM;
using Catel.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.TileSetEditor.Models;
using Tomato.TileSetEditor.Services;

namespace Tomato.TileSetEditor.ViewModels
{
    class TileUnitElementEditorViewModel : ViewModelBase
    {
        [Model]
        public TileUnitElementModel Model { get; private set; }

        public TileModel Tile { get; private set; }

        public int Index => Model.Index;

        public Command EditCommand { get; }

        private readonly ITileService _tileService;

        public TileUnitElementEditorViewModel(TileUnitElementModel model, ITileService tileService)
        {
            _tileService = tileService;
            Model = model;
            EditCommand = new Command(OnEditCommand);
            UpdateTile();
        }

        private void OnEditCommand()
        {
            var ui = this.GetDependencyResolver().Resolve<IUIVisualizerService>();
            var viewModel = new SelectTileWithHeightViewModel(_tileService.GetAllTiles());
            ui.ShowAsync(viewModel, (s, e) =>
            {
                if(e.Result == true)
                {
                    Model.SetTile(viewModel.SelectedTile, viewModel.Height);
                }
            });
        }

        private void UpdateTile()
        {
            var index = Model.Element?.Tile;
            if(index.HasValue)
            {
                Tile = _tileService.GetTileByTileId(index.Value);
            }
        }
    }
}
