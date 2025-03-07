using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows;
using System.Diagnostics;

namespace Chess
{
    class BoardUI
    {
        private readonly Grid pieceGrid;
        private readonly Border[,] pieceBorders = new Border[8, 8]; // To keep track of borders
        private readonly Image[,] pieceImages = new Image[8, 8];
        private Images images;
        private readonly Label turnLabel;

        public BoardUI(Grid grid, Label turnLabel, Images images)
        {
            this.pieceGrid = grid;
            this.images = images;
            this.turnLabel = turnLabel;
            InitializeBoard();
        }
            
        



        public void InitializeBoard()
        {

            pieceGrid.Children.Clear(); // Clear older images

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
                    pieceGrid.Children.Add(border);


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
                    pieceGrid.Children.Add(image);

                }
            }
        }

        public void UpdateBoard(string[,] boardState)
        {

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

        public void UpdateTurnDisplay(bool isWhiteturn)
        {

            turnLabel.Content =isWhiteturn ? "White's Turn" : "Black's Turn";


        }

        public void HighlightSquare(int row, int col, Brush color)
        {
            pieceBorders[row, col].Background = color;
        }

        public void ClearHighlights()
        {
            for (int row = 0; row < 8; row++)
            {
                for (int col = 0; col < 8; col++)
                {
                    pieceBorders[row, col].Background = Brushes.Transparent;
                }
            }
        }
    }
}
