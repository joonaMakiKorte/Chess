using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Animation;


namespace Chess
{
    public class ChessGame : IDisposable
    {
        private BoardUI boardUI;
        public bool isWhiteTurn { get; private set; } // Make active turn readable to others
        private int fullMoves; // Store move clocks

        private string[,] pieceLocations = new string[8, 8]; // Init empty 8x8 grid to store board pieces
        private IntPtr board; // Pointer to native board
        public bool isAIGame { get; private set; } // Store game mode
        public bool isWhiteAI { get; private set; } // Store player playing as ai
        private int difficulty; // None initially if human v human game

        // Store previous board state info after move
        ChessEngineInterop.BoardStatusInfo boardStatusInfo;

        // Event to notify about game over conditions
        public event Action<string> GameOver;

        public ChessGame(bool whiteIsHuman, bool blackIsHuman, bool bottomIsWhite, string aiDifficulty, BoardUI UI)
        {
            isWhiteTurn = true; // Initially white turn
            isAIGame = !whiteIsHuman || !blackIsHuman; // Determine game mode
            isWhiteAI = !whiteIsHuman; // Determine AI player

            // Get difficulty
            if (aiDifficulty != null)
            {
                if (aiDifficulty == "Easy") this.difficulty = 1;
                else if (aiDifficulty == "Medium") this.difficulty = 3;
                else this.difficulty = 5;
            }

            this.boardUI = UI; // Instance to UI

            board = ChessEngineInterop.CreateBoard(); // Initialize DLL board
            if (board == IntPtr.Zero)
            {
                throw new Exception("Failed to initialize the chess board from the engine.");
            }

            // Get initial board state and apply to pieceLocations
            boardStatusInfo = ChessEngineInterop.GetBoardStatus(board);
            LoadFromFEN(boardStatusInfo.Fen);
        }

        // Parse data from FEN representation of the board status
        private void LoadFromFEN(string fen)
        {
            string[] sections = fen.Split(' '); // Split
            string[] rows = sections[0].Split('/'); // Parts for board setup

            // Apply piece locations for rows
            for (int row = 0; row < 8; row++)
            {
                int col = 0;
                // Iterate over every row
                foreach (char c in rows[row])
                {
                    if (char.IsDigit(c)) // Empty square
                    {
                        int emptyCount = c - '0';
                        //Iterate according to c
                        for (int i = 0; i < emptyCount; i++)
                        {
                            // Add nothing to every part
                            pieceLocations[row, col++] = "";
                        }

                    }
                    else // Piece character is in the spot
                    {
                        pieceLocations[row, col++] = c.ToString();
                    }
                }
            }

            // Read turn
            isWhiteTurn = sections[1] == "w";

            // Read full moves
            fullMoves = int.Parse(sections[5]);
        }

        // Function to convert a bitboard to algebraic notation
        public static string BitboardToAlgebraic(ulong bitboard)
        {
            List<string> moves = new List<string>();

            for (int square = 0; square < 64; square++)
            {
                if ((bitboard & (1UL << square)) != 0) // Check if bit is set
                {
                    moves.Add(SquareToAlgebraic(square));
                }
            }

            return string.Join(", ", moves);
        }

        // Converts a square index (0-63) to algebraic notation (e.g., "e4")
        static string SquareToAlgebraic(int square)
        {
            int file = square % 8;  // Column (0 = 'a', 1 = 'b', ..., 7 = 'h')
            int rank = square / 8;  // Row (0 = rank 1, 7 = rank 8)

            char fileChar = (char)('a' + file);
            char rankChar = (char)('1' + rank);

            return $"{fileChar}{rankChar}";
        }

        // Gives the pieceLocations dict fully
        public string[,] GetBoardState() => pieceLocations;

        // Get turn
        public bool IsWhiteTURN() => isWhiteTurn;

        // Get all valid moves from the square as a bitboard
        public ulong GetValidMoves(int square)
        {
            ulong validMoves =  ChessEngineInterop.ValidMoves(board, square);    
            return validMoves;
        }

        // Moves the piece
        // Activated only after move is validated
        public void MovePiece(int source, int target)

        {
            // Temporary pawn promotion logic
            // Is promotion when pawn reaches the last rank
            // Queen = 'q', rook = 'r', bishop = 'b', knight = 'k'
            char promotion = '-'; // No promotion by default
            if (((pieceLocations[7 - (source / 8), source % 8] == "P" && target >= 56) ||
               (pieceLocations[7 - (source / 8), source % 8] == "p") && target <= 7))
            {
                promotion = 'q';
            }

            // Apply move in the native engine
            ChessEngineInterop.MakeMove(board, source, target, promotion);

            // Get new state
            boardStatusInfo = ChessEngineInterop.GetBoardStatus(board);
            LoadFromFEN(boardStatusInfo.Fen); // Update fen
            boardUI.LogMove(boardStatusInfo.Move, !isWhiteTurn, fullMoves); // Log move

        }

        public void MakeAIMove()
        {
            ChessEngineInterop.MakeBestMove(board,difficulty,isWhiteAI);

            // Ensure all UI updates happen on the main thread
            Application.Current.Dispatcher.Invoke(() =>
            {
                // Get new state
                boardStatusInfo = ChessEngineInterop.GetBoardStatus(board);
                LoadFromFEN(boardStatusInfo.Fen); // Update fen
                boardUI.LogMove(boardStatusInfo.Move, !isWhiteTurn, fullMoves); // Log move
            });
        }

        // Destroy board
        public void Dispose()
        {
            if (board != IntPtr.Zero)
            {
                ChessEngineInterop.DestroyBoard(board);
                board = IntPtr.Zero;
            }
        }

        ~ChessGame()
        {
            Dispose();
        }

    }
}
