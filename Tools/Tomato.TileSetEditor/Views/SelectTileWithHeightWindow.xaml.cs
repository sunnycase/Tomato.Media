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
    public partial class SelectTileWithHeightWindow : Catel.Windows.DataWindow
    {
        new SelectTileWithHeightViewModel ViewModel
        {
            get { return base.ViewModel as SelectTileWithHeightViewModel; }
        }

        public SelectTileWithHeightWindow()
            : base(Catel.Windows.DataWindowMode.OkCancel)
        {
            InitializeComponent();
        }
    }
}
