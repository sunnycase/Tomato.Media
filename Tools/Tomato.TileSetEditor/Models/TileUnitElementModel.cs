using Catel.Data;
using System;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Models
{
    public class TileUnitElementModel : ModelBase
    {
        private readonly TileUnitElement[] _elements;
        private readonly int _index;

        public event Action Updated;

        public TileUnitElement Element
        {
            get { return _elements[_index]; }
            set { _elements[_index] = value; }
        }

        public int Index => _index;

        public TileUnitElementModel(TileUnitElement[] elements, int index)
        {
            _elements = elements;
            _index = index;
        }

        internal void SetTile(TileModel tile, int height)
        {
            if (Element == null)
                Element = new TileUnitElement
                {
                    Tile = tile.Id,
                    Height = height
                };
            else
            {
                Element.Tile = tile.Id;
                Element.Height = height;
            }
            Updated?.Invoke();
        }
    }
}