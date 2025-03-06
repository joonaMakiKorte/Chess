using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace Chess
{
    public class ChessGame
    {


        private bool isWhiteTurn = true; // white moves first


        private IntPtr board;
        string[,] pieceLocations = new string[8, 8] {
    // Rank 8 (Black back row)
    { "r", "n", "b", "q", "k", "b", "n", "r" },
    // Rank 7 (Black pawns)
    { "p", "p", "p", "p", "p", "p", "p", "p" },
    // Rank 6 (Empty)
    { "", "", "", "", "", "", "", "" },
    // Rank 5 (Empty)
    { "", "", "", "", "", "", "", "" },
    // Rank 4 (Empty)
    { "", "", "", "", "", "", "", "" },
    // Rank 3 (Empty)
    { "", "", "", "", "", "", "", "" },
    // Rank 2 (White pawns)
    { "P", "P", "P", "P", "P", "P", "P", "P" },
    // Rank 1 (White back row)
    { "R", "N", "B", "Q", "K", "B", "N", "R" }
    };

        public string[,] GetBoardState()
        {
            return pieceLocations;
        }
        public bool IsWhiteTURN()
        {
            return isWhiteTurn;
        }
        //Method to switch turn
        public void SwitchTurn()
        {
            isWhiteTurn = !isWhiteTurn; // switches between false and true
        }


        // Moves the piece
        public void MovePiece(int fromRow, int fromCol, int toRow, int toCol)
        {
            

            pieceLocations[toRow, toCol] = pieceLocations[fromRow, fromCol]; // Moves the piece
            pieceLocations[fromRow, fromCol] = ""; // clear the old pos
        }



    }
}
