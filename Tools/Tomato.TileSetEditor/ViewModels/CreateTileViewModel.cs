using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Cache;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media.Imaging;
using Catel.Data;
using Catel.IoC;
using Catel.MVVM;
using Catel.Services;
using Tomato.TileSetEditor.Models;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.ViewModels
{
    class CreateTileViewModel : ViewModelBase
    {
        [Model]
        public CreateTileModel Model { get; private set; }

        [ViewModelToModel("Model")]
        public ExtraImageRef? ExtraImage { get; set; }

        [ViewModelToModel("Model")]
        public BitmapImage TileImageSource { get; set; }

        [ValidationToViewModel]
        public IValidationSummary ValidationSummary { get; private set; }

        public Command SelectTileImageSourceCommand { get; private set; }

        public bool HasExtraImage
        {
            get { return ExtraImage.HasValue; }
            set { ExtraImage = value ? (ExtraImageRef?)new ExtraImageRef() : null; }
        }

        public int ExtraImageId
        {
            get { return ExtraImage?.ExtraImage ?? 0; }
            set { ExtraImage = new ExtraImageRef { ExtraImage = value, Offset = ExtraImage.Value.Offset }; }
        }

        public Point ExtraImageOffset
        {
            get { return new Point(ExtraImage?.Offset.X ?? 0, ExtraImage?.Offset.Y ?? 0); }
            set { ExtraImage = new ExtraImageRef { ExtraImage = ExtraImageId, Offset = new Offset { X = (int)value.X, Y = (int)value.Y } }; }
        }

        public CreateTileViewModel(CreateTileModel model)
        {
            Model = model;
            SelectTileImageSourceCommand = new Command(OnSelectTileImageSourceCommand);
        }

        private void OnSelectTileImageSourceCommand()
        {
            var openFileService = this.GetDependencyResolver().Resolve<IOpenFileService>();
            openFileService.Filter = Constants.SupportedImageSourceFilter;
            openFileService.CheckFileExists = true;
            if (openFileService.DetermineFile())
            {
                var imageSource = new BitmapImage(new Uri(openFileService.FileName)) { CacheOption = BitmapCacheOption.OnLoad };
                if (imageSource.PixelWidth != Model.TileWidth || imageSource.PixelHeight != Model.TileHeight)
                {
                    var messageService = this.GetDependencyResolver().Resolve<IMessageService>();
                    messageService.ShowWarningAsync($"图片分辨率必须为 {Model.TileWidth}x{Model.TileHeight}。");
                }
                else
                    TileImageSource = imageSource;
            }
        }
    }
}
