using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;
using Catel.Data;
using Catel.IoC;
using Catel.MVVM;
using Catel.Services;
using Tomato.TileSetEditor.Models;

namespace Tomato.TileSetEditor.ViewModels
{
    class CreateExtraImageViewModel : ViewModelBase
    {
        [Model]
        public CreateExtraImageModel Model { get; private set; }

        [ViewModelToModel("Model")]
        public BitmapImage ExtraImageSource { get; set; }

        [ValidationToViewModel]
        public IValidationSummary ValidationSummary { get; private set; }

        public Command SelectExtraImageSourceCommand { get; private set; }

        public CreateExtraImageViewModel(CreateExtraImageModel model)
        {
            Model = model;
            SelectExtraImageSourceCommand = new Command(OnSelectExtraImageSourceCommand);
        }

        private void OnSelectExtraImageSourceCommand()
        {
            var openFileService = this.GetDependencyResolver().Resolve<IOpenFileService>();
            openFileService.Filter = Constants.SupportedImageSourceFilter;
            openFileService.CheckFileExists = true;
            if (openFileService.DetermineFile())
            {
                var imageSource = new BitmapImage(new Uri(openFileService.FileName)) { CacheOption = BitmapCacheOption.OnLoad };
                //if (imageSource.DpiX != 96 || imageSource.DpiY != 96)
                //{
                //    var messageService = this.GetDependencyResolver().Resolve<IMessageService>();
                //    messageService.ShowWarningAsync($"图片 Dpi 必须为 96。");
                //}
                //else
                    ExtraImageSource = imageSource;
            }
        }
    }
}
