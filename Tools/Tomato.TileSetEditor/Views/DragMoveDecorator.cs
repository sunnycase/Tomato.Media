using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace Tomato.TileSetEditor.Views
{
    class DragMoveDecorator : Decorator
    {
        public readonly static DependencyProperty OffsetProperty = DependencyProperty.Register("Offset", typeof(Point), typeof(DragMoveDecorator),
            new PropertyMetadata(OnOffsetChanged));

        public Point Offset
        {
            get { return (Point)GetValue(OffsetProperty); }
            set { SetValue(OffsetProperty, value); }
        }

        private static void OnOffsetChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var dec = d as DragMoveDecorator;
            dec?.UpdateOffset();
        }

        private void UpdateOffset()
        {
            var offset = new Point(Math.Max(0, Offset.X), Math.Max(0, Offset.Y));
            Canvas.SetLeft(this, offset.X);
            Canvas.SetTop(this, offset.Y);
            Offset = offset;
        }

        private bool _dragInProgress = false;
        private Point _lastMousePosition;
        protected override void OnPreviewMouseDown(MouseButtonEventArgs e)
        {
            base.OnPreviewMouseDown(e);
            _dragInProgress = true;
            _lastMousePosition = e.GetPosition(this);
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);
            if(_dragInProgress)
            {
                if(e.LeftButton != MouseButtonState.Pressed)
                    _dragInProgress = false;
                else
                {
                    var newPosition = e.GetPosition(this);
                    var offset = newPosition - _lastMousePosition;
                    var oldOffset = Offset;
                    Offset = oldOffset + offset;
                    offset = Offset - oldOffset;
                    _lastMousePosition = newPosition - offset;
                }
            }
        }

        protected override void OnPreviewMouseUp(MouseButtonEventArgs e)
        {
            base.OnPreviewMouseUp(e);
            _dragInProgress = false;
        }
    }
}
