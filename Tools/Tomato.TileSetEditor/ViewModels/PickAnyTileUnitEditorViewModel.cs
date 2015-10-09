using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.IoC;
using Catel.MVVM;
using Catel.Services;
using Tomato.TileSetEditor.Models;
using Tomato.TileSetEditor.Services;

namespace Tomato.TileSetEditor.ViewModels
{
    class PickAnyTileUnitEditorViewModel : ViewModelBase
    {
        [Model]
        public PickAnyTileUnitModel Model { get; private set; }

        [ViewModelToModel("Model")]
        public IReadOnlyCollection<TileModel> Tiles { get; set; }

        public Command AddTilesCommand { get; }

        public PickAnyTileUnitEditorViewModel(PickAnyTileUnitModel model)
        {
            Model = model;
            AddTilesCommand = new Command(OnAddTilesCommand);
        }

        private void OnAddTilesCommand()
        {
            var uiVisualizerService = this.GetDependencyResolver().Resolve<IUIVisualizerService>();
            var tileService = this.GetDependencyResolver().Resolve<ITileService>();
            var viewModel = new SelectTilesViewModel(tileService.GetAllTiles().Except(Tiles))
            {
                SelectionMode = System.Windows.Controls.SelectionMode.Multiple
            };
            uiVisualizerService.ShowDialog(viewModel, (s, e) =>
            {
                if (e.Result == true)
                    foreach (var item in viewModel.SelectedTiles)
                        Model.AddTile(item);
            });
        }
    }
}
