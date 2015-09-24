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

        void Navigate(Type navigateType);
    }
}
