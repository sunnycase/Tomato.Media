using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.IoC;
using Catel.MVVM;
using Catel.Services;
using Tomato.TileSetEditor.Models;

namespace Tomato.TileSetEditor.ViewModels
{
    class TileUnitsEditorViewModel : ViewModelBase
    {
        [Model]
        public TileUnitsEditorModel Model { get; private set; }

        [ViewModelToModel("Model")]
        public IReadOnlyCollection<TileUnitModel> TileUnits { get; private set; }

        public Command AddTileUnitCommand { get; }

        public TileUnitModel SelectedTileUnit { get; set; }

        public TileUnitsEditorViewModel(TileUnitsEditorModel model)
        {
            Model = model;
            AddTileUnitCommand = new Command(OnAddTileUnitCommand);
        }

        private void OnAddTileUnitCommand()
        {
            var uiVisualizerService = this.GetDependencyResolver().Resolve<IUIVisualizerService>();
            var viewModel = new CreateTileUnitViewModel();
            uiVisualizerService.ShowAsync(viewModel, (s, e) =>
            {
                if(e.Result == true)
                {
                    Model.AddTileUnit(viewModel.Category, viewModel.Size);
                }
            });
        }
    }
}
