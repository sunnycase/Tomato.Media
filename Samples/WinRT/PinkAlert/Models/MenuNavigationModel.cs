using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Xaml;

namespace PinkAlert.Models
{
    /// <summary>
    /// 菜单导航视图模型
    /// </summary>
    class MenuNavigationModel : DependencyObject
    {
        /// <summary>
        /// 视图类型
        /// </summary>
        public Type ViewType { get; set; }

        /// <summary>
        /// 菜单设置
        /// </summary>
        public MenuConfig MenuConfig { get; set; }
    }
}
