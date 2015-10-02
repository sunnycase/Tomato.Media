using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Tomato.Media;
using Windows.Media.MediaProperties;
using Windows.Storage;
using Windows.Storage.Streams;

namespace PinkAlert.Services
{
    class SoundService : ISoundService
    {
        private Dictionary<string, XAudioSound[]> _sounds;
        private XAudioSession audioSession;
        private XAudioChannel audioChannel;
        private Random random = new Random();

        public SoundService()
        {
            audioSession = new XAudioSession();
            audioChannel = audioSession.AddChannel(AudioEncodingProperties.CreatePcm(22050, 2, 16));
            LoadSounds();
        }

        private async void LoadSounds()
        {
            var soundFile = await FileIO.ReadTextAsync(await StorageFile.GetFileFromApplicationUriAsync(new Uri("ms-appx:///Assets/Config/sounds.json")));
            var soundsLocations = JsonConvert.DeserializeObject<Dictionary<string, string[]>>(soundFile);
            var readTasks = from s in soundsLocations
                            let fs = from f in s.Value
                                     select LoadSoundFileAsync(f)
                            select new
                            {
                                Name = s.Key,
                                Files = Task.WhenAll(fs)
                            };
            _sounds = new Dictionary<string, XAudioSound[]>();
            foreach (var task in readTasks)
            {
                var files = from f in await task.Files
                            select new XAudioSound(f);
                _sounds.Add(task.Name, files.ToArray());
            }
        }

        private async Task<byte[]> LoadSoundFileAsync(string path)
        {
            var buffer = await FileIO.ReadBufferAsync(await StorageFile.GetFileFromApplicationUriAsync(
                new Uri("ms-appx:///" + path)));
            using (var dataReader = DataReader.FromBuffer(buffer))
            {
                var bytes = new byte[buffer.Length];
                dataReader.ReadBytes(bytes);
                return bytes;
            }
        }

        public void PlaySound(string name)
        {
            XAudioSound[] sounds;
            if(_sounds.TryGetValue(name, out sounds))
            {
                var sound = sounds[random.Next(sounds.Length)];
                audioChannel.Play(sound);
            }
        }
    }
}
