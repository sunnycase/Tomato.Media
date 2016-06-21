using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using Catel.MVVM;
using Tomato.TileSetEditor.Models;
using System.ComponentModel.DataAnnotations;

namespace Tomato.TileSetEditor.ViewModels
{
    class SelectTileWithHeightViewModel : ViewModelBase
    {
        public IEnumerable<TileModel> TilesSource { get; private set; }

        [Required]
        public TileModel SelectedTile { get; set; }

        public int Height { get; set; }

        public SelectTileWithHeightViewModel(IEnumerable<TileModel> tilesSource)
        {
            TilesSource = tilesSource;
        }
    }
}
