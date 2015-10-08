using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Services
{
    interface ITileSetContextService
    {
        ImageSource Image { get; }
        ImageSource ExtraImage { get; }
        Size TileSize { get; }
        Tile FindTile(int id);
        ExtraImage FindExtraImage(int id);
        RectangleGeometry GetImageClip(int tileId);
        RectangleGeometry GetExtraImageClip(int extraImageId);
    }
}
