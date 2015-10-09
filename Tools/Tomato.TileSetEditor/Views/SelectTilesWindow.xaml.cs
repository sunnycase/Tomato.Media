using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using Tomato.TileSetEditor.Models;
using Tomato.TileSetEditor.ViewModels;

namespace Tomato.TileSetEditor.Views
{
    /// <summary>
    /// SelectTilesWindow.xaml 的交互逻辑
    /// </summary>
    public partial class SelectTilesWindow : Catel.Windows.DataWindow
    {
        new SelectTilesViewModel ViewModel
        {
            get { return base.ViewModel as SelectTilesViewModel; }
        }

        public SelectTilesWindow()
            : base(Catel.Windows.DataWindowMode.OkCancel)
        {
            InitializeComponent();
        }

        private void tiles_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var selectedTiles = ViewModel?.SelectedTiles;
            if (selectedTiles != null)
            {
                foreach (TileModel item in e.RemovedItems)
                    selectedTiles.Remove(item);
                foreach (TileModel item in e.AddedItems)
                    selectedTiles.Add(item);
            }
        }
    }
}
