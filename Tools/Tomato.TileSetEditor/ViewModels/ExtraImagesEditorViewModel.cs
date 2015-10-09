using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.IoC;
using Catel.MVVM;
using Catel.Services;
using Tomato.TileSetEditor.Models;

namespace Tomato.TileSetEditor.ViewModels
{
    class ExtraImagesEditorViewModel : ViewModelBase
    {
        [Model]
        public ExtraImagesEditorModel Model { get; private set; }

        [ViewModelToModel("Model")]
        public IReadOnlyCollection<ExtraImageModel> ExtraImages { get; private set; }

        public Command AddExtraImageCommand { get; }

        public ExtraImagesEditorViewModel(ExtraImagesEditorModel model)
        {
            Model = model;
            AddExtraImageCommand = new Command(OnAddExtraImageCommand);
        }

        private void OnAddExtraImageCommand()
        {
            var model = new CreateExtraImageModel();
            var uiVisualizerService = this.GetDependencyResolver().Resolve<IUIVisualizerService>();
            uiVisualizerService.ShowDialog<CreateExtraImageViewModel>(model, (s, e) =>
            {
                if (e.Result == true)
                    Model.AddExtraImage(model.ExtraImageSource);
            });
        }
    }
}
