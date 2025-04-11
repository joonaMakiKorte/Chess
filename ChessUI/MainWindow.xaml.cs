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
    /// <summary>
    /// Main application window, hosts UI for chess engine DLL
    /// </summary>
    public partial class MainWindow : Window
    {
        private ChessGame chessGame;
        private Images images = new Images();
        private BoardUI boardUI;
        private BoardInteract boardInteract;
        private object pieceGrid;

        public MainWindow(bool whiteIsHuman, bool blackIsHuman, bool bottomIsWhite, string aiDifficulty, int timer)
        {
            InitializeComponent();

            // Init UI
            boardUI = new BoardUI(PieceGrid, TurnLabel, WhiteTimerLabel, BlackTimerLabel,
                images, timer, !bottomIsWhite, MoveLogView);

            // Init chess logic
            chessGame = new ChessGame(whiteIsHuman, blackIsHuman, bottomIsWhite, aiDifficulty, boardUI);

            // Update status from chessGame to ui
            boardUI.UpdateBoard(chessGame.GetBoardState());
            boardUI.UpdateTurnDisplay(chessGame.IsWhiteTURN());

            // Init UI interactions
            boardInteract = new BoardInteract(PieceGrid, chessGame, boardUI);
        }
    }
}
