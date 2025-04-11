using System;
using System.Globalization; // Required for Converter
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;     // Required for Converter
// Add other necessary using statements like System.IO if AudioPlayer needs it

namespace Chess
{
    public partial class StartWindow : Window
    {
        private bool _isSwappingSides = false;

        public StartWindow()
        {
            InitializeComponent();
            this.Loaded += StartWindow_Loaded; // Keep Loaded event for initialization
        }

        private void StartWindow_Loaded(object sender, RoutedEventArgs e)
        {
            // Set a default timer value (e.g., 10 minutes) after UI is ready
            // The XAML binding should update the text block automatically.
            TimerSlider.Value = 10;
            // Manually update text block just in case binding hasn't fired yet or fails
            TimerValueTextBlock.Text = $"{TimerSlider.Value:F0} min";

            // Ensure initial visibility state is correct (though binding should handle this)
            UpdateAIControlsVisibility();
        }

        // --- New Event Handler for Radio Buttons ---
        private void PlayerType_Changed(object sender, RoutedEventArgs e)
        {
            // Prevent execution during swap or initial load
            if (_isSwappingSides ||
                WhiteHumanRadio == null || WhiteEngineRadio == null ||
                BlackHumanRadio == null || BlackEngineRadio == null ||
                WhiteAIDifficultyComboBox == null || BlackAIDifficultyComboBox == null)
            {
                return;
            }

            // Update visibility based on current selections (mostly handled by binding now)
            UpdateAIControlsVisibility();

            // --- Enforce "No AI vs AI" Rule ---
            if (WhiteEngineRadio.IsChecked == true && BlackEngineRadio.IsChecked == true)
            {
                MessageBox.Show("Sorry, Engine vs Engine games are not supported.", "Invalid Selection", MessageBoxButton.OK, MessageBoxImage.Warning);

                // Determine which radio button caused the conflict and revert the *other* one.
                if (sender == WhiteEngineRadio)
                {
                    BlackHumanRadio.IsChecked = true; // Revert Black to Human
                }
                else if (sender == BlackEngineRadio)
                {
                    WhiteHumanRadio.IsChecked = true; // Revert White to Human
                }
                else
                {
                    // Fallback if sender isn't identifiable (shouldn't normally happen)
                    BlackHumanRadio.IsChecked = true; // Revert Black as a default action
                }
                // Update visibilities again after correction (mainly if not using binding)
                UpdateAIControlsVisibility();
            }
        }

        // --- Helper to manage AI control visibility (optional if binding works reliably) ---
        private void UpdateAIControlsVisibility()
        {
            var whiteLabel = (TextBlock)this.FindName("WhiteAIDifficultyLabel"); // Assuming you add x:Name="WhiteAIDifficultyLabel" to the TextBlock
            var blackLabel = (TextBlock)this.FindName("BlackAIDifficultyLabel"); // Assuming you add x:Name="BlackAIDifficultyLabel"

            Visibility whiteVisibility = WhiteEngineRadio.IsChecked == true ? Visibility.Visible : Visibility.Collapsed;
            WhiteAIDifficultyComboBox.Visibility = whiteVisibility;
            if (whiteLabel != null) whiteLabel.Visibility = whiteVisibility;


            Visibility blackVisibility = BlackEngineRadio.IsChecked == true ? Visibility.Visible : Visibility.Collapsed;
            BlackAIDifficultyComboBox.Visibility = blackVisibility;
             if (blackLabel != null) blackLabel.Visibility = blackVisibility;
        }


        // --- Updated Timer Slider Handler ---
        private void TimerSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            // XAML binding now updates the text block.
            // This handler can be kept empty, or used for additional logic if needed.
            // If you want C# to explicitly update the text:
            if (TimerValueTextBlock != null) // Check if UI element exists
            {
                // Use StringFormat to show whole number and " min"
                TimerValueTextBlock.Text = $"{e.NewValue:F0} min";
            }

            // The old snapping logic using 'allowedTimes' is removed.
            // The slider now uses IsSnapToTickEnabled="True" and TickFrequency="1" in XAML
            // for snapping to whole minute values between Minimum and Maximum.
        }


        // --- GameMode_Click is Obsolete ---
        // Remove the entire GameMode_Click method.


