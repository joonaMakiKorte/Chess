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
        private (int row, int col)? highlightedSquare = null;

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
                    // Add border to grid
                    Grid.SetRow(border, row);
                    Grid.SetColumn(border, col);
                    pieceGrid.Children.Add(border);

                    // Create image for each piece
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

            ClearValidMoveHighlights(); // Deletes former highlights


            //Console.WriteLine("Updating board...");
            for (int row = 0; row < 8; row++)
            {
                for (int col = 0; col < 8; col++)
                {

                    //Console.WriteLine($"Row {row}, Col {col}: {boardState[row, col]}");
                    string piece = boardState[row, col]; // Get piece at square
                    // Image for each piece
                    pieceImages[row, col].Source = images.GetPieceImage(piece);
                    // Set image alignment and stretch
                    pieceImages[row, col].HorizontalAlignment = HorizontalAlignment.Stretch;
                    pieceImages[row, col].VerticalAlignment = VerticalAlignment.Stretch;
                    pieceImages[row, col].Stretch = Stretch.UniformToFill;
                }
            }
        }

        public void UpdateTurnDisplay(bool isWhiteturn) => turnLabel.Content = isWhiteturn ? "White's Turn" : "Black's Turn";

        public void HighlightSquare(int row, int col, Brush color)
        {
            highlightedSquare = (row, col);
            pieceBorders[row, col].Background = color;
        }

        public void ClearHighlights()
        {
            // Clear highlights at selected square
            if (highlightedSquare != null)
            {
                (int row, int col) = highlightedSquare.Value;
                pieceBorders[row, col].Background = Brushes.Transparent;
                highlightedSquare = null;
            }
        }

        public void HighlightValidMoves(ulong validMoves)
        {

            ClearValidMoveHighlights();
            for (int square = 0; square < 64; square++)
            {
                if ((validMoves & (1UL << square)) != 0) // Check if bit is set
                {
                    int row = 7 - (square / 8);
                    int col = square % 8;
                    HighlightSquare(row, col, Brushes.LightGreen);
                }
            }
        }


        public void ClearValidMoveHighlights()
        {
            for (int row = 0; row < 8; row++)
            {
                for (int col = 0; col < 8; col++)
                {
                    pieceBorders[row, col].Background = Brushes.Transparent;
                }
            }
            highlightedSquare = null; // null so it doesn't remain in memory
        }
    }
}
