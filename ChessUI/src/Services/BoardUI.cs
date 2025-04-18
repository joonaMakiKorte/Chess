using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows;
using System.Diagnostics;
using System.Windows.Media.Animation;
using System.Windows.Threading;

namespace Chess
{
    public class BoardUI
    {
        private readonly Grid pieceGrid;
        private readonly Border[,] pieceBorders = new Border[8, 8]; // To keep track of borders
        private readonly Image[,] pieceImages = new Image[8, 8];
        private Images images; // Hold piece image data

        // Move-log logic
        public class MoveLogEntry
        {
           public int Turn { get; set; }
            public string WhiteMove { get; set; }
            public string BlackMove { get; set; }
        }
        private readonly ListView moveLogPanel;
        private readonly List<MoveLogEntry> moveLogEntries = new List<MoveLogEntry>();

        // Timer logic
        // Top and bottom timers are determined by flipped flag
        private readonly Label topTimerLabel;
        private readonly Label bottomTimerLabel;
        private DispatcherTimer whiteTimer;
        private DispatcherTimer blackTimer;
        private TimeSpan whiteTimeRemaining;
        private TimeSpan blackTimeRemaining;

        // Store the single selected square using LOGIC coordinates internally
        private (int logicRow, int logicCol)? highlightedSquareLogicCoords = null;
        // Store valid move highlights using UI coordinates
        private List<(int uiRow, int uiCol)> highlightedValidMoveUiCoords = new List<(int uiRow, int uiCol)>();

        private bool isFlipped;

        public BoardUI(Grid grid, Label topTimerLabel, Label bottomTimerLabel,
            Images images, int initialTimerMinutes, bool flipped, ListView moveLogPanel)
        {
            this.pieceGrid = grid;
            this.images = images;
            this.topTimerLabel = topTimerLabel;
            this.bottomTimerLabel = bottomTimerLabel;
            this.moveLogPanel = moveLogPanel;
            this.isFlipped = flipped;

            InitializeBoard();
            InitializeTimers(initialTimerMinutes);

            moveLogPanel.ItemsSource = moveLogEntries;
        }
        
        public void InitializeBoard()
        {
            pieceGrid.Children.Clear(); // Clear older images

            for (int uiRow = 0; uiRow < 8; uiRow++)
            {
                for (int uiCol = 0; uiCol < 8; uiCol++)
                {
                    // Create a border for each square
                    Border border = new Border
                    {
                        BorderBrush = Brushes.Transparent, // no border first
                        BorderThickness = new Thickness(3), // set border's thickness
                        HorizontalAlignment = HorizontalAlignment.Stretch,
                        VerticalAlignment = VerticalAlignment.Stretch
                    };
                    // Store by UI coords
                    pieceBorders[uiRow, uiCol] = border;
                    // Add border to grid
                    Grid.SetRow(border, uiRow);
                    Grid.SetColumn(border, uiCol);
                    pieceGrid.Children.Add(border);

                    // Create image for each piece
                    Image image = new Image
                    {
                        HorizontalAlignment = HorizontalAlignment.Stretch,
                        VerticalAlignment = VerticalAlignment.Stretch,
                        Stretch = Stretch.UniformToFill
                    };
                    // Store by UI coords
                    pieceImages[uiRow, uiCol] = image;
                    // Add to grid at UI coords
                    Grid.SetRow(image, uiRow);
                    Grid.SetColumn(image, uiCol);
                    pieceGrid.Children.Add(image);

                }
            }
        }
        private void InitializeTimers(int initialTimerMinutes)
        {
            // Set initial time for both players (e.g., 5 minutes each)
            whiteTimeRemaining = TimeSpan.FromMinutes(initialTimerMinutes);
            blackTimeRemaining = TimeSpan.FromMinutes(initialTimerMinutes);

            whiteTimer = new DispatcherTimer { Interval = TimeSpan.FromSeconds(1) };
            blackTimer = new DispatcherTimer { Interval = TimeSpan.FromSeconds(1) };

            whiteTimer.Tick += WhiteTimerTick;
            blackTimer.Tick += BlackTimerTick;

            UpdateTimerLabels();
        }

 
        public void UpdateBoard(string[,] boardState)
        {
            for (int uiRow = 0; uiRow < 8; uiRow++) // Loop UI rows/cols
            {
                for (int uiCol = 0; uiCol < 8; uiCol++)
                {
                    // Get the corresponding LOGIC coordinates to read from boardState
                    (int logicRow, int logicCol) = GetLogicCoords(uiRow, uiCol);

                    // Get piece from the logic board state
                    string piece = boardState[logicRow, logicCol];

                    // Update the image at the current UI coordinate
                    pieceImages[uiRow, uiCol].Source = images.GetPieceImage(piece);
                    // Ensure alignment/stretch (might be redundant if set in InitializeBoard)
                    pieceImages[uiRow, uiCol].HorizontalAlignment = HorizontalAlignment.Stretch;
                    pieceImages[uiRow, uiCol].VerticalAlignment = VerticalAlignment.Stretch;
                    pieceImages[uiRow, uiCol].Stretch = Stretch.UniformToFill;
                }
            }
        }

