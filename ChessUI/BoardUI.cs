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
        private Images images;
        private readonly Label turnLabel;

        // Move log logic
        public class MoveLogEntry
        {
           public int Turn { get; set; }
            public string WhiteMove { get; set; }
            public string BlackMove { get; set; }
        }
        private readonly ListView moveLogPanel;
        private readonly List<MoveLogEntry> moveLogEntries = new List<MoveLogEntry>();

        private ChessGame chessGame;

        private readonly Label whiteTimerLabel;
        private readonly Label blackTimerLabel;
        private DispatcherTimer whiteTimer;
        private DispatcherTimer blackTimer;
        private TimeSpan whiteTimeRemaining;
        private TimeSpan blackTimeRemaining;

        // Define an event to notify loss
        public event Action<string> LossOccurred;




        private (int row, int col)? highlightedSquare = null;

        public BoardUI(Grid grid, Label turnLabel, Label whiteTimerLabel, Label blackTimerLabel,
            Images images, int initialtimerMinutes,
            ListView moveLogPanel)
        {
            this.pieceGrid = grid;
            this.images = images;
            this.turnLabel = turnLabel;
            this.whiteTimerLabel = whiteTimerLabel;
            this.blackTimerLabel = blackTimerLabel;
            this.moveLogPanel = moveLogPanel;
            

            InitializeBoard();
            InitializeTimers(initialtimerMinutes);

            moveLogPanel.ItemsSource = moveLogEntries;
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
        private void InitializeTimers(int initialTimerMinutes)
        {
            // Set initial time for both players (e.g., 5 minutes each)
            whiteTimeRemaining = TimeSpan.FromMinutes(initialTimerMinutes);
            blackTimeRemaining = TimeSpan.FromMinutes(initialTimerMinutes);

            whiteTimer = new DispatcherTimer { Interval = TimeSpan.FromSeconds(1) };
            blackTimer = new DispatcherTimer { Interval = TimeSpan.FromSeconds(1) };

            whiteTimer.Tick += WhiteTimer_Tick;
            blackTimer.Tick += BlackTimer_Tick;

            UpdateTimerLabels();
        }

        private void WhiteTimer_Tick(object sender, EventArgs e)
        {
            if (whiteTimeRemaining > TimeSpan.Zero)
            {
                whiteTimeRemaining = whiteTimeRemaining.Subtract(TimeSpan.FromSeconds(1));
                UpdateTimerLabels();
            }
            else
            {
                whiteTimer.Stop();
                // Handle time out for white player
                ShowLossPopup("Black won! White player ran out of time!");

            }
        }

        private void BlackTimer_Tick(object sender, EventArgs e)
        {
            if (blackTimeRemaining > TimeSpan.Zero)
            {
                blackTimeRemaining = blackTimeRemaining.Subtract(TimeSpan.FromSeconds(1));
                UpdateTimerLabels();
            }
            else
            {
                blackTimer.Stop();
                // Handle time out for black player
                ShowLossPopup("White won! Black player ran out of time!");
            }
        }
        private void UpdateTimerLabels()
        {
            whiteTimerLabel.Content = $"White: {whiteTimeRemaining:mm\\:ss}";
            blackTimerLabel.Content = $"Black: {blackTimeRemaining:mm\\:ss}";
        }

        public void StartWhiteTimer()
        {
            whiteTimer.Start();
            blackTimer.Stop();
        }

        public void StartBlackTimer()
        {
            blackTimer.Start();
            whiteTimer.Stop();
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



        public void UpdateTurnDisplay( bool isWhiteTurn)
        {
            string turnText = isWhiteTurn ? "White's Turn" : "Black's Turn";
            turnLabel.Content = turnText;
            // start the timer at the same time as turn changes
            if (isWhiteTurn)
            {
                StartWhiteTimer();
            }
            else
            {
                StartBlackTimer();
            }

        }
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


        public static ulong EnPassantToBitboard(string enPassantSquare)
        {
            // Ensure the enPassantSquare is valid, e.g., "e6"
            if (enPassantSquare.Length != 2)
                throw new ArgumentException("Invalid en passant square notation.");

            char file = enPassantSquare[0]; // e.g., 'e'
            char rank = enPassantSquare[1]; // e.g., '6'

            // Convert the file (e.g., 'a' -> 0, 'b' -> 1, ..., 'h' -> 7)
            int fileIndex = file - 'a';

            // Convert the rank (e.g., '8' -> 0, '7' -> 1, ..., '1' -> 7)
            int rankIndex = 8 - (rank - '0');

            // Calculate the index on the bitboard
            int bitboardIndex = rankIndex * 8 + fileIndex;

            // Return the corresponding bitboard (1UL << bitboardIndex)
            return 1UL << bitboardIndex;
        }


        public void HighlightValidMoves(ulong validMoves)
        {

            
            ClearValidMoveHighlights();


            // make a string to check for enpassant
            string valids = ChessGame.BitboardToAlgebraic(validMoves);
            //Console.WriteLine(valids);
            


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

        public void ShowLossPopup(string reason)
        {
            // Freeze the chess board
            pieceGrid.IsEnabled = false;

            // Create and show the popup window
            LossPopup lossPopup = new LossPopup(reason);
            lossPopup.ShowDialog();   
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
    }
}
