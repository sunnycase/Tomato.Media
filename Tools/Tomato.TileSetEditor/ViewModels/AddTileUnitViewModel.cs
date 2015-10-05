using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.Data;
using Catel.MVVM;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.ViewModels
{
    class AddTileUnitViewModel : ViewModelBase
    {
        [Model]
        public TileUnit TileUnit { get; private set; }

        [ViewModelToModel("TileUnit")]
        public Size Size { get; set; }

        [ViewModelToModel("TileUnit")]
        public string Category { get; set; }

        [ValidationToViewModel]
        public IValidationSummary ValidationSummary { get; set; }

        public int XLength
        {
            get { return Size.XLength; }
            set { Size = new Size { XLength = value, ZLength = Size.ZLength }; }
        }

        public int ZLength
        {
            get { return Size.ZLength; }
            set { Size = new Size { XLength = Size.XLength, ZLength = value }; }
        }

        public AddTileUnitViewModel()
        {
            TileUnit = new TileUnit()
            {
                Category = "新地形",
                Size = new Size { XLength = 1, ZLength = 1 }
            };
        }
    }
}