        public void SwitchActiveTimer( bool isWhiteTurn)
        {
            // Start the timer at the same time as turn changes
            if (isWhiteTurn)
            {
                ToggleTimer(true, true); // Start white
                ToggleTimer(false, false); // Stop black
            }
            else
            {
                ToggleTimer(false, true); // Start black
                ToggleTimer(true, false); // Stop white
            }

        }
        
        // Takes LOGIC coordinates as input for correct mapping
        public void HighlightSquare(int logicRow, int logicCol, Brush color)
        {
            // Clear previous blue highlight first
            ClearHighlights();

            // Store the *logic* coordinates of the selected piece
            highlightedSquareLogicCoords = (logicRow, logicCol);

            // Calculate the UI coordinates to apply the highlight
            (int uiRow, int uiCol) = GetUiCoords(logicRow, logicCol);

            // Apply highlight to the border at the UI coordinate
            if (uiRow >= 0 && uiRow < 8 && uiCol >= 0 && uiCol < 8) // Bounds check
            {
                pieceBorders[uiRow, uiCol].Background = color;
            }
        }

        public void ClearHighlights()
        {
            // Clear the single selected square (blue) highlight
            if (highlightedSquareLogicCoords != null)
            {
                (int logicRow, int logicCol) = highlightedSquareLogicCoords.Value;
                // Calculate UI coords to clear the correct border
                (int uiRow, int uiCol) = GetUiCoords(logicRow, logicCol);

                if (uiRow >= 0 && uiRow < 8 && uiCol >= 0 && uiCol < 8) // Bounds check
                {
                    // Only clear if it wasn't also a valid move highlight (optional complexity)
                    // For simplicity, just clear it. Valid moves will be reapplied if needed.
                    pieceBorders[uiRow, uiCol].Background = Brushes.Transparent;
                }
                highlightedSquareLogicCoords = null;
            }
        }


        public void HighlightValidMoves(ulong validMoves)
        {
            // Clear previous valid move highlights FIRST
            ClearValidMoveHighlights();

            for (int square = 0; square < 64; square++)
            {
                if ((validMoves & (1UL << square)) != 0) // Check if bit is set
                {
                    // Calculate LOGIC coordinates from bitboard index
                    int logicRow = 7 - (square / 8);
                    int logicCol = square % 8;

                    // Calculate UI coordinates for highlighting
                    (int uiRow, int uiCol) = GetUiCoords(logicRow, logicCol);

                    // Apply highlight at UI coordinate
                    if (uiRow >= 0 && uiRow < 8 && uiCol >= 0 && uiCol < 8) // Bounds check
                    {
                        // Use a different color/style for valid moves
                        pieceBorders[uiRow, uiCol].Background = Brushes.LightGreen;
                        // Store the UI coord of the valid move highlight
                        highlightedValidMoveUiCoords.Add((uiRow, uiCol));
                    }
                }
            }
        }

