using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Catel.Data;
using Catel.IoC;
using Tomato.TileSetEditor.Services;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Models
{
    class TilesEditorModel : ModelBase, ITileService
    {
        private readonly TileSetModel _tileSetModel;

        private ObservableCollection<TileModel> _tiles;
        public IReadOnlyCollection<TileModel> Tiles
        {
            get { return _tiles; }
        }

        public int TileWidth { get; private set; }
        public int TileHeight { get; private set; }

        public TilesEditorModel(TileSetModel tileSetModel, BitmapSource tileImage)
        {
            _tileSetModel = tileSetModel;

            var tileSet = tileSetModel.TileSet;
            TileWidth = tileSet.TileWidth;
            TileHeight = tileSet.TileHeight;
            _tiles = new ObservableCollection<TileModel>(_tileSetModel.TileSet.Tiles.Select((t, i) => new TileModel(i, t, LoadImageFromTileId(i, tileImage))));
            RaisePropertyChanged(nameof(Tiles));
            _tiles.CollectionChanged += _tiles_CollectionChanged;

            this.GetServiceLocator().RegisterInstance<ITileService>(this);
        }

        public void AddTile(ImageSource tileImage, ExtraImageRef? extraImage)
        {
            _tiles.Add(new TileModel(_tiles.Count, new Tile()
            {
                ExtraImage = extraImage
            }, tileImage));
        }

        public BitmapSource CreateMergedTileImage()
        {
            if (_tiles.Count != 0)
            {
                var columns = Math.Max(6, (int)Math.Sqrt(_tiles.Count));
                var rows = (int)Math.Ceiling((double)_tiles.Count / columns);
                var drawing = new DrawingVisual();
                int id = 0;
                using (var context = drawing.RenderOpen())
                {
                    for (int cntRow = 0; cntRow < rows; cntRow++)
                    {
                        for (int cntCol = 0; cntCol < columns; cntCol++)
                        {
                            if (id >= _tiles.Count) break;
                            var rect = new Rect(cntCol * TileWidth, cntRow * TileHeight, TileWidth, TileHeight);
                            context.DrawImage(_tiles[id++].TileImage, rect);
                        }
                    }
                }
                var bitmap = new RenderTargetBitmap(columns * TileWidth, rows * TileHeight, 96, 96, PixelFormats.Pbgra32);
                bitmap.Render(drawing);
                return bitmap;
            }
            return new RenderTargetBitmap(4, 4, 96, 96, PixelFormats.Pbgra32);
        }

        private void _tiles_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            switch (e.Action)
            {
                case System.Collections.Specialized.NotifyCollectionChangedAction.Add:
                    foreach (TileModel item in e.NewItems)
                        _tileSetModel.TileSet.Tiles.Add(item.Tile);
                    break;
                case System.Collections.Specialized.NotifyCollectionChangedAction.Remove:
                    foreach (TileModel item in e.OldItems)
                        _tileSetModel.TileSet.Tiles.Remove(item.Tile);
                    break;
                case System.Collections.Specialized.NotifyCollectionChangedAction.Reset:
                    _tileSetModel.TileSet.Tiles.Clear();
                    foreach (var item in _tiles)
                        _tileSetModel.TileSet.Tiles.Add(item.Tile);
                    break;
            }
        }

        public BitmapSource LoadImageFromTileId(int id, BitmapSource tileImage)
        {
            var image = new WriteableBitmap(TileWidth, TileHeight, 96, 96, PixelFormats.Pbgra32, null);
            var tilePerRow = tileImage.PixelWidth / TileWidth;
            var rect = new Int32Rect(id % tilePerRow * TileWidth, id / tilePerRow * TileHeight, TileWidth, TileHeight);
            image.Lock();
            tileImage.CopyPixels(rect, image.BackBuffer, image.BackBufferStride * image.PixelHeight, image.BackBufferStride);
            image.Unlock();
            image.Freeze();
            return image;
        }

        public TileModel GetTileByTileId(int id)
        {
            if (id < _tiles.Count)
                return _tiles[id];
            throw new ArgumentOutOfRangeException(nameof(id));
        }

        public IEnumerable<TileModel> GetAllTiles()
        {
            return _tiles;
        }
    }
}
