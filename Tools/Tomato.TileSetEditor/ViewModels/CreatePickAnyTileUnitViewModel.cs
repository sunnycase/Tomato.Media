using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;

namespace Tomato.TileSetEditor.ViewModels
{
    class CreatePickAnyTileUnitViewModel : ViewModelBase
    {
        [Required]
        public string Category { get; set; }

        public CreatePickAnyTileUnitViewModel()
        {

        }
    }
}
