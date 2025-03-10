using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Input;
using System.Windows;

namespace Chess
{
    class BoardInteract
    {
        private ChessGame chessGame;
        private BoardUI boardUi;
        private (int row, int col)? selectedPiece = null;
        private readonly Grid pieceGrid; // store to reference PieceGrid


        public BoardInteract( Grid pieceGrid, ChessGame chessGame, BoardUI boardUi)
        {
            this.pieceGrid = pieceGrid;
            this.chessGame = chessGame;
            this.boardUi = boardUi;

            // Attach event handler directly to constructor
            this.pieceGrid.MouseDown += PieceGrid_MouseDown;
        }

        public void PieceGrid_MouseDown(object sender, MouseButtonEventArgs e)
        {
            Point position = e.GetPosition(pieceGrid);
            double cellWidth = pieceGrid.ActualWidth / 8;
            double cellHeight = pieceGrid.ActualHeight / 8;

            int col = (int)(position.X / cellWidth);
            int row = (int)(position.Y / cellHeight);

            string[,] boardState = chessGame.GetBoardState();
            string piece = boardState[row, col];

            // If no piece is selected, attempt to select one
            if (selectedPiece == null)
            {
                if (!string.IsNullOrEmpty(piece)) // Ensure it's not an empty square
                {
                    if ((chessGame.IsWhiteTURN() && Char.IsUpper(piece[0])) ||
                        (!chessGame.IsWhiteTURN() && Char.IsLower(piece[0])))
                    {
                        selectedPiece = (row, col);
                        boardUi.HighlightSquare(row, col, Brushes.LightBlue);
                    }
                }
            }
            else
            {
                (int fromRow, int fromCol) = selectedPiece.Value;

                // If clicking the same square again, deselect it
                if (fromRow == row && fromCol == col)
                {
                    Console.WriteLine("Deselected piece");
                    selectedPiece = null;
                    boardUi.ClearHighlights();
                    return;
                }
              
                // Print move in algebraic notation for debugging purposes
                Console.WriteLine($"{(char)('a' + fromCol)}{8 - fromRow} -> {(char)('a' + col)}{8 - row}");

                // Convert squares to their little-endian ranking indexes
                int source = fromCol + 8 * (7 - fromRow);
                int target = col + 8 * (7 - row);

                // Validate move by getting valid moves from source square as a bitboard
                // and comparing target square with valid moves with bitwise OR
                ulong validMoves = chessGame.GetValidMoves(source);
                bool isValid = (validMoves & (1UL << target)) != 0;

                // Make a move if valid
                if (isValid)
                {
                    chessGame.MovePiece(source, target);
                    boardUi.UpdateBoard(chessGame.GetBoardState());
                    boardUi.UpdateTurnDisplay(chessGame.IsWhiteTURN());
                }

                boardUi.ClearHighlights();
                selectedPiece = null; // Deselect after move
            }
        }
    }
}
