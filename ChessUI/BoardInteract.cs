using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Input;
using System.Windows;
using System.Media;

namespace Chess
{
    class BoardInteract
    {
        private ChessGame chessGame;
        private BoardUI boardUi;
        private (int row, int col)? selectedPiece = null;
        private readonly Grid pieceGrid; // store to reference PieceGrid

        // list of all sound players
        private List<SoundPlayer> soundPlayers = new List<SoundPlayer>(); 


        public BoardInteract( Grid pieceGrid, ChessGame chessGame, BoardUI boardUi)
        {
            this.pieceGrid = pieceGrid;
            this.chessGame = chessGame;
            this.boardUi = boardUi;

            // Attach event handler directly to constructor
            this.pieceGrid.MouseDown += PieceGrid_MouseDown;

            // Ensure pieceGrid can receive key events
            this.pieceGrid.Focusable = true;
            this.pieceGrid.Focus();  // Force focus to receive key events
        }


        
public async void PieceGrid_MouseDown(object sender, MouseButtonEventArgs e)
        {
            // Prevent selection if game mode is AI and it's Black's turn
            if (chessGame.GameMode == "AI" && !chessGame.IsWhiteTURN())
            {
                return;
                
            }
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

                        // Highlight valid moves  
                        int source = col + 8 * (7 - row);
                        ulong validMoves = chessGame.GetValidMoves(source);
                        boardUi.HighlightValidMoves(validMoves);
                    }
                }
            }
            else
            {
                (int fromRow, int fromCol) = selectedPiece.Value;

                // If clicking the same square again, deselect it  
                if (fromRow == row && fromCol == col)
                {
                    selectedPiece = null;
                    boardUi.ClearHighlights();
                    boardUi.ClearValidMoveHighlights(); // Clear valid move highlights as well  
                    return;
                }

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

                    // Prevent selection if game mode is AI and it's Black's turn
                    if (chessGame.GameMode == "AI" && !chessGame.IsWhiteTURN())
                    {
                        await Task.Run(() =>
                        {
                            // Run AI move on a separate thread  
                            chessGame.MakeBlackMove();
                        });
                        // Ensure UI updates happen on the main thread
                        Application.Current.Dispatcher.Invoke(() =>
                        {
                            boardUi.UpdateBoard(chessGame.GetBoardState());
                            boardUi.UpdateTurnDisplay(chessGame.IsWhiteTURN());
                            boardUi.ClearHighlights();
                            boardUi.ClearValidMoveHighlights();
                            selectedPiece = null; // Deselect after move
                        });

                    }

                }

                boardUi.ClearHighlights();
                boardUi.ClearValidMoveHighlights();
                selectedPiece = null; // Deselect after move  
            }
        }
        private void MuteButton_Click(object sender, RoutedEventArgs e)
        {
            // Stop all sounds
            foreach (var player in soundPlayers)
            {
                player.Stop();
            }
        }


        
    }
}
