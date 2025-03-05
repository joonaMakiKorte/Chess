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

        public MainWindow()
        {
            InitializeComponent();
            InitializeBoard();
            UpdateBoard();


        }

        private void InitializeBoard()
        {

            PieceGrid.Children.Clear();
            for (int row = 0; row < 8; row++)
            {
                for (int col = 0; col < 8; col++)
                {
                    Image image = new Image();
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
                    pieceImages[row, col].Source = images.GetPieceImage(piece);
                }
            }
        }
    }
}
