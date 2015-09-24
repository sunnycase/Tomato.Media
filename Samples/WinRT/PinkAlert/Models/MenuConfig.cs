using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PinkAlert.Models
{
    /// <summary>
    /// 菜单设置
    /// </summary>
    class MenuConfig
    {
        /// <summary>
        /// 顶部的按钮
        /// </summary>
        public List<MenuButtonModel> TopButtons { get; } = new List<MenuButtonModel>();

        /// <summary>
        /// 底部的按钮
        /// </summary>
        public List<MenuButtonModel> BottomButtons { get; } = new List<MenuButtonModel>();
    }
}
