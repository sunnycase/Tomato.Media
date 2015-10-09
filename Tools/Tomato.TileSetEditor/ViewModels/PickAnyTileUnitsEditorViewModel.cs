using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Tomato.TileSetEditor.Models;

namespace Tomato.TileSetEditor.ViewModels
{
    class PickAnyTileUnitsEditorViewModel : ViewModelBase
    {
        [Model]
        public PickAnyTileUnitsEditorModel Model { get; private set; }

        [ViewModelToModel("Model")]
        public IReadOnlyCollection<PickAnyTileUnitModel> TileUnits { get; private set; }

        public PickAnyTileUnitsEditorViewModel(PickAnyTileUnitsEditorModel model)
        {
            Model = model;
        }
    }
}