        // --- Updated Start Game Handler ---
        private void StartGame_Click(object sender, RoutedEventArgs e)
        {
            // --- Gather Settings from New Controls ---
            bool isWhiteHuman = WhiteHumanRadio.IsChecked == true;
            bool isBlackHuman = BlackHumanRadio.IsChecked == true;

            string whiteName = WhitePlayerName.Text.Trim();
            string blackName = BlackPlayerName.Text.Trim();

            // Provide default names if empty
            if (string.IsNullOrEmpty(whiteName)) whiteName = "White";
            if (string.IsNullOrEmpty(blackName)) blackName = "Black";

            // Get AI difficulty only if the player is an engine
            string whiteDifficulty = null;
            if (!isWhiteHuman)
            {
                whiteDifficulty = (WhiteAIDifficultyComboBox.SelectedItem as ComboBoxItem)?.Content.ToString();
                if (string.IsNullOrEmpty(whiteDifficulty)) whiteDifficulty = "Medium"; // Default if somehow null
            }

            string blackDifficulty = null;
            if (!isBlackHuman)
            {
                blackDifficulty = (BlackAIDifficultyComboBox.SelectedItem as ComboBoxItem)?.Content.ToString();
                if (string.IsNullOrEmpty(blackDifficulty)) blackDifficulty = "Medium"; // Default if somehow null
            }


            int timerMinutes = (int)TimerSlider.Value;

            // --- Final Check: Prevent Engine vs Engine ---
            // (Should already be prevented by PlayerType_Changed, but good safeguard)
            if (!isWhiteHuman && !isBlackHuman)
            {
                MessageBox.Show("Engine vs Engine is not allowed. Please set at least one player to Human.", "Configuration Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return; // Stop game start
            }

            // --- TODO: Update MainWindow constructor call ---
            // You MUST adapt the constructor of your MainWindow class
            // to accept these new parameters. The old constructor:
            // MainWindow(gameMode, aiDifficulty, timer) is no longer suitable.

            // Example of how you might need to change the MainWindow call:
            // Assuming MainWindow constructor is now like:
            // public MainWindow(bool whiteIsHuman, string whitePlayerName, string whiteAiLevel,
            //                   bool blackIsHuman, string blackPlayerName, string blackAiLevel,
            //                   int gameTimerMinutes)
            try
            {
                // isWhiteHuman, whiteName, whiteDifficulty,isBlackHuman, blackName, blackDifficulty,timerMinutes
                MainWindow mainWindow = new MainWindow(
                    "AI", blackDifficulty, "10"
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

        private void SwitchSides_Click(object sender, RoutedEventArgs e)
        {
            if (_isSwappingSides) return; // Prevent re-entrancy if something goes wrong

            _isSwappingSides = true; // Set flag

            try
            {
                // --- Read Current States ---
                bool whiteWasHuman = WhiteHumanRadio.IsChecked ?? false;
                string whiteName = WhitePlayerName.Text;
                int whiteDiffIndex = WhiteAIDifficultyComboBox.SelectedIndex;

                bool blackWasHuman = BlackHumanRadio.IsChecked ?? false;
                string blackName = BlackPlayerName.Text;
                int blackDiffIndex = BlackAIDifficultyComboBox.SelectedIndex;

                // --- Apply White's State to Black ---
                if (whiteWasHuman) BlackHumanRadio.IsChecked = true; else BlackEngineRadio.IsChecked = true;
                BlackPlayerName.Text = whiteName;
                // Only set index if it was valid for White (i.e., White was AI)
                if (!whiteWasHuman) BlackAIDifficultyComboBox.SelectedIndex = whiteDiffIndex;
                else BlackAIDifficultyComboBox.SelectedIndex = -1; // Or set to default if needed

                // --- Apply Black's State to White ---
                if (blackWasHuman) WhiteHumanRadio.IsChecked = true; else WhiteEngineRadio.IsChecked = true;
                WhitePlayerName.Text = blackName;
                // Only set index if it was valid for Black (i.e., Black was AI)
                if (!blackWasHuman) WhiteAIDifficultyComboBox.SelectedIndex = blackDiffIndex;
                else WhiteAIDifficultyComboBox.SelectedIndex = -1; // Or set to default if needed

                // --- Trigger UI Updates (Optional but recommended) ---
                // Although setting IsChecked should trigger PlayerType_Changed/bindings,
                // calling UpdateAIControlsVisibility ensures the combo boxes show/hide correctly.
                UpdateAIControlsVisibility();
            }
            finally
            {
                _isSwappingSides = false; // Reset flag
            }
        }
    }
}