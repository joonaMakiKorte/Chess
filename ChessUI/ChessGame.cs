using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Animation;

namespace Chess
{
    public class ChessGame : IDisposable
    {
        private bool isWhiteTurn = true; // White moves first
        private bool whiteKingside, whiteQueenside, blackKingside, blackQueenside; // Store castling rights
        private string enPassantTarget; // Store en passant target square using algebraic notation
        private int halfMoves, fullMoves; // Store move clocks

        private string[,] pieceLocations = new string[8, 8]; // Init empty 8x8 grid to store board pieces
        private IntPtr board; // Pointer to native board



        

        public ChessGame()
        {
           

            board = ChessEngineInterop.CreateBoard(); // Initialize DLL board
            if (board == IntPtr.Zero)
            {
                throw new Exception("Failed to initialize the chess board from the engine.");
            }

            // Get initial board state and apply to pieceLocations
            string fen = ChessEngineInterop.GetBoardStateString(board);
            LoadFromFEN(fen);
        }

        // this is to move halfmove updates
        public event Action<int> OnHalfMoveUpdated;

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
                        //Iterate accoding to c
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

            // Read castling rights
            whiteKingside = sections[2].Contains("K");
            whiteQueenside = sections[2].Contains("Q");
            blackKingside = sections[2].Contains("k");
            blackQueenside = sections[2].Contains("q");

            // Read en passant target square
            enPassantTarget = sections[3].ToString();



            // Read half moves
            halfMoves = int.Parse(sections[4]);
            Console.WriteLine(halfMoves);
            OnHalfMoveUpdated?.Invoke(halfMoves); // Notify UI

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
            Console.WriteLine("Valid Moves: " + BitboardToAlgebraic(validMoves));
            return validMoves;
        }


        

        
        // Moves the piece
        // Activated only after move is validated
        public void MovePiece(int source, int target)

        {
            // Apply move in the native engine
            ChessEngineInterop.MakeMove(board, source, target);

            // Update local board state from DLL
            string fen = ChessEngineInterop.GetBoardStateString(board);
            Console.WriteLine(fen);
            LoadFromFEN(fen);

            bool mate = ChessEngineInterop.isCheckmate(board);
            bool check =  ChessEngineInterop.isInCheck(board);
            Console.WriteLine("Checkmate: " + mate + ", In Check: " + check);


            

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
