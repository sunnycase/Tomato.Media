using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using Catel.MVVM;
using Tomato.TileSetEditor.Models;

namespace Tomato.TileSetEditor.ViewModels
{
    class SelectExtraImageViewModel : ViewModelBase
    {
        public IEnumerable<ExtraImageModel> ExtraImagesSource { get; private set; }

        public ExtraImageModel SelectedExtraImage { get; set; }

        public SelectExtraImageViewModel(IEnumerable<ExtraImageModel> extraImagesSource)
        {
            ExtraImagesSource = extraImagesSource;
        }
    }
}
