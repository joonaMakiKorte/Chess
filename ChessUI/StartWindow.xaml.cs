using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.IO;


namespace Chess
{
    public partial class StartWindow : Window
    {


        private AudioPlayer _mediaPlayer;
        private readonly int[] allowedTimes = { 1, 3, 5, 10, 15, 30, 60 };
        public StartWindow()
        {
            InitializeComponent();

            _mediaPlayer = new AudioPlayer();
            this.Loaded += StartWindow_Loaded; // Ensure UI is fully loaded before modifying the slider



            
            _mediaPlayer.PlayBackgroundMusic();

        }



        private void StartWindow_Loaded(object sender, RoutedEventArgs e)
        {
            TimerSlider.Value = allowedTimes[0]; // Set default value after UI is ready
            TimerValueTextBlock.Text = allowedTimes[0].ToString();
        }


        
        private void TimerSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (TimerValueTextBlock == null) return; // Prevents the NullReferenceException

            double newValue = e.NewValue;
            int closest = allowedTimes.OrderBy(v => Math.Abs(v - newValue)).First();
            TimerSlider.Value = closest; // Snap to the closest value
            TimerValueTextBlock.Text = closest.ToString();
        }


        // function for the game mode clicker boxes, also shows the ai difficulty if chosen
        private void GameMode_Click(object sender, RoutedEventArgs e)
        {

            if (sender == ButtonAI)
            {
                TimerSlider.Visibility = Visibility.Collapsed;
                TimerValueTextBlock.Visibility = Visibility.Collapsed;

                // Show AI Difficulty Selection
                AIDifficultyComboBox.Visibility = Visibility.Visible;

                // Change text to "Select Difficulty:"
                SelectionTextBlock.Text = "Select Difficulty:";

                // update disabled and abled button
                Button1v1.IsEnabled = false;
                ButtonAI.IsEnabled = true;
            }
            else if (sender == Button1v1)
            {
                TimerSlider.Visibility = Visibility.Visible;
                TimerValueTextBlock.Visibility = Visibility.Visible;

                // Hide AI Difficulty Selection
                AIDifficultyComboBox.Visibility = Visibility.Collapsed;

                // Change text back to "Select Timer (minutes):"
                SelectionTextBlock.Text = "Select Timer (minutes):";

                // Disable the AI button and enable the 1v1 button
                Button1v1.IsEnabled = true;
                ButtonAI.IsEnabled = false;
            }

        }

        private void StartGame_Click(object sender, RoutedEventArgs e)
        {
            string gameMode = "1v1";

            if (Button1v1.IsEnabled && !ButtonAI.IsEnabled)
            {
                gameMode = "1v1";
            }

            else if (!Button1v1.IsEnabled && ButtonAI.IsEnabled)
            {
                gameMode = "AI";
            }

            string aiDifficulty = (AIDifficultyComboBox.SelectedItem as ComboBoxItem)?.Content.ToString();
            string timer = TimerSlider.Value.ToString(); // Get the timer value

            // Pass these settings to the MainWindow or game logic
            MainWindow mainWindow = new MainWindow(gameMode, aiDifficulty, timer);
            mainWindow.Show();
            this.Close();
        }

        

        private void Exit_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }
    }
}

