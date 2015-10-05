using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.ViewModels
{
    class TileUnitViewModel : ViewModelBase
    {
        [Model]
        public TileUnit TileUnit { get; private set; }

        [ViewModelToModel("TileUnit")]
        public string Category { get; set; }

        public int TileCount { get { return TileUnit.Tiles.Count / TileUnit.Size.Count; } }

        public TileUnitViewModel(TileUnit tileUnit)
        {
            TileUnit = tileUnit;
        }
    }
}
