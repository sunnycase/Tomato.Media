using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PinkAlert.Services
{
    interface IMainMenuViewService
    {
        void PlayAlterMeterPointerAnimation();

        void NavigateContent(Type navigateType);
        void ExitMenu();
        void NavigateScene(Type navigateType);
    }
}
