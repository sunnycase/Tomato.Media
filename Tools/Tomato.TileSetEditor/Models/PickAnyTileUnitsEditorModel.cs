using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.Data;
using Tomato.Tools.Common.Gaming;

namespace Tomato.TileSetEditor.Models
{
    class PickAnyTileUnitsEditorModel : ModelBase
    {
        private readonly TileSetModel _tileSet;
        private ObservableCollection<PickAnyTileUnitModel> _tileUnits;
        public IReadOnlyCollection<PickAnyTileUnitModel> TileUnits
        {
            get { return _tileUnits; }
        }

        public PickAnyTileUnitsEditorModel(TileSetModel tileSet)
        {
            _tileSet = tileSet;
            _tileUnits = LoadTileUnits();
            _tileUnits.CollectionChanged += _tileUnits_CollectionChanged;
            RaisePropertyChanged(nameof(TileUnits));
        }

        private void _tileUnits_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            switch (e.Action)
            {
                case System.Collections.Specialized.NotifyCollectionChangedAction.Add:
                    foreach (PickAnyTileUnitModel item in e.NewItems)
                        _tileSet.TileSet.PickAnyTileUnits.Add(item.TileUnit);
                    break;
                case System.Collections.Specialized.NotifyCollectionChangedAction.Remove:
                    foreach (PickAnyTileUnitModel item in e.OldItems)
                        _tileSet.TileSet.PickAnyTileUnits.Remove(item.TileUnit);
                    break;
                case System.Collections.Specialized.NotifyCollectionChangedAction.Reset:
                    _tileSet.TileSet.PickAnyTileUnits.Clear();
                    foreach (var item in _tileUnits)
                        _tileSet.TileSet.PickAnyTileUnits.Add(item.TileUnit);
                    break;
            }
        }

        public void AddTileUnit(string category)
        {
            _tileUnits.Add(new PickAnyTileUnitModel(new PickAnyTileUnit()
            {
                Category = category
            }));
        }

        private ObservableCollection<PickAnyTileUnitModel> LoadTileUnits()
        {
            return new ObservableCollection<PickAnyTileUnitModel>(from tu in _tileSet.TileSet.PickAnyTileUnits
                                                                  select new PickAnyTileUnitModel(tu));
        }
    }
}
