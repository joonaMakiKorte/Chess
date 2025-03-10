using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

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
                        for (int i=0; i<emptyCount; i++)
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

            // Read full moves
            fullMoves = int.Parse(sections[5]);
        }

        // Gives the pieceLocations dict fully
        public string[,] GetBoardState() => pieceLocations;

        // Get turn
        public bool IsWhiteTURN() => isWhiteTurn;


        

        // Moves the piece
        public bool MovePiece(string move)

        {
            
            Console.WriteLine(move); // Debug

            if (!ChessEngineInterop.ValidateMove(board,move))
            {
                Console.WriteLine($"Invalid move: {move}");
                return false;
            }

            // Apply move in the native engine
            ChessEngineInterop.MakeMove(board, move);

            // Update local board state from DLL
            string fen = ChessEngineInterop.GetBoardStateString(board);
            LoadFromFEN(fen);
            Console.WriteLine(fen);


            return true;
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
