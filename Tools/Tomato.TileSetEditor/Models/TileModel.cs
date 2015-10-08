using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.Data;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Models
{
    class TileModel : ModelBase
    {
        public int Id { get; private set; }
        public Tile Tile { get; private set; }

        public TileModel(int id, Tile tile)
        {
            Id = id;
            Tile = tile;
        }
    }
}
