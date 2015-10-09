using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using Catel.Data;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Models
{
    class TileModel : ModelBase
    {
        public int Id { get; set; }

        public Tile Tile { get; private set; }

        public ImageSource TileImage { get; set; }

        public TileModel(int id, Tile tile, ImageSource tileImage)
        {
            Id = id;
            Tile = tile;
            TileImage = tileImage;
        }
    }
}
