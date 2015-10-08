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

        public int TileCount { get { return TileUnit.Tiles.Length / TileUnit.Size.Count; } }

        private List<DrawingOperation> _imageDrawingOperations = new List<DrawingOperation>();
        private List<DrawingOperation> _extraImageDrawingOperations = new List<DrawingOperation>();
        private Rect _boundingRect;
        private bool _isDirty = true;

        public TileUnitModel(TileUnit tileUnit)
        {
            TileUnit = tileUnit;
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
                var tileSetService = this.GetDependencyResolver().Resolve<ITileSetContextService>();
                DrawImageOperations(context, _imageDrawingOperations, tileSetService.Image);
                DrawImageOperations(context, _extraImageDrawingOperations, tileSetService.ExtraImage);
            }
            var output = new RenderTargetBitmap((int)_boundingRect.Width, (int)_boundingRect.Height, 96, 96, PixelFormats.Pbgra32);
            output.Render(drawing);
            output.Freeze();
            Output = output;
        }

        private void DrawImageOperations(DrawingContext drawingContext, IEnumerable<DrawingOperation> opeations, ImageSource imageSource)
        {
            if (imageSource != null)
            {
                foreach (var oper in opeations)
                {
                    drawingContext.PushClip(oper.Clip);
                    drawingContext.DrawImage(imageSource, oper.DestRect);
                    drawingContext.Pop();
                }
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

                var tileSetService = this.GetDependencyResolver().Resolve<ITileSetContextService>();
                var tileSize = tileSetService.TileSize;
                // 每次 z 坐标移动 1 格时的位移
                var zMoveVec = new Vector(tileSize.Width / 2.0, -tileSize.Height / 2.0);
                // 每次 x 坐标移动 1 格时的位移
                var xMoveVec = new Vector(tileSize.Width / 2.0, tileSize.Height / 2.0);
                // 每次 y 坐标移动 1 格时的位移
                var yMoveVec = new Vector(0.0, -tileSize.Height / 2.0);

                var boundingRect = Rect.Empty;
                int tileIdx = 0;
                var zLineBasePos = new Point();
                for (int cntX = 0; cntX < tileUnitSize.XLength; cntX++)
                {
                    var xLineBasePos = zLineBasePos;
                    for (int cntZ = 0; cntZ < tileUnitSize.ZLength; cntZ++)
                    {
                        if (TileUnit.Tiles[tileIdx].HasValue)
                        {
                            var tileUnitElem = TileUnit.Tiles[tileIdx].Value;
                            var tile = tileSetService.FindTile(tileUnitElem.Tile);

                            // 绘制 tile
                            var tileRect = Rect.Offset(new Rect(tileSize), yMoveVec * tileUnitElem.Height);
                            boundingRect.Union(tileRect);
                            _imageDrawingOperations.Add(new DrawingOperation
                            {
                                Clip = tileSetService.GetImageClip(tileUnitElem.Tile),
                                DestRect = tileRect
                            });

                            // 绘制 extra image
                            if (tile.ExtraImage.HasValue)
                            {
                                var extraImageRef = tile.ExtraImage.Value;
                                var extraImage = tileSetService.FindExtraImage(extraImageRef.ExtraImage);
                                var extraRect = new Rect(extraImageRef.Offset.X, extraImageRef.Offset.Y, extraImage.Width, extraImage.Height);
                                extraRect.Offset((Vector)tileRect.Location);
                                boundingRect.Union(extraRect);
                                _extraImageDrawingOperations.Add(new DrawingOperation
                                {
                                    Clip = tileSetService.GetExtraImageClip(extraImageRef.ExtraImage),
                                    DestRect = extraRect
                                });
                            }
                        }
                        tileIdx++;
                        xLineBasePos += zMoveVec;
                    }
                    zLineBasePos += xMoveVec;
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
            public RectangleGeometry Clip { get; set; }
            public Rect DestRect { get; set; }
        }
    }
}
