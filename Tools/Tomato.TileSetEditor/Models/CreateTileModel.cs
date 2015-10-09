using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;
using Catel.Data;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Models
{
    class CreateTileModel : ModelBase
    {
        public ExtraImageRef? ExtraImage { get; set; }

        [Required]
        public BitmapImage TileImageSource { get; set; }

        public int TileWidth { get; private set; }

        public int TileHeight { get; private set; }

        public CreateTileModel(int tileWidth, int tileHeight)
        {
            TileWidth = tileWidth;
            TileHeight = tileHeight;
        }
    }
}
