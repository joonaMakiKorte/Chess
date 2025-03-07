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
        private (int row, int col)? selectedPiece = null; // clicked piece pos
        private BoardUI boardUi;

        


        public MainWindow()
        {
            boardUi = new BoardUI(PieceGrid, TurnLabel, images);
            boardUi.UpdateBoard(chessGame.GetBoardState());
            boardUi.UpdateTurnDisplay(chessGame.IsWhiteTURN());
            InitializeComponent();
            chessGame = new ChessGame();
            chessGame.LoadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            
        }


        private void PieceGrid_MouseDown(object sender, MouseButtonEventArgs e)
        {
            Point position = e.GetPosition(PieceGrid);
            double cellWidth = PieceGrid.ActualWidth / 8;
            double cellHeight = PieceGrid.ActualHeight / 8;

            int col = (int)(position.X / cellWidth);
            int row = (int)(position.Y / cellHeight);

            if (selectedPiece == null)
            {
                string piece = chessGame.GetBoardState()[row, col];
                if (!string.IsNullOrEmpty(piece))
                {
                    if ((chessGame.IsWhiteTURN() && Char.IsUpper(piece[0])) ||
                        (!chessGame.IsWhiteTURN() && Char.IsLower(piece[0])))
                    {
                        selectedPiece = (row, col);
                    }
                }
                else
                {
                    return;
                }
            }
            else
            {
                (int fromRow, int fromCol) = selectedPiece.Value;
                string move = $"{(char)('a' + fromCol)}{8 - fromRow}{(char)('a' + col)}{8 - row}";
                Console.WriteLine(move);
                chessGame.MovePiece(fromRow, fromCol, row, col);
                boardUi.UpdateBoard(chessGame.GetBoardState());

                chessGame.SwitchTurn();
                boardUi.UpdateTurnDisplay(chessGame.IsWhiteTURN());
                selectedPiece = null;
            }
        }





    }
}
