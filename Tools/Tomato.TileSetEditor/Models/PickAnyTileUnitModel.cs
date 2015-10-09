using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.Data;
using Catel.IoC;
using Tomato.TileSetEditor.Services;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Models
{
    class PickAnyTileUnitModel : ModelBase
    {
        public PickAnyTileUnit TileUnit { get; }

        private readonly ObservableCollection<TileModel> _tiles;
        public IReadOnlyCollection<TileModel> Tiles
        {
            get { return _tiles; }
        }

        public PickAnyTileUnitModel(PickAnyTileUnit tileUnit)
        {
            TileUnit = tileUnit;
            _tiles = LoadTiles();
            RaisePropertyChanged(nameof(Tiles));
        }

        private ObservableCollection<TileModel> LoadTiles()
        {
            var tileService = this.GetDependencyResolver().Resolve<ITileService>();
            return new ObservableCollection<TileModel>(from i in TileUnit.Tiles
                                                       select tileService.GetTileByTileId(i));
        }
    }
}
