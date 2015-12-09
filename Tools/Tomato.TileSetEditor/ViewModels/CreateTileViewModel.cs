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
using Tomato.TileSetEditor.Services;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.ViewModels
{
    class CreateTileViewModel : ViewModelBase
    {
        [Model]
        public CreateTileModel Model { get; private set; }

        [ViewModelToModel("Model")]
        public ExtraImageModel ExtraImage { get; set; }

        [ViewModelToModel("Model")]
        public Point ExtraImageOffset { get; set; }

        [ViewModelToModel("Model")]
        public BitmapImage TileImageSource { get; set; }

        [ValidationToViewModel]
        public IValidationSummary ValidationSummary { get; private set; }

        public Command SelectTileImageSourceCommand { get; }

        public Command SelectExtraImageCommand { get; }

        public bool HasExtraImage { get; set; }

        public CreateTileViewModel(CreateTileModel model)
        {
            Model = model;
            SelectTileImageSourceCommand = new Command(OnSelectTileImageSourceCommand);
            SelectExtraImageCommand = new Command(OnSelectExtraImageCommand);
        }

        private void OnSelectExtraImageCommand()
        {
            var uiVisualizerService = this.GetDependencyResolver().Resolve<IUIVisualizerService>();
            var extraImageService = this.GetDependencyResolver().Resolve<IExtraImageService>();
            var viewModel = new SelectExtraImageViewModel(extraImageService.GetAllExtraImages())
            {
                SelectedExtraImage = ExtraImage
            };
            uiVisualizerService.ShowDialog(viewModel, (s, e) =>
            {
                if (e.Result == true)
                    ExtraImage = viewModel.SelectedExtraImage;
            });
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
                //else if(imageSource.DpiX != 96 || imageSource.DpiY != 96)
                //{
                //    var messageService = this.GetDependencyResolver().Resolve<IMessageService>();
                //    messageService.ShowWarningAsync($"图片 Dpi 必须为 96。");
                //}
                else
                    TileImageSource = imageSource;
            }
        }

        private void OnHasExtraImageChanged()
        {
            if (!HasExtraImage)
                ExtraImage = null;
        }
    }
}
