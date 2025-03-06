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



namespace Chess
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    /// 
    public partial class MainWindow : Window
    {

        private readonly Image[,] pieceImages = new Image[8, 8];
        private ChessGame chessGame = new ChessGame();
        private Images images = new Images();
        private (int row, int col)? selectedPiece = null; // clicked piece pos

        public MainWindow()
        {
            InitializeComponent();
            chessGame = new ChessGame();
            chessGame.LoadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            InitializeBoard();
            UpdateBoard();


        }


        


        private void PieceGrid_MouseDown(object sender, MouseButtonEventArgs e)
        {
            // Get the mouse position relative to the grid.
            Point position = e.GetPosition(PieceGrid);

            // Calculate the dimensions of each cell assuming an 8x8 grid.
            double cellWidth = PieceGrid.ActualWidth / 8;
            double cellHeight = PieceGrid.ActualHeight / 8;

            // Determine the column and row indices based on the click.
            int col = (int)(position.X / cellWidth);
            int row = (int)(position.Y / cellHeight);

            // First click: select a piece if one exists.
            if (selectedPiece == null)
            {
                string piece = chessGame.GetBoardState()[row, col];

                // check if piece exists and if it belongs to right player
                if (!string.IsNullOrEmpty(piece))
                {
                    if ((chessGame.IsWhiteTURN() && Char.IsUpper(piece[0]) ||
                        (!chessGame.IsWhiteTURN() && Char.IsLower(piece[0])))) 
                    {
                        selectedPiece = (row, col);

                        pieceBorders[row, col].BorderBrush = Brushes.Blue; // change color
                    }


                else
                    {
                        return;
                    } 
                }
            }
            else // Second click: move the piece.
            {
                (int fromRow, int fromCol) = selectedPiece.Value;
                string move = $"{(char)('a' + fromCol)}{8 - fromRow}{(char)('a' + col)}{8 - row}";
                Console.WriteLine(move);
                chessGame.MovePiece(fromRow, fromCol, row, col);
                UpdateBoard();

                // clear border after move
                pieceBorders[fromRow, fromCol].BorderBrush = Brushes.Transparent;


                chessGame.SwitchTurn();

                UpdateTurnDisplay();
                selectedPiece = null; // Reset selection.
            }
        }


        //keeps track of borders
        private readonly Border[,] pieceBorders = new Border[8, 8]; // To keep track of borders
        private void InitializeBoard()
        {

            PieceGrid.Children.Clear(); // Clear older images

            for (int row = 0; row < 8; row++)
            {
                for (int col = 0; col < 8; col++)
                {


                    // Create a border for each square
                    Border border = new Border
                    {
                        BorderBrush = Brushes.Transparent, // no border first
                        BorderThickness = new Thickness(3), // set border's thickness
                        HorizontalAlignment = HorizontalAlignment.Stretch,
                        VerticalAlignment = VerticalAlignment.Stretch
                    };
                    pieceBorders[row, col] = border;

                    // add border to grid
                    Grid.SetRow(border, row);
                    Grid.SetColumn(border, col);
                    PieceGrid.Children.Add(border);


                    // create image for each piece
                    Image image = new Image
                    {
                        HorizontalAlignment = HorizontalAlignment.Stretch,
                        VerticalAlignment = VerticalAlignment.Stretch,
                        Stretch = Stretch.UniformToFill
                    };
                    pieceImages[row, col] = image;
                    
                    Grid.SetRow(image, row);
                    Grid.SetColumn(image, col);
                    PieceGrid.Children.Add(image);

                }
            }
        }

        private void UpdateBoard()
        {
            string[,] boardState = chessGame.GetBoardState();

            for (int row = 0; row < 8; row++)
            {
                for (int col = 0; col < 8; col++)
                {
                    string piece = boardState[row, col];

                    // image for each piece
                    pieceImages[row, col].Source = images.GetPieceImage(piece);

                    // set image alignment and stretch
                    pieceImages[row, col].HorizontalAlignment = HorizontalAlignment.Stretch;
                    pieceImages[row, col].VerticalAlignment = VerticalAlignment.Stretch;
                    pieceImages[row, col].Stretch = Stretch.UniformToFill;
                }
            }
        }

        private void UpdateTurnDisplay()
        {
           
            TurnLabel.Content = chessGame.IsWhiteTURN() ? "White's Turn" : "Black's Turn";


        }
    }
}
