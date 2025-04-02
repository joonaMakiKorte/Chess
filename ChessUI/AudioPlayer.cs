using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.IO;

namespace Chess
{
    class AudioPlayer
    {
        private MediaPlayer _mediaPlayer;

        public void PlayBackgroundMusic()
        {
            // Create a new MediaPlayer instance for the move sound
            var backSoundPlayer = new MediaPlayer();

            // Get the base directory of the application
            string baseDirectory = AppDomain.CurrentDomain.BaseDirectory;

            // Construct the full path to the music file
            string musicFilePath = Path.Combine(baseDirectory, "pics", "Lobbymusic.mp3");

            var uri = new Uri(musicFilePath);
            backSoundPlayer.Open(uri);
            // Play the audio file
            backSoundPlayer.Play();

            // Optionally, set the volume
            backSoundPlayer.Volume = 0.05; // 5% volume
        }

        public void StopMusic()
        {
            if (_mediaPlayer != null)
            {
                _mediaPlayer.Stop();
                _mediaPlayer.Close();
            }
        }

        public void PlayMoveSound()
        {

            // Stop any existing music before playing new music
            StopMusic();

            // Create a new MediaPlayer instance for the move sound
            var moveSoundPlayer = new MediaPlayer();

            // Get the base directory of the application
            string baseDirectory = AppDomain.CurrentDomain.BaseDirectory;

            // Construct the full path to the music file
            string musicFilePath = Path.Combine(baseDirectory, "pics", "piecemove.mp3");

            // Debug: Print the constructed path
            //Console.WriteLine("Constructed music file path: " + musicFilePath);

            // check if file exists
            if (File.Exists(musicFilePath))
            {
                // Create a URI for the full path
                var uri = new Uri(musicFilePath);
                moveSoundPlayer.Open(uri);

                // play the sound
                moveSoundPlayer.Play();

            }

            else
            {
                Console.WriteLine("Music not found");
            }
            
        }
    }
}
