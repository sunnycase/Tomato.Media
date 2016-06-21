using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using System.Windows;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.ViewModels
{
    class CreateTileUnitViewModel : ViewModelBase
    {
        [Required]
        public string Category { get; set; }

        public TileUnitSize Size;

        public int XLength { get; set; }
        public int ZLength { get; set; }

        public CreateTileUnitViewModel()
        {

        }

        private void OnXLengthChanged()
        {
            Size.XLength = XLength;
        }

        private void OnZLengthChanged()
        {
            Size.ZLength = ZLength;
        }
    }
}
