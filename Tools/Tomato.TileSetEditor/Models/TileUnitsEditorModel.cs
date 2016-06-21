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
    class TileUnitsEditorModel : ModelBase
    {
        private readonly TileSetModel _tileSet;
        private ObservableCollection<TileUnitModel> _tileUnits;
        public IReadOnlyCollection<TileUnitModel> TileUnits
        {
            get { return _tileUnits; }
        }

        public TileUnitsEditorModel(TileSetModel tileSet)
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
                    foreach (TileUnitModel item in e.NewItems)
                        _tileSet.TileSet.TileUnits.Add(item.TileUnit);
                    break;
                case System.Collections.Specialized.NotifyCollectionChangedAction.Remove:
                    foreach (TileUnitModel item in e.OldItems)
                        _tileSet.TileSet.TileUnits.Remove(item.TileUnit);
                    break;
                case System.Collections.Specialized.NotifyCollectionChangedAction.Reset:
                    _tileSet.TileSet.PickAnyTileUnits.Clear();
                    foreach (var item in _tileUnits)
                        _tileSet.TileSet.TileUnits.Add(item.TileUnit);
                    break;
            }
        }

        public void AddTileUnit(string category, TileUnitSize size)
        {
            _tileUnits.Add(new TileUnitModel(new TileUnit(size)
            {
                Category = category
            }));
        }

        private ObservableCollection<TileUnitModel> LoadTileUnits()
        {
            return new ObservableCollection<TileUnitModel>(from tu in _tileSet.TileSet.TileUnits
                                                                  select new TileUnitModel(tu));
        }
    }
}
