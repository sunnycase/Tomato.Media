using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Catel.IoC;
using Tomato.TileSetEditor.Models;
using Catel.Services;
using Catel.Data;

namespace Tomato.TileSetEditor.ViewModels
{
    class CreateTileSetViewModel : ViewModelBase
    {
        [Model]
        public CreateTileSetModel Model { get; private set; }

        [ViewModelToModel("Model")]
        public string FileName { get; set; }

        [ViewModelToModel("Model")]
        public string Name { get; set; }

        [ViewModelToModel("Model")]
        public int TileWidth { get; set; }

        [ViewModelToModel("Model")]
        public int TileHeight { get; set; }

        [ValidationToViewModel]
        public IValidationSummary ValidationSummary { get; set; }

        public Command BrowseFileNameCommand { get; private set; }

        public CreateTileSetViewModel(CreateTileSetModel model)
        {
            Model = model;
            BrowseFileNameCommand = new Command(OnBrowseFileNameCommand);
        }

        private void OnBrowseFileNameCommand()
        {
            var saveFileService = this.GetDependencyResolver().Resolve<ISaveFileService>();
            saveFileService.Filter = Constants.TileSetFilter;
            if (saveFileService.DetermineFile())
                FileName = saveFileService.FileName;
        }
    }
}
