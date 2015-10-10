using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace Tomato.TileSetEditor.Views
{
    class AutoSizeCanvas : Canvas
    {
        protected override Size MeasureOverride(Size constraint)
        {
            base.MeasureOverride(constraint);
            if (!base.InternalChildren.OfType<UIElement>().Any())
                return new System.Windows.Size(1, 1);

            double width = base
                .InternalChildren
                .OfType<UIElement>()
                .Max(i => i.DesiredSize.Width + SafeDouble(Canvas.GetLeft(i)));

            double height = base
                .InternalChildren
                .OfType<UIElement>()
                .Max(i => i.DesiredSize.Height + SafeDouble(Canvas.GetTop(i)));

            return new Size(width, height);
        }

        private static double SafeDouble(double value)
        {
            if (double.IsNaN(value))
                return 0;
            return value;
        }
    }
}
