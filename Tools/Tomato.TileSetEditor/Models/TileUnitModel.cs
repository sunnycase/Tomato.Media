using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using Catel.Data;
using Tomato.Tools.Common.Gaming;
using Catel.IoC;

namespace Tomato.TileSetEditor.Models
{
    using System.Diagnostics;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using Services;

    class TileUnitModel : ModelBase
    {
        public TileUnit TileUnit { get; private set; }
        public ImageSource Output { get; private set; }

        public string Category
        {
            get { return TileUnit.Category; }
            set { TileUnit.Category = value; }
        }

        public TileUnitSize Size => TileUnit.Size;

        private List<DrawingOperation> _imageDrawingOperations = new List<DrawingOperation>();
        private List<DrawingOperation> _extraImageDrawingOperations = new List<DrawingOperation>();
        private Rect _boundingRect;
        private bool _isDirty = true;

        public TileUnitElementModel[] Elements { get; }

        public TileUnitModel(TileUnit tileUnit)
        {
            TileUnit = tileUnit;
            Elements = tileUnit.Tiles.Select((o, i) => new TileUnitElementModel(tileUnit.Tiles, i)).ToArray();
            foreach (var item in Elements)
                item.Updated += Item_Updated;
            Draw();
        }

        private void Item_Updated()
        {
            _isDirty = true;
            Draw();
        }

        public void Draw()
        {
            BuildDrawingOperations();
            DoDrawing();
        }

        private void DoDrawing()
        {
            var drawing = new DrawingVisual();
            using (var context = drawing.RenderOpen())
            {
                DrawImageOperations(context, _imageDrawingOperations);
                DrawImageOperations(context, _extraImageDrawingOperations);
            }
            var output = new RenderTargetBitmap((int)_boundingRect.Width, (int)_boundingRect.Height, 96, 96, PixelFormats.Pbgra32);
            output.Render(drawing);
            output.Freeze();
            Output = output;
        }

        private void DrawImageOperations(DrawingContext drawingContext, IEnumerable<DrawingOperation> opeations)
        {
            foreach (var oper in opeations)
            {
                drawingContext.DrawImage(oper.Image, oper.DestRect);
            }
        }

        private void BuildDrawingOperations()
        {
            if (_isDirty)
            {
                _imageDrawingOperations.Clear();
                _extraImageDrawingOperations.Clear();

                var tileUnitSize = TileUnit.Size;
                var totalTileCount = tileUnitSize.Count;
                if (TileUnit.Tiles.Length != totalTileCount)
                    throw new ArgumentException("Invalid tiles count in tileunit.");

                var tileSetService = this.GetDependencyResolver().Resolve<ITileService>();
                var tileSize = this.GetDependencyResolver().Resolve<ITileSetContextService>().TileSize;
                // 每次 z 坐标移动 1 格时的位移
                var zMoveVec = new Vector(tileSize.Width / 2.0, -tileSize.Height / 2.0);
                // 每次 x 坐标移动 1 格时的位移
                var xMoveVec = new Vector(tileSize.Width / 2.0, tileSize.Height / 2.0);
                // 每次 y 坐标移动 1 格时的位移
                var yMoveVec = new Vector(0.0, -tileSize.Height / 2.0);

                var boundingRect = Rect.Empty;
                int tileIdx = 0;
                for (int cntX = 0; cntX < tileUnitSize.XLength; cntX++)
                {
                    for (int cntZ = 0; cntZ < tileUnitSize.ZLength; cntZ++)
                    {
                        if (TileUnit.Tiles[tileIdx] != null)
                        {
                            var tileUnitElem = TileUnit.Tiles[tileIdx];
                            var tile = tileSetService.GetTileByTileId(tileUnitElem.Tile);

                            // 绘制 tile
                            var tileRect = Rect.Offset(new Rect(tileSize), yMoveVec * tileUnitElem.Height + xMoveVec * cntX + zMoveVec * cntZ);
                            boundingRect.Union(tileRect);
                            _imageDrawingOperations.Add(new DrawingOperation
                            {
                                Image = tile.TileImage,
                                DestRect = tileRect
                            });

                            // 绘制 extra image
                            if (tile.ExtraImage != null)
                            {
                                var extraImageRef = tile.ExtraImage;
                                var extraImage = extraImageRef.ImageSource;
                                var extraRect = new Rect(tile.ExtraImageOffset.X, tile.ExtraImageOffset.Y, extraImage.Width, extraImage.Height);
                                extraRect.Offset((Vector)tileRect.Location);
                                boundingRect.Union(extraRect);
                                _extraImageDrawingOperations.Add(new DrawingOperation
                                {
                                    Image = extraImage,
                                    DestRect = extraRect
                                });
                            }
                        }
                        tileIdx++;
                    }
                }
                // 修正 boudingRect
                var offset = new Vector(-boundingRect.Left, -boundingRect.Top);
                if (offset != new Vector())
                {
                    foreach (var oper in _imageDrawingOperations)
                        oper.DestRect = Rect.Offset(oper.DestRect, offset);
                    foreach (var oper in _extraImageDrawingOperations)
                        oper.DestRect = Rect.Offset(oper.DestRect, offset);
                }

                _boundingRect = Rect.Offset(boundingRect, offset);
                Debug.Assert(_boundingRect.Location == new Point());
                _isDirty = false;
            }
        }

        private class DrawingOperation
        {
            public ImageSource Image { get; set; }
            public Rect DestRect { get; set; }
        }
    }
}
