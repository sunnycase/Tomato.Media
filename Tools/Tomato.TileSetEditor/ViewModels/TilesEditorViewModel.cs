using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.Fody;
using Catel.MVVM;
using Tomato.TileSetEditor.Models;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.ViewModels
{
    class TilesEditorViewModel : ViewModelBase
    {
        [Model]
        public TilesEditorModel Model { get; private set; }

        [ViewModelToModel("Model")]
        public IEnumerable<Tile> Tiles { get; private set; }

        public TilesEditorViewModel([NotNull]TilesEditorModel model)
        {
            Model = model;
        }
    }
}
