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
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    /// 
    public partial class MainWindow : Window
    {

        
        private ChessGame chessGame = new ChessGame();
        private Images images = new Images();
        private BoardUI boardUi;
        private BoardInteract boardInteract;

        public MainWindow()
        {
            InitializeComponent();


            chessGame = new ChessGame();
            chessGame.LoadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");


            boardUi = new BoardUI(PieceGrid, TurnLabel, images);
            boardUi.UpdateBoard(chessGame.GetBoardState());
            boardUi.UpdateTurnDisplay(chessGame.IsWhiteTURN());

            boardInteract = new BoardInteract(PieceGrid, chessGame, boardUi);
            
        }

    }
}
