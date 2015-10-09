using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;

namespace Tomato.TileSetEditor.Models
{
    class CreateExtraImageModel
    {
        [Required]
        public BitmapImage ExtraImageSource { get; set; }

        public CreateExtraImageModel()
        {

        }
    }
}
