using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.Data;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Models
{
    class TilesEditorModel : ModelBase
    {
        private readonly TileSetModel _tileSetModel;

        private ObservableCollection<TileModel> _tiles;
        public IEnumerable<TileModel> Tiles
        {
            get { return _tiles; }
        }
        
        public int MaxTileCount { get; private set; }

        public TilesEditorModel(TileSetModel tileSetModel)
        {
            _tileSetModel = tileSetModel;

            var tileSet = tileSetModel.TileSet;
            _tiles = new ObservableCollection<TileModel>(from t in _tileSetModel.TileSet.Tiles
                                                         select new TileModel(t.Key, t.Value));
            RaisePropertyChanged(nameof(Tiles));
            MaxTileCount = tileSetModel.GetMaxTileCount();
            _tiles.CollectionChanged += _tiles_CollectionChanged;
        }

        private void AddTile()
        {
            if (_tiles.Count + 1 > MaxTileCount)
                throw new NotSupportedException("Cannot add more tile.");
            var id = _tiles.Count;
            var tile = new Tile();
            _tiles.Add(new TileModel(id, tile));
        }

        private void _tiles_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            switch (e.Action)
            {
                case System.Collections.Specialized.NotifyCollectionChangedAction.Add:
                    foreach (TileModel item in e.NewItems)
                        _tileSetModel.TileSet.Tiles.Add(item.Id, item.Tile);
                    break;
                case System.Collections.Specialized.NotifyCollectionChangedAction.Remove:
                    foreach (TileModel item in e.OldItems)
                        _tileSetModel.TileSet.Tiles.Remove(item.Id);
                    break;
                case System.Collections.Specialized.NotifyCollectionChangedAction.Reset:
                    _tileSetModel.TileSet.Tiles.Clear();
                    foreach (var item in _tiles)
                        _tileSetModel.TileSet.Tiles.Add(item.Id, item.Tile);
                    break;
            }
        }
    }
}
