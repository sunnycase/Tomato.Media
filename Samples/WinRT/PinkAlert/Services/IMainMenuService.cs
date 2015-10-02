using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PinkAlert.Services
{
    interface IMainMenuService
    {
        void NavigateToMenu(Uri uri);
        void NavigateScene(Uri uri);
    }
}
