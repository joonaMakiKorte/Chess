using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using static System.Net.Mime.MediaTypeNames;
using static Chess.ChessGame;



namespace Chess
{
    // Handle promotion events
    public interface IPromotionUI
    {
        char GetPromotionChoice(bool isWhite); // Returns chosen piece char ('q','r','n','b') or '-' for cancel
    }

    /// <summary>
    /// Main application window, hosts UI for chess engine DLL
    /// </summary>
    public partial class MainWindow : Window, IPromotionUI
    {
        private ChessGame chessGame;
        private Images images = new Images();
        private BoardUI boardUI;
        private BoardInteract boardInteract;

        public MainWindow(bool whiteIsHuman, bool blackIsHuman, bool bottomIsWhite, string aiDifficulty, int timer)
        {
            
            InitializeComponent();

            // Apply window size inherited from startup
            this.Top = Properties.Settings.Default.WindowTop;
            this.Left = Properties.Settings.Default.WindowLeft;
            this.Width = Properties.Settings.Default.WindowWidth;
            this.Height = Properties.Settings.Default.WindowHeight;

            // Init UI
            boardUI = new BoardUI(PieceGrid, TopTimerLabel, BottomTimerLabel,
                images, timer, !bottomIsWhite, MoveLogView);

            // Init chess logic
            chessGame = new ChessGame(whiteIsHuman, blackIsHuman, bottomIsWhite, aiDifficulty, boardUI, this); // Pass IPromotionUI
            chessGame.GameOver += ChessGame_GameOver; // Subscribe to GameOver-event

            // Update status from chessGame to ui
            boardUI.UpdateBoard(chessGame.GetBoardState());

            // Init UI interactions
            boardInteract = new BoardInteract(PieceGrid, ResignButton, NewGameButton, chessGame, boardUI, !bottomIsWhite);

            // Show resign button only if AI game
            if (!whiteIsHuman || !blackIsHuman) ResignButton.Visibility = Visibility.Visible;

            // Task to start game (needed in case white plays as ai)
            _ = boardInteract.StartGameASync();
        }

        // Activate promotion window to get promotion choice
        public char GetPromotionChoice(bool isWhite)
        {
            return Dispatcher.Invoke(() => {
                var promoDialog = new PromotionWindow(isWhite, this.images);
                promoDialog.Owner = this; // Owner is MainWindow
                boardUI.ToggleTimer(isWhite, false); // Stop active timer
                bool? dialogResult = promoDialog.ShowDialog();
                boardUI.ToggleTimer(isWhite, true); // Start active timer
                return (dialogResult == true) ? promoDialog.SelectedPromotionPiece : '-'; // Return '-' if cancelled
            });
        }

        // End game by resign
        private void ResignButton_Click(object sender, EventArgs e)
        {
            // Trigger GameOver with 'resign'
            chessGame.isOngoing = false;
            ChessGame_GameOver(this, new GameOverEventArgs("resign"));
        }

        // Triggered by GameOver-event
        private void ChessGame_GameOver(object sender, GameOverEventArgs e)
        {
            // Ensure running on UI thread
            Dispatcher.Invoke(() => {
                boardUI.ToggleTimer(chessGame.isWhiteTurn, false); // Stop active timer

                string message = GetGameOverMessage(e.State); // Format a user-friendly message
                GameOverTextBlock.Text = message;
                GameOverField.Visibility = Visibility.Visible; // Show the dedicated text block

                // Disable inputs
                PieceGrid.IsEnabled = false;
                ResignButton.Visibility = Visibility.Hidden;
            });
        }

        private string GetGameOverMessage(string state)
        {
            // Convert state string to user-friendly message
            switch (state)
            {
                case "mate": return chessGame.isWhiteTurn ? "0-1\nBlack wins!" : "1-0\nWhite wins!";
                case "stalemate": return "½-½\nStalemate!";
                case "draw_repetition": return "½-½\nDraw by repetition!";
                case "draw_50": return "½-½\nDraw by 50 move rule!";
                case "draw_insufficient": return "½-½\nInsufficient material!";
                case "resign": return chessGame.isWhiteAI ? "½-0\nBlack resigns!" : "0-½\nWhite resigns!";
                default: return $"Game Over: {state}"; // Should never reach here
            }
        }

        private void NewGameButton_Click(object sender, EventArgs e)
        {
            // If game has ended, launch start window right away
            if (!chessGame.isOngoing)
            {
                StartWindow startWindow = new StartWindow();
                startWindow.Show();
                this.Close();
            }
            // Else show confirmation buttons
            else
            {
                // Show confirmation question
                GameOverTextBlock.Text = "Are you sure?";
                GameOverField.Visibility = Visibility.Visible;

                // Disable inputs
                PieceGrid.IsEnabled = false;
                ResignButton.IsEnabled = false;

                NewGameButton.Visibility = Visibility.Collapsed;
                ConfirmationPanel.Visibility = Visibility.Visible;
            }
        }

        private void YesButton_Click(Object sender, EventArgs e)
        {
            // Launch start window
            StartWindow startWindow = new StartWindow();
            startWindow.Show();
            this.Close();
        }

        private void NoButton_Click(Object sender, EventArgs e)
        {
            // Hide confirmation question and enable inputs
            GameOverField.Visibility = Visibility.Collapsed;

            PieceGrid.IsEnabled = true;
            ResignButton.IsEnabled = true;

            NewGameButton.Visibility = Visibility.Visible;
            ConfirmationPanel.Visibility = Visibility.Collapsed;
        }

        protected override void OnClosed(EventArgs e)
        {
            chessGame.Dispose(); // Explicit cleanup

            // Save size to setting 
            Properties.Settings.Default.WindowTop = this.Top;
            Properties.Settings.Default.WindowLeft = this.Left;
            Properties.Settings.Default.WindowWidth = this.Width;
            Properties.Settings.Default.WindowHeight = this.Height;
            Properties.Settings.Default.Save();

            base.OnClosed(e);  
        }
    }
}
