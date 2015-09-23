using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.Data;

namespace PinkAlert.Models
{
    class MenuButtonModel : ModelBase
    {
        public string Text { get; set; }

        public Type NavigationType { get; set; }

        public bool IsEnabled { get; set; }
    }
}
