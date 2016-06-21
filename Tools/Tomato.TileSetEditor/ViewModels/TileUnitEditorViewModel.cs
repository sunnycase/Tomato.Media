using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Tomato.TileSetEditor.Models;
using Tomato.Tools.Common.Gaming;
using System.Windows.Media;

namespace Tomato.TileSetEditor.ViewModels
{
    class TileUnitEditorViewModel : ViewModelBase
    {
        [Model]
        public TileUnitModel TileUnit { get; private set; }

        [ViewModelToModel("TileUnit")]
        public string Category { get; set; }

        [ViewModelToModel("TileUnit")]
        public ImageSource Output { get; set; }

        public TileUnitEditorViewModel(TileUnitModel tileUnit)
        {
            TileUnit = tileUnit;
        }
    }
}
