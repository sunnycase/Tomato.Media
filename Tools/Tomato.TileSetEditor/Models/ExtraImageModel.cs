using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using Catel.Data;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Models
{
    class ExtraImageModel : ModelBase
    {
        public int Id { get; set; }

        public ExtraImage ExtraImage { get; private set; }

        public ImageSource ImageSource { get; set; }

        public ExtraImageModel(int id, ExtraImage extraImage, ImageSource imageSource)
        {
            Id = id;
            ExtraImage = extraImage;
            ImageSource = imageSource;
        }
    }
}
