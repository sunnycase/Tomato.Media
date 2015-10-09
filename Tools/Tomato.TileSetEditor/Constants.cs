using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tomato.TileSetEditor
{
    static class Constants
    {
        public const string TileSetJsonFileName = "tileSet.json";
        public const string ImageFileName = "image.dds";
        public const string ExtraImageFileName = "extraImage.dds";
        public const string TileSetFilter = "Tomato TileSet (*.tts)|*.tts";
        public const string SupportedImageSourceFilter = "Image|*.bmp;*tiff;*.dds;*.png;*.jpg;*.jpe;*.jpeg";

        public static readonly Guid GUID_ContainerFormatDds = new Guid("9967cb95-2e85-4ac8-8ca283d7ccd425c9");
        public static readonly Guid CLSID_WICDdsEncoder = new Guid("a61dde94-66ce-4ac1-881b71680588895e");
    }
}
