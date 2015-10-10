using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using Catel.Data;
using Catel.IoC;
using Tomato.TileSetEditor.Services;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Models
{
    class TileModel : ModelBase
    {
        public int Id { get; set; }

        public Tile Tile { get; private set; }

        public ImageSource TileImage { get; set; }

        public ExtraImageModel ExtraImage { get; set; }

        public Point ExtraImageOffset { get; set; }

        public TileModel(int id, Tile tile, ImageSource tileImage)
        {
            Id = id;
            Tile = tile;
            TileImage = tileImage;
            if (tile.ExtraImage != null)
            {
                var extraImage = this.GetDependencyResolver().Resolve<IExtraImageService>().GetExtraImageByTileId(tile.ExtraImage.Value.ExtraImage);
                var offset = tile.ExtraImage.Value.Offset;
                ExtraImage = extraImage;
                ExtraImageOffset = new Point(offset.X, offset.Y);
            }
        }

        private void OnExtraImageChanged()
        {
            if (ExtraImage == null)
                Tile.ExtraImage = null;
            else
            {
                Tile.ExtraImage = new ExtraImageRef
                {
                    ExtraImage = ExtraImage.Id,
                    Offset = new Offset { X = (int)ExtraImageOffset.X, Y = (int)ExtraImageOffset.Y }
                };
            }
        }

        private void OnExtraImageOffsetChanged()
        {
            if (ExtraImage != null)
            {
                Tile.ExtraImage = new ExtraImageRef
                {
                    ExtraImage = ExtraImage.Id,
                    Offset = new Offset { X = (int)ExtraImageOffset.X, Y = (int)ExtraImageOffset.Y }
                };
            }
        }
    }
}