        public void ClearValidMoveHighlights()
        {
            // Iterate through the stored list of highlighted valid move UI coordinates
            foreach (var coord in highlightedValidMoveUiCoords)
            {
                if (coord.uiRow >= 0 && coord.uiRow < 8 && coord.uiCol >= 0 && coord.uiCol < 8) // Bounds check
                {
                    // Check if this square is NOT the main selected piece square before clearing
                    bool isSelectedSquare = false;
                    if (highlightedSquareLogicCoords.HasValue)
                    {
                        var selectedUiCoords = GetUiCoords(highlightedSquareLogicCoords.Value.logicRow, highlightedSquareLogicCoords.Value.logicCol);
                        isSelectedSquare = selectedUiCoords.uiRow == coord.uiRow && selectedUiCoords.uiCol == coord.uiCol;
                    }

                    if (!isSelectedSquare) // Only clear if it's not the main selected square
                    {
                        pieceBorders[coord.uiRow, coord.uiCol].Background = Brushes.Transparent;
                    }
                }
            }
            // Clear the list
            highlightedValidMoveUiCoords.Clear();
        }

        public void LogMove(string move, bool isWhite, int currentTurn)
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                if (isWhite)
                {
                    moveLogEntries.Add(new MoveLogEntry
                    {
                        Turn = currentTurn,
                        WhiteMove = move,
                        BlackMove = ""
                    });
                }
                else
                {
                    // Update the last entry
                    if (moveLogEntries.Count > 0)
                    {
                        moveLogEntries[moveLogEntries.Count - 1].BlackMove = move;
                    }
                    currentTurn++;
                }

                moveLogPanel.Items.Refresh(); // Update UI

                // Make sure there is at least one entry
                if (moveLogEntries.Count > 0)
                {
                    moveLogPanel.ScrollIntoView(moveLogEntries.Last());
                }
            });
        }

        // Helper for coordinate mapping
        private (int uiRow, int uiCol) GetUiCoords(int logicRow, int logicCol)
        {
            int uiRow = isFlipped ? (7 - logicRow) : logicRow;
            int uiCol = isFlipped ? (7 - logicCol): logicCol;

            return (uiRow, uiCol);
        }

        private (int logicRow, int logicCol) GetLogicCoords(int uiRow, int uiCol)
        {
            int logicRow = isFlipped? (7 - uiRow) : uiRow;
            int logicCol = isFlipped? (7 - uiCol): uiCol;

            return (logicRow, logicCol);
        }

        private void WhiteTimerTick(object sender, EventArgs e)
        {
            if (whiteTimeRemaining > TimeSpan.Zero)
            {
                whiteTimeRemaining = whiteTimeRemaining.Subtract(TimeSpan.FromSeconds(1));
                UpdateTimerLabels();
            }
            else
            {
                whiteTimer.Stop();

            }
        }

        private void BlackTimerTick(object sender, EventArgs e)
        {
            if (blackTimeRemaining > TimeSpan.Zero)
            {
                blackTimeRemaining = blackTimeRemaining.Subtract(TimeSpan.FromSeconds(1));
                UpdateTimerLabels();
            }
            else
            {
                blackTimer.Stop();
            }
        }
        private void UpdateTimerLabels()
        {
            // Handle normal and flipped UI logic
            if (isFlipped)
            {
                topTimerLabel.Content = $"{whiteTimeRemaining:mm\\:ss}";
                bottomTimerLabel.Content = $"{blackTimeRemaining:mm\\:ss}";
            }
            else
            {
                bottomTimerLabel.Content = $"{whiteTimeRemaining:mm\\:ss}";
                topTimerLabel.Content = $"{blackTimeRemaining:mm\\:ss}";
            }
        }

        // Toggle timers
        public void ToggleTimer(bool white, bool start)
        {
            if (white)
            {
                if (start)
                {
                    whiteTimer.Start();
                }
                else
                {
                    whiteTimer.Stop();
                }
            }
            else
            {
                if (start)
                {
                    blackTimer.Start();
                }
                else
                {
                    blackTimer.Stop();
                }
            }
        }
    }
}
