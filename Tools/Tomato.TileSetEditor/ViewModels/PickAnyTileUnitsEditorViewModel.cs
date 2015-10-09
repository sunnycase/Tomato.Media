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
    class PickAnyTileUnitsEditorViewModel : ViewModelBase
    {
        [Model]
        public PickAnyTileUnitsEditorModel Model { get; private set; }

        [ViewModelToModel("Model")]
        public IReadOnlyCollection<PickAnyTileUnitModel> TileUnits { get; private set; }

        public Command AddTileUnitCommand { get; }

        public PickAnyTileUnitsEditorViewModel(PickAnyTileUnitsEditorModel model)
        {
            Model = model;
            AddTileUnitCommand = new Command(OnAddTileUnitCommand);
        }

        private void OnAddTileUnitCommand()
        {
            var uiVisualizerService = this.GetDependencyResolver().Resolve<IUIVisualizerService>();
            var viewModel = new CreatePickAnyTileUnitViewModel();
            uiVisualizerService.ShowDialog(viewModel, (s, e) =>
            {
                if(e.Result == true)
                {
                    Model.AddTileUnit(viewModel.Category);
                }
            });
        }
    }
}
