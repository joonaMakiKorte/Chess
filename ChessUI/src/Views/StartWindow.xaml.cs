using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Converters;

namespace Chess
{
    /// <summary>
    /// Startup window for game config
    /// </summary>
    public partial class StartWindow : Window
    {
        public StartWindow()
        {
            InitializeComponent();

            // Apply size
            this.Top = Properties.Settings.Default.WindowTop;
            this.Left = Properties.Settings.Default.WindowLeft;
            this.Width = Properties.Settings.Default.WindowWidth;
            this.Height = Properties.Settings.Default.WindowHeight;

            this.Loaded += StartWindow_Loaded; // Keep Loaded event for initialization
        }

        private void StartWindow_Loaded(object sender, RoutedEventArgs e)
        {
            // Set a default timer value (e.g., 10 minutes) after UI is ready
            // The XAML binding should update the text block automatically.
            TimerSlider.Value = 10;
            // Manually update text block just in case binding hasn't fired yet or fails
            TimerValueTextBlock.Text = $"{TimerSlider.Value:F0} min";

            // Ensure initial visibility state is correct
            UpdateAIControlsVisibility();
        }

        // --- Event Handler for Radio Buttons ---
        private void PlayerType_Changed(object sender, RoutedEventArgs e)
        {
            // Prevent execution during swap or initial load
            if (LeftHumanRadio == null || LeftEngineRadio == null ||
               RightHumanRadio == null || RightEngineRadio == null ||
               LeftAIDifficultyComboBox == null || RightAIDifficultyComboBox == null ||
               LeftAIDifficultyLabel == null || RightAIDifficultyLabel == null)
            {
                return; // Controls not ready yet
            }

            // Update visibility based on current selections
            UpdateAIControlsVisibility();

            // --- Enforce "No AI vs AI" Rule ---
            if (LeftEngineRadio.IsChecked == true && RightEngineRadio.IsChecked == true)
            {
                // Determine which radio button caused the conflict and revert the *other* one.
                if (sender == LeftEngineRadio)
                {
                    RightHumanRadio.IsChecked = true; // Revert Black to Human
                }
                else if (sender == RightEngineRadio)
                {
                    LeftHumanRadio.IsChecked = true; // Revert White to Human
                }
                else
                {
                    // Fallback if sender isn't identifiable (shouldn't normally happen)
                    RightHumanRadio.IsChecked = true; // Revert Black as a default action
                }
                // Update visibilities again after correction
                UpdateAIControlsVisibility();
            }
        }

        // --- Helper to manage AI control visibility ---
        private void UpdateAIControlsVisibility()
        {
            if (LeftEngineRadio == null || LeftAIDifficultyLabel == null || LeftAIDifficultyComboBox == null ||
                 RightEngineRadio == null || RightAIDifficultyLabel == null || RightAIDifficultyComboBox == null)
            {
                return; // Controls not loaded
            }

            // Left Side AI Controls
            Visibility leftVisibility = LeftEngineRadio.IsChecked == true ? Visibility.Visible : Visibility.Hidden;
            LeftAIDifficultyLabel.Visibility = leftVisibility;
            LeftAIDifficultyComboBox.Visibility = leftVisibility;

            // Right Side AI Controls
            Visibility rightVisibility = RightEngineRadio.IsChecked == true ? Visibility.Visible : Visibility.Hidden;
            RightAIDifficultyLabel.Visibility = rightVisibility;
            RightAIDifficultyComboBox.Visibility = rightVisibility;
        }


        // --- Updated Timer Slider Handler ---
        private void TimerSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (TimerValueTextBlock != null) // Check if UI element exists
            {
                // Use StringFormat to show whole number and " min"
                TimerValueTextBlock.Text = $"{e.NewValue:F0} min";
            }
        }

        private void SwitchSides_Click(object sender, RoutedEventArgs e)
        {
            // Ensure labels exist
            if (LeftHeaderLabel == null || RightHeaderLabel == null) return;

            // Simple swap of the header text
            object leftHeaderText = LeftHeaderLabel.Text; // Use Text property
            LeftHeaderLabel.Text = RightHeaderLabel.Text;
            RightHeaderLabel.Text = (string)leftHeaderText; // Cast back to string

            // Note: The actual controls (radio buttons, combo boxes) underneath
            // do NOT swap their states. Only the labels change.
        }

        // --- Start Game Handler ---
        private void StartGame_Click(object sender, RoutedEventArgs e)
        {
            // Determine which side is currently labelled "White"
            bool leftIsWhite = LeftHeaderLabel.Text == "White";

            // Get settings based on which side is currently White/Black
            bool whiteIsHuman, blackIsHuman;
            string difficulty = null;

            if (leftIsWhite)
            {
                // Left side is White, Right side is Black
                whiteIsHuman = LeftHumanRadio.IsChecked == true;
                blackIsHuman = RightHumanRadio.IsChecked == true;
                if (!whiteIsHuman) difficulty = (LeftAIDifficultyComboBox.SelectedItem as ComboBoxItem)?.Content.ToString() ?? "Medium";
                if (!blackIsHuman) difficulty = (RightAIDifficultyComboBox.SelectedItem as ComboBoxItem)?.Content.ToString() ?? "Medium";
            }
            else
            {
                // Right side is White, Left side is Black
                whiteIsHuman = RightHumanRadio.IsChecked == true;
                blackIsHuman = LeftHumanRadio.IsChecked == true;
                if (!whiteIsHuman) difficulty = (RightAIDifficultyComboBox.SelectedItem as ComboBoxItem)?.Content.ToString() ?? "Medium";
                if (!blackIsHuman) difficulty = (LeftAIDifficultyComboBox.SelectedItem as ComboBoxItem)?.Content.ToString() ?? "Medium";
            }


            int timerMinutes = (int)TimerSlider.Value;

            // --- Final Check: Prevent Engine vs Engine ---
            // (Should already be prevented by PlayerType_Changed, but good safeguard)
            if (!whiteIsHuman && !blackIsHuman)
            {
                MessageBox.Show("Engine vs Engine is not allowed. Please set at least one player to Human.", "Configuration Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return; // Stop game start
            }

            try
            {
                // Save window position & size to settings
                Properties.Settings.Default.WindowTop = this.Top;
                Properties.Settings.Default.WindowLeft = this.Left;
                Properties.Settings.Default.WindowWidth = this.Width;
                Properties.Settings.Default.WindowHeight = this.Height;
                Properties.Settings.Default.Save();

                // Pass AI flags, playing sides, difficulty and timer to MainWindow
                MainWindow mainWindow = new MainWindow(
                    whiteIsHuman, blackIsHuman, leftIsWhite, difficulty, timerMinutes
                );
                mainWindow.Show();
                this.Close(); // Close the startup window
            }
            catch (Exception ex)
            {
                // Handle potential errors during MainWindow creation or showing
                MessageBox.Show($"Error starting game: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }

        }

        private void Exit_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }
    }
}