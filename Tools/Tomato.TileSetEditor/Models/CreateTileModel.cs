using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media.Imaging;
using Catel.Data;
using Tomato.Tools.Common.Gaming;
using System.Windows.Media;

namespace Tomato.TileSetEditor.Models
{
    class CreateTileModel : ModelBase
    {
        public ExtraImageModel ExtraImage { get; set; }

        public Point ExtraImageOffset { get; set; }

        [Required]
        public ImageSource TileImageSource { get; set; }

        public int TileWidth { get; private set; }

        public int TileHeight { get; private set; }

        public CreateTileModel(int tileWidth, int tileHeight)
        {
            TileWidth = tileWidth;
            TileHeight = tileHeight;
        }
    }
}
