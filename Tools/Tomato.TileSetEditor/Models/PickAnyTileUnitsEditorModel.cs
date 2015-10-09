using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.Data;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Models
{
    class PickAnyTileUnitsEditorModel : ModelBase
    {
        private readonly TileSetModel _tileSet;
        private ObservableCollection<PickAnyTileUnitModel> _tileUnits;
        public IReadOnlyCollection<PickAnyTileUnitModel> TileUnits
        {
            get { return _tileUnits; }
        }

        public PickAnyTileUnitsEditorModel(TileSetModel tileSet)
        {
            _tileSet = tileSet;
            _tileUnits = LoadTileUnits();
            RaisePropertyChanged(nameof(TileUnits));
        }

        private ObservableCollection<PickAnyTileUnitModel> LoadTileUnits()
        {
            return new ObservableCollection<PickAnyTileUnitModel>(from tu in _tileSet.TileSet.PickAnyTileUnits
                                                                  select new PickAnyTileUnitModel(tu));
        }


    }
}
