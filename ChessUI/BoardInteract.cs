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

            if (selectedPiece == null)
            {
                string piece = chessGame.GetBoardState()[row, col];
                if (!string.IsNullOrEmpty(piece))
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
                // Convert selected squares to "e4e3" for example
                (int fromRow, int fromCol) = selectedPiece.Value;
                string move = $"{(char)('a' + fromCol)}{8 - fromRow}{(char)('a' + col)}{8 - row}";
                // Apply move in dll
                if (chessGame.MovePiece(move))
                {
                    boardUi.UpdateBoard(chessGame.GetBoardState());
                    boardUi.UpdateTurnDisplay(chessGame.IsWhiteTURN());
                    boardUi.ClearHighlights();
                    selectedPiece = null; // Deselect
                }
            }
        }
    }
}
