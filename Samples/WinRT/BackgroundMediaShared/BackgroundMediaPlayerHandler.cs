using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Media;

namespace BackgroundMediaShared
{
    public sealed class BackgroundMediaPlayerHandler : IBackgroundMediaPlayerHandler
    {
        private BackgroundMediaPlayer mediaPlayer;

        public void OnActivated(BackgroundMediaPlayer mediaPlayer)
        {
            this.mediaPlayer = mediaPlayer;
        }
    }
}
