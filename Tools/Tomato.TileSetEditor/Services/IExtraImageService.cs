using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.TileSetEditor.Models;

namespace Tomato.TileSetEditor.Services
{
    interface IExtraImageService
    {
        ExtraImageModel GetExtraImageByTileId(int id);
        IEnumerable<ExtraImageModel> GetAllExtraImages();
    }
}
