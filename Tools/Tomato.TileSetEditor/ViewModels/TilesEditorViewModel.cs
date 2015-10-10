using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.Fody;
using Catel.IoC;
using Catel.MVVM;
using Catel.Services;
using Tomato.TileSetEditor.Models;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.ViewModels
{
    class TilesEditorViewModel : ViewModelBase
    {
        [Model]
        public TilesEditorModel Model { get; private set; }

        [ViewModelToModel("Model")]
        public IReadOnlyCollection<TileModel> Tiles { get; private set; }

        public Command AddTileCommand { get; private set; }

        public TilesEditorViewModel([NotNull]TilesEditorModel model)
        {
            Model = model;
            AddTileCommand = new Command(OnAddTileCommand);
        }

        private void OnAddTileCommand()
        {
            var model = new CreateTileModel(Model.TileWidth, Model.TileHeight);
            var uiVisualizerService = this.GetDependencyResolver().Resolve<IUIVisualizerService>();
            uiVisualizerService.ShowDialog<CreateTileViewModel>(model, (s, e) =>
            {
                if (e.Result == true)
                    Model.AddTile(model.TileImageSource, model.ExtraImage, model.ExtraImageOffset);
            });
        }
    }
}
