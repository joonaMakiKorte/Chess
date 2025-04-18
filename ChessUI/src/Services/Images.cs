using Chess;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;


namespace Chess
{
    public class Images
    {
        private Dictionary<string, BitmapImage> pieceImages = new Dictionary<string, BitmapImage>();

        public Images()
        {
            pieceImages["P"] = new BitmapImage(new Uri("pack://application:,,,/src/Assets/Images/PawnW.png"));
            pieceImages["R"] = new BitmapImage(new Uri("pack://application:,,,/src/Assets/Images/RookW.png"));
            pieceImages["N"] = new BitmapImage(new Uri("pack://application:,,,/src/Assets/Images/KnightW.png"));
            pieceImages["B"] = new BitmapImage(new Uri("pack://application:,,,/src/Assets/Images/BishopW.png"));
            pieceImages["Q"] = new BitmapImage(new Uri("pack://application:,,,/src/Assets/Images/QueenW.png"));
            pieceImages["K"] = new BitmapImage(new Uri("pack://application:,,,/src/Assets/Images/KingW.png"));

            pieceImages["p"] = new BitmapImage(new Uri("pack://application:,,,/src/Assets/Images/PawnB.png"));
            pieceImages["r"] = new BitmapImage(new Uri("pack://application:,,,/src/Assets/Images/RookB.png"));
            pieceImages["n"] = new BitmapImage(new Uri("pack://application:,,,/src/Assets/Images/KnightB.png"));
            pieceImages["b"] = new BitmapImage(new Uri("pack://application:,,,/src/Assets/Images/BishopB.png"));
            pieceImages["q"] = new BitmapImage(new Uri("pack://application:,,,/src/Assets/Images/QueenB.png"));
            pieceImages["k"] = new BitmapImage(new Uri("pack://application:,,,/src/Assets/Images/KingB.png"));
        }

        public BitmapImage GetPieceImage(string piece)
        {
            return pieceImages.ContainsKey(piece) ? pieceImages[piece] : null;
        }
    }   
}
