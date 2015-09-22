using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using PinkAlert.ViewModels;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Data;

namespace PinkAlert.Controls
{
    class MenuButtonListBox : ListBox
    {
        protected override void PrepareContainerForItemOverride(DependencyObject element, object item)
        {
            base.PrepareContainerForItemOverride(element, item);

            BindingOperations.SetBinding(element, ListBoxItem.IsEnabledProperty, new Binding
            {
                Source = item,
                Path = new PropertyPath("IsEnabled"),
                Mode = BindingMode.OneWay
            });
            var viewModel = (MenuButtonViewModel)item;
            viewModel.ButtonItem = (ListBoxItem)element;
        }
    }
}
