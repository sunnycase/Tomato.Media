using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Catel.Data;
using Catel.Fody;
using ICSharpCode.SharpZipLib.Core;
using ICSharpCode.SharpZipLib.Zip;
using Newtonsoft.Json;
using Tomato.TileSetEditor.Services;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Models
{
    class TileSetModel : ModelBase
    {
        private readonly TileSet _tileSet;
        public TileSet TileSet { get { return _tileSet; } }
        public string FileName { get; private set; }
        public BitmapSource ExtraImage { get; private set; }
        public string Name { get; set; }

        public ExtraImagesEditorModel ExtraImagesEditor { get; private set; }
        public TilesEditorModel TilesEditor { get; private set; }
        public PickAnyTileUnitsEditorModel PickAnyTileUnitsEditor { get; private set; }

        public TileSetModel([NotNull]TileSet tileSet, string fileName, BitmapSource image, BitmapSource extraImage)
        {
            _tileSet = tileSet;
            FileName = fileName;
            ExtraImage = extraImage;

            Name = tileSet.Name;
            ExtraImagesEditor = new ExtraImagesEditorModel(this, extraImage);
            TilesEditor = new TilesEditorModel(this, image);
            PickAnyTileUnitsEditor = new PickAnyTileUnitsEditorModel(this);
        }

        private void OnNameChanged()
        {
            _tileSet.Name = Name;
        }

        public async Task SaveAsync()
        {
            var buffer = new byte[4096];
            using (var zipStream = new ZipOutputStream(new FileStream(FileName, FileMode.Create)) { IsStreamOwner = true })
            {
                zipStream.SetLevel(3);

                zipStream.PutNextEntry(new ZipEntry(Constants.TileSetJsonFileName) { DateTime = DateTime.Now });
                await SaveTileSetJson(zipStream, buffer);
                zipStream.CloseEntry();

                zipStream.PutNextEntry(new ZipEntry(Constants.ImageFileName) { DateTime = DateTime.Now });
                await SaveImageSource(zipStream, TilesEditor.CreateMergedTileImage(), buffer);
                zipStream.CloseEntry();

                zipStream.PutNextEntry(new ZipEntry(Constants.ExtraImageFileName) { DateTime = DateTime.Now });
                await SaveImageSource(zipStream, ExtraImagesEditor.CreateMergedExtraImage(), buffer);
                zipStream.CloseEntry();
            }
        }

        private async Task SaveTileSetJson(Stream outputStream, byte[] buffer)
        {
            using (var stream = new MemoryStream(Encoding.UTF8.GetBytes(JsonConvert.SerializeObject(_tileSet))))
            {
                StreamUtils.Copy(stream, outputStream, buffer);
            }
        }

        private async Task SaveImageSource(Stream outputStream, BitmapSource bitmapSource, byte[] buffer)
        {
            using (var stream = new MemoryStream())
            {
                var encoder = new PngBitmapEncoder();
                var bitmapFrame = BitmapFrame.Create(bitmapSource);
                encoder.Frames.Add(bitmapFrame);
                encoder.Save(stream);
                stream.Position = 0;

                StreamUtils.Copy(stream, outputStream, buffer);
            }
        }

        public static async Task<TileSetModel> Load(string fileName)
        {
            var buffer = new byte[4096];
            using (var zipFile = new ZipFile(new FileStream(fileName, FileMode.Open)) { IsStreamOwner = true })
            {
                var tileSet = await LoadTileSetJson(zipFile.GetInputStream(zipFile.GetEntry(Constants.TileSetJsonFileName)), buffer);
                var image = await LoadImageSource(zipFile.GetInputStream(zipFile.GetEntry(Constants.ImageFileName)), buffer);
                var extraImage = await LoadImageSource(zipFile.GetInputStream(zipFile.GetEntry(Constants.ExtraImageFileName)), buffer);

                return new TileSetModel(tileSet, fileName, image, extraImage);
            }
        }

        private static async Task<TileSet> LoadTileSetJson(Stream inputStream, byte[] buffer)
        {
            using (var stream = new MemoryStream())
            {
                StreamUtils.Copy(inputStream, stream, buffer);
                stream.Position = 0;
                var json = Encoding.UTF8.GetString(stream.ToArray());
                return JsonConvert.DeserializeObject<TileSet>(json);
            }
        }

        private static async Task<BitmapSource> LoadImageSource(Stream inputStream, byte[] buffer)
        {
            using (var stream = new MemoryStream())
            {
                StreamUtils.Copy(inputStream, stream, buffer);
                stream.Position = 0;

                var decoder = new PngBitmapDecoder(stream, BitmapCreateOptions.None, BitmapCacheOption.OnLoad);
                return decoder.Frames.First();
            }
        }
    }
}
