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
        private string[,] pieceLocations = new string[8, 8];
        public ChessGame()
        {
            LoadFromFEN("rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
        }

        public void LoadFromFEN(string fen)
        {
            string[] sections = fen.Split(' '); // split fen string
            Console.WriteLine(sections);

            string[] rows = sections[0].Split('/'); // Board setup part

            for (int row = 0; row < 8; row++)
            {
                int col = 0;

                // iterate over every row
                foreach (char c in rows[row])
                {
                    if (char.IsDigit(c)) // empty square
                    {
                        int emptyCount = c - '0';

                        //iterate accoding to c
                        for (int i=0; i<emptyCount; i++)
                        {
                            // add nothing to every part
                            pieceLocations[row, col++] = "";
                        }

                    }

                    else // piece character is in the spot
                    {
                        pieceLocations[row, col++] = c.ToString();
                    }
                }
            }

        }




    

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
