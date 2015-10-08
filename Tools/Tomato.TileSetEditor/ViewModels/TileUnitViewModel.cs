using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Tomato.TileSetEditor.Models;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.ViewModels
{
    class TileUnitViewModel : ViewModelBase
    {
        [Model]
        public TileUnitModel TileUnit { get; private set; }

        [ViewModelToModel("TileUnit")]
        public string Category { get; set; }

        [ViewModelToModel("TileUnit")]
        public int TileCount { get; set; }

        public TileUnitViewModel(TileUnit tileUnit)
        {
            TileUnit = new TileUnitModel(tileUnit);
        }
    }
}
