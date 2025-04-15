using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace Chess
{
    public partial class PromotionWindow : Window
    {
        // Property to hold the chosen piece character ('q', 'r', 'n', 'b')
        public char SelectedPromotionPiece { get; private set; } = '-'; // Default to invalid/none

        // Constructor takes whose turn it is and the image lookup table
        public PromotionWindow(bool isPromotingPlayerWhite, Images pieceImages)
        {
            InitializeComponent();

            // --- Determine Correct Dictionary Keys ---
            // Use uppercase for white pieces, lowercase for black, 'N'/'n' for Knight
            string queenKey = isPromotingPlayerWhite ? "Q" : "q";
            string rookKey = isPromotingPlayerWhite ? "R" : "r";
            string bishopKey = isPromotingPlayerWhite ? "B" : "b";
            string knightKey = isPromotingPlayerWhite ? "N" : "n"; // Use 'N'/'n'

            // --- Set Image Sources from Lookup Table ---
            QueenImage.Source = pieceImages.GetPieceImage(queenKey);
            RookImage.Source = pieceImages.GetPieceImage(rookKey);
            BishopImage.Source = pieceImages.GetPieceImage(bishopKey);
            KnightImage.Source = pieceImages.GetPieceImage(knightKey);

            // --- Ensure Button Tags use standard lowercase chars for selection logic ---
            // (Even though keys N/n are used for lookup)
            QueenButton.Tag = "q";
            RookButton.Tag = "r";
            BishopButton.Tag = "b";
            KnightButton.Tag = "n"; // Use 'n' for the tag that becomes SelectedPromotionPiece
        }

        // Button click handler remains the same - uses the Tag property
        private void PromotionButton_Click(object sender, RoutedEventArgs e)
        {
            if (sender is Button clickedButton && clickedButton.Tag is string pieceTag)
            {
                // SelectedPromotionPiece expects 'q', 'r', 'b', or 'n'
                SelectedPromotionPiece = pieceTag.ToLowerInvariant()[0];
                this.DialogResult = true; // Signal that a choice was made
                this.Close();
            }
        }

        // Optional: Escape key handler remains the same
        private void Window_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
            {
                this.DialogResult = false; // Signal cancellation
                this.Close();
            }
        }
    }
}