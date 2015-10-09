using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
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

        public TileModel FirstTile { get; private set; }

        public string Category { get; set; }

        public PickAnyTileUnitModel(PickAnyTileUnit tileUnit)
        {
            TileUnit = tileUnit;
            _tiles = LoadTiles();
            _tiles.CollectionChanged += _tiles_CollectionChanged;
            RaisePropertyChanged(nameof(Tiles));
            Category = tileUnit.Category;
            UpdateFirstTile();
        }

        public void AddTile(TileModel tile)
        {
            if (!_tiles.Contains(tile))
                _tiles.Add(tile);
        }

        private void _tiles_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            UpdateFirstTile();
            switch (e.Action)
            {
                case System.Collections.Specialized.NotifyCollectionChangedAction.Add:
                    foreach (TileModel item in e.NewItems)
                        TileUnit.Tiles.Add(item.Id);
                    break;
                case System.Collections.Specialized.NotifyCollectionChangedAction.Remove:
                    foreach (TileModel item in e.OldItems)
                        TileUnit.Tiles.Remove(item.Id);
                    break;
                case System.Collections.Specialized.NotifyCollectionChangedAction.Reset:
                    TileUnit.Tiles.Clear();
                    foreach (var item in _tiles)
                        TileUnit.Tiles.Add(item.Id);
                    break;
            }
        }

        private void UpdateFirstTile()
        {
            FirstTile = Tiles.FirstOrDefault();
        }

        private ObservableCollection<TileModel> LoadTiles()
        {
            var tileService = this.GetDependencyResolver().Resolve<ITileService>();
            return new ObservableCollection<TileModel>(from i in TileUnit.Tiles
                                                       select tileService.GetTileByTileId(i));
        }

        private void OnCategoryChanged()
        {
            TileUnit.Category = Category;
        }
    }
}
