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
    /// <summary>
    /// Handle board interactions and coordinate AI moves
    /// </summary>
    class BoardInteract
    {
        private ChessGame chessGame;
        private BoardUI boardUi;
        private (int row, int col)? selectedPiece = null;
        private readonly Grid pieceGrid; // store to reference PieceGrid
        private bool isProcessingMove = false; // Flag to prevent overlapping actions

        public BoardInteract( Grid pieceGrid, ChessGame chessGame, BoardUI boardUi)
        {
            this.pieceGrid = pieceGrid;
            this.chessGame = chessGame;
            this.boardUi = boardUi;

            // Attach event handler directly to constructor
            this.pieceGrid.MouseDown += PieceGrid_MouseDown;
            this.pieceGrid.Focusable = true; // Ensure pieceGrid can receive key events
        }

        // Gets called AFTER the game window is loaded and the board is displayed initially
        public async Task StartGameASync()
        {
            // Set initial focus
            pieceGrid.Focus();
            // Check if AI needs to make the first move (e.g., White AI)
            await CheckAndMakeAIMoveAsync();
        }

        // --- AI Move Logic ---
        public async Task CheckAndMakeAIMoveAsync()
        {
            // Check if it's an AI's turn and we're not already processing
            bool isAITurn = chessGame.isAIGame && (chessGame.isWhiteAI == chessGame.isWhiteTurn);

            if (isAITurn)
            {
                // Prevent re-entrancy / concurrency ONLY if called again while processing
                if (isProcessingMove) return;

                isProcessingMove = true;
                pieceGrid.IsEnabled = false; // Disable board input during AI move

                try
                {
                    // Indicate AI is thinking (optional)
                    // boardUi.ShowThinkingIndicator(true);

                    await Task.Run(() =>
                    {
                        // This runs on a background thread
                        chessGame.MakeAIMove();
                    });

                    // Ensure UI updates happen back on the main thread
                    Application.Current.Dispatcher.Invoke(() =>
                    {
                        // boardUi.ShowThinkingIndicator(false);
                        boardUi.UpdateBoard(chessGame.GetBoardState());
                        boardUi.UpdateTurnDisplay(chessGame.IsWhiteTURN());
                        boardUi.ClearHighlights(); // Clear any selection visuals
                        boardUi.ClearValidMoveHighlights();
                        selectedPiece = null; // Ensure no piece is selected after AI move

                        // Check for game end AFTER AI move
                        //CheckGameEnd();
                    });
                }
                catch (Exception ex)
                {
                    // Handle potential errors during AI move or UI update
                    MessageBox.Show($"Error during AI move: {ex.Message}", "AI Error", MessageBoxButton.OK, MessageBoxImage.Error);
                    // Consider resetting game state or allowing user interaction if error occurs
                }
                finally
                {
                    isProcessingMove = false;
                    // Re-enable grid ONLY if it's now a human's turn
                    bool isNextTurnHuman = !chessGame.isAIGame || (chessGame.isWhiteAI != chessGame.IsWhiteTURN());
                    if (isNextTurnHuman /* && !IsGameOver()*/) // Re-enable only if game is not over
                    {
                        pieceGrid.IsEnabled = true;
                        pieceGrid.Focus(); // Ensure grid can receive input
                    }
                }
            }
        }
        
        // --- Mouse Down Handler ---
        public async void PieceGrid_MouseDown(object sender, MouseButtonEventArgs e)
        {
            // Prevent clicks if processing or it's AI's turn
            bool isAITurn = chessGame.isAIGame && (chessGame.isWhiteAI == chessGame.isWhiteTurn);
            if (isProcessingMove || isAITurn) // Also prevent clicks if game over
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
                    if ((chessGame.isWhiteTurn && Char.IsUpper(piece[0])) ||
                        (!chessGame.isWhiteTurn && Char.IsLower(piece[0])))
                    {
                        selectedPiece = (row, col);
                        boardUi.ClearHighlights(); // Clear previous selection highlight
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

                // Clear visual selection highlights immediately
                var pieceJustSelected = selectedPiece; // Store for potential re-selection logic
                selectedPiece = null;
                boardUi.ClearHighlights();
                boardUi.ClearValidMoveHighlights();

                // If clicking the same square again, don't proceed
                if (fromRow == row && fromCol == col)
                {
                    return; 
                }

                // If clicking another piece of the same color, select that one instead
                if (!string.IsNullOrEmpty(piece) &&
                   ((chessGame.isWhiteTurn && Char.IsUpper(piece[0])) ||
                    (!chessGame.isWhiteTurn && Char.IsLower(piece[0]))))
                {
                    selectedPiece = (row, col);
                    boardUi.HighlightSquare(row, col, Brushes.LightBlue);
                    int newSource = col + 8 * (7 - row);
                    ulong newValidMoves = chessGame.GetValidMoves(newSource);
                    boardUi.HighlightValidMoves(newValidMoves);
                    return; // Switched selection, don't proceed with move
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
                    pieceGrid.IsEnabled = false; // Disable grid during move applying

                    try
                    {
                        chessGame.MovePiece(source, target); // Make human move
                        boardUi.UpdateBoard(chessGame.GetBoardState()); // Update board
                        boardUi.UpdateTurnDisplay(chessGame.IsWhiteTURN()); // Update turn display

                        // TODO
                        // Check for game end AFTER human move

                        // Trigger the AI check/move AFTER human move completes and UI updates
                        await CheckAndMakeAIMoveAsync();
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show($"Error processing move: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                        // Consider resetting state or handling error
                    }
                    finally
                    {
                        pieceGrid.IsEnabled = true;
                    }
                }
 
            }
        }    
    }
}
