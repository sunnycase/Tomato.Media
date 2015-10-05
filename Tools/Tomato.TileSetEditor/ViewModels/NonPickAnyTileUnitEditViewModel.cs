using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.MVVM;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.ViewModels
{
    [InterestedIn(typeof(MainViewModel))]
    class NonPickAnyTileUnitEditViewModel : ViewModelBase
    {
        [Model]
        public TileUnit TileUnit { get; private set; }

        public TileSetContext TileSetContext { get; private set; }

        public NonPickAnyTileUnitEditViewModel(TileUnit tileUnit)
        {
            TileUnit = tileUnit;
        }

        protected override void OnViewModelPropertyChanged(IViewModel viewModel, string propertyName)
        {
            var mainViewModel = viewModel as MainViewModel;
            if(mainViewModel != null && propertyName == nameof(MainViewModel.TileSet))
            {
                TileSetContext = mainViewModel.TileSet;
            }
        }

        private void OnTileSetContextChanged()
        {

        }
    }
}
