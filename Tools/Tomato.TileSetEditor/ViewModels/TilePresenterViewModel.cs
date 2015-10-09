using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using Catel.MVVM;
using Tomato.TileSetEditor.Models;

namespace Tomato.TileSetEditor.ViewModels
{
    class TilePresenterViewModel : ViewModelBase
    {
        [Model]
        public TileModel Tile { get; private set; }

        [ViewModelToModel("Tile")]
        public ImageSource TileImage { get; set; }

        public TilePresenterViewModel(TileModel tile)
        {
            Tile = tile;
        }
    }
}
