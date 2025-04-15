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
        private (int logicRow, int logicCol)? selectedPiece = null; // LOGIC coordinates of the selected piece
        private readonly Grid pieceGrid; // store to reference PieceGrid
        private bool isProcessingMove = false; // Flag to prevent overlapping actions
        private bool isFlipped;

        // Small delay for AI and timer start in the initial call
        const int initialStartDelayMs = 750;

        public BoardInteract( Grid pieceGrid, ChessGame chessGame, BoardUI boardUi, bool flipped)
        {
            this.pieceGrid = pieceGrid;
            this.chessGame = chessGame;
            this.boardUi = boardUi;
            this.isFlipped = flipped; // Flipped logic if black is bottom

            // Attach event handler directly to constructor
            this.pieceGrid.MouseDown += PieceGrid_MouseDown;
            this.pieceGrid.Focusable = true; // Ensure pieceGrid can receive key events
        }

        // Gets called AFTER the game window is loaded and the board is displayed initially
        public async Task StartGameASync()
        {
            // Set initial focus
            if (pieceGrid != null)
            {
                pieceGrid.Focus();
            }

            // Apply delay for timer start
            await Task.Delay(initialStartDelayMs);

            // Double the delay if AI starts
            if (chessGame.isAIGame && chessGame.isWhiteAI)
            {
                await Task.Delay(initialStartDelayMs);
            }

            // Start timer
            boardUi.SwitchActiveTimer(chessGame.isWhiteTurn);
            
            // Check if AI needs to make the first move (e.g., White AI)
            await CheckAndMakeAIMoveAsync();
        }

        // --- AI Move Logic ---
        public async Task CheckAndMakeAIMoveAsync()
        {
            // Check if it's an AI's turn and we're not already processing
            bool isAITurn = chessGame.isAIGame && (chessGame.isWhiteAI == chessGame.isWhiteTurn);

            if (isAITurn && chessGame.isOngoing)
            {
                // Prevent re-entrancy / concurrency ONLY if called again while processing
                if (isProcessingMove) return;

                isProcessingMove = true;
                pieceGrid.IsEnabled = false; // Disable board input during AI move

                try
                {
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
                        boardUi.SwitchActiveTimer(chessGame.isWhiteTurn);
                        boardUi.ClearHighlights(); // Clear any selection visuals
                        boardUi.ClearValidMoveHighlights();
                        selectedPiece = null; // Ensure no piece is selected after AI move

                        // Check for game end AFTER AI move
                        chessGame.CheckGameEnd();
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
            if (isProcessingMove || isAITurn || !chessGame.isOngoing) // Also prevent clicks if game over
            {
                return;
            }

            Point position = e.GetPosition(pieceGrid);
            // Prevent errors if grid size is zero
            if (pieceGrid.ActualWidth == 0 || pieceGrid.ActualHeight == 0) return;

            double cellWidth = pieceGrid.ActualWidth / 8;
            double cellHeight = pieceGrid.ActualHeight / 8;

            // Calculate UI row/col clicked
            int uiCol = (int)(position.X / cellWidth);
            int uiRow = (int)(position.Y / cellHeight);

            // Bounds check for safety
            if (uiRow < 0 || uiRow > 7 || uiCol < 0 || uiCol > 7) return;

            // *** TRANSLATE UI Coords to LOGIC Coords ***
            int logicRow = isFlipped ? (7 - uiRow) : uiRow;
            int logicCol = isFlipped ? (7 - uiCol) : uiCol;

            string[,] boardState = chessGame.GetBoardState();
            // Use LOGIC coords to get piece info
            string piece = boardState[logicRow, logicCol];

            // If no piece is selected, attempt to select one  
            if (selectedPiece == null)
            {
                if (!string.IsNullOrEmpty(piece)) // Ensure it's not an empty square  
                {
                    if ((chessGame.isWhiteTurn && Char.IsUpper(piece[0])) ||
                        (!chessGame.isWhiteTurn && Char.IsLower(piece[0])))
                    {
                        // Store LOGIC coords
                        selectedPiece = (logicRow, logicCol);
                        boardUi.ClearHighlights();
                        // Highlight using LOGIC coords (BoardUI handles mapping)
                        boardUi.HighlightSquare(logicRow, logicCol, Brushes.LightBlue);

                        // Calculate source index using LOGIC coords
                        int source = logicCol + 8 * (7 - logicRow); // Formula expects logic coords
                        ulong validMoves = chessGame.GetValidMoves(source);
                        // Highlight valid moves (BoardUI handles mapping)
                        boardUi.HighlightValidMoves(validMoves);
                    }
                }
            }
            else
            {
                // Retrieve the LOGIC coordinates of the already selected piece
                (int fromLogicRow, int fromLogicCol) = selectedPiece.Value;

                // Clear visual selection highlights immediately
                selectedPiece = null; // Deselect logical storage first
                boardUi.ClearHighlights();
                boardUi.ClearValidMoveHighlights();

                // If clicking the same square again, don't proceed
                if (fromLogicRow == logicRow && fromLogicCol == logicCol)
                {
                    return; // Just deselected
                }

                // If clicking another piece of the same color, select that one instead
                if (!string.IsNullOrEmpty(piece) &&
                   ((chessGame.isWhiteTurn && Char.IsUpper(piece[0])) ||
                    (!chessGame.isWhiteTurn && Char.IsLower(piece[0]))))
                {
                    // Select the new piece (store LOGIC coords)
                    selectedPiece = (logicRow, logicCol);
                    // Highlight using LOGIC coords
                    boardUi.HighlightSquare(logicRow, logicCol, Brushes.LightBlue);
                    // Show its valid moves (calculate source index using LOGIC coords)
                    int newSource = logicCol + 8 * (7 - logicRow);
                    ulong newValidMoves = chessGame.GetValidMoves(newSource);
                    boardUi.HighlightValidMoves(newValidMoves);
                    return; // Switched selection
                }

                // Convert selected LOGIC coords to source index
                int source = fromLogicCol + 8 * (7 - fromLogicRow);
                // Convert clicked LOGIC coords to target index
                int target = logicCol + 8 * (7 - logicRow);

                // Validate move using source/target indices
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
                        boardUi.SwitchActiveTimer(chessGame.isWhiteTurn); // Update turn display

                        // Check for game end
                        chessGame.CheckGameEnd();

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
