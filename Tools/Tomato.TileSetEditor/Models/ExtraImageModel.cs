using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
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

        public Point Location { get; set; }

        public Size Size { get; set; }

        public ExtraImageModel(int id, ExtraImage extraImage, ImageSource imageSource)
        {
            Id = id;
            ExtraImage = extraImage;
            ImageSource = imageSource;
            Location = new Point(ExtraImage.X, ExtraImage.Y);
            Size = new Size(ExtraImage.Width, ExtraImage.Height);
        }

        private void OnLocationChanged()
        {
            ExtraImage.X = (int)Location.X;
            ExtraImage.Y = (int)Location.Y;
        }

        private void OnSizeChanged()
        {
            ExtraImage.Width = (int)Size.Width;
            ExtraImage.Height = (int)Size.Height;
        }
    }
}
