using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Tomato.TileSetEditor.Models;

namespace Tomato.TileSetEditor.Services
{
    interface ITileService
    {
        TileModel GetTileByTileId(int id);
        IEnumerable<TileModel> GetAllTiles();
    }
}
