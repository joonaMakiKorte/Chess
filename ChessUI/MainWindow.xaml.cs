using System;
using System.Collections.Generic;
using System.Linq;
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
            
            // Init UI
            boardUI = new BoardUI(PieceGrid, TopTimerLabel, BottomTimerLabel,
                images, timer, !bottomIsWhite, MoveLogView);

            // Init chess logic
            chessGame = new ChessGame(whiteIsHuman, blackIsHuman, bottomIsWhite, aiDifficulty, boardUI, this);

            // Update status from chessGame to ui
            boardUI.UpdateBoard(chessGame.GetBoardState());

            // Init UI interactions
            boardInteract = new BoardInteract(PieceGrid, chessGame, boardUI, !bottomIsWhite);

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
    }
}
