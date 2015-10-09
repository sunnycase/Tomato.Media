using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using Catel.MVVM;
using Tomato.TileSetEditor.Models;

namespace Tomato.TileSetEditor.ViewModels
{
    class SelectTilesViewModel : ViewModelBase
    {
        public IEnumerable<TileModel> TilesSource { get; private set; }

        public ObservableCollection<TileModel> SelectedTiles { get; private set; }

        public SelectionMode SelectionMode { get; set; }

        public SelectTilesViewModel(IEnumerable<TileModel> tilesSource)
        {
            TilesSource = tilesSource;
            SelectedTiles = new ObservableCollection<TileModel>();
        }
    }
}
