using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace Chess
{
    class AudioPlayer
    {
        private MediaPlayer _mediaPlayer;

        public void PlayBackgroundMusic(string filePath)
        {
            // Create a new MediaPlayer instance
            _mediaPlayer = new MediaPlayer();

            // Open the audio file
            var uri = new Uri(filePath);
            _mediaPlayer.Open(uri);

            // Play the audio file
            _mediaPlayer.Play();

            // Optionally, set the volume
            _mediaPlayer.Volume = 0.5; // 50% volume
        }

        public void StopMusic()
        {
            if (_mediaPlayer != null)
            {
                _mediaPlayer.Stop();
                _mediaPlayer.Close();
                _mediaPlayer = new MediaPlayer(); // Reset the player
            }
        }

        public void PlayMoveSound()
        {

            // Stop any existing music before playing new music
            StopMusic();
            // Create a new MediaPlayer instance for the move sound
            var moveSoundPlayer = new MediaPlayer();
            var uri = new Uri("C:\\chessproject\\Chess\\ChessUI\\pics\\piecemove.mp3");
            moveSoundPlayer.Open(uri);
            moveSoundPlayer.Play();
        }
    }
}
