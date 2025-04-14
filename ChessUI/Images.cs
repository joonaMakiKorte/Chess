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
            pieceImages["P"] = new BitmapImage(new Uri("pics/PawnW.png", UriKind.Relative));
            pieceImages["R"] = new BitmapImage(new Uri("pics/RookW.png", UriKind.Relative));
            pieceImages["N"] = new BitmapImage(new Uri("pics/KnightW.png", UriKind.Relative));
            pieceImages["B"] = new BitmapImage(new Uri("pics/BishopW.png", UriKind.Relative));
            pieceImages["Q"] = new BitmapImage(new Uri("pics/QueenW.png", UriKind.Relative));
            pieceImages["K"] = new BitmapImage(new Uri("pics/KingW.png", UriKind.Relative));

            pieceImages["p"] = new BitmapImage(new Uri("pics/PawnB.png", UriKind.Relative));
            pieceImages["r"] = new BitmapImage(new Uri("pics/RookB.png", UriKind.Relative));
            pieceImages["n"] = new BitmapImage(new Uri("pics/KnightB.png", UriKind.Relative));
            pieceImages["b"] = new BitmapImage(new Uri("pics/BishopB.png", UriKind.Relative));
            pieceImages["q"] = new BitmapImage(new Uri("pics/QueenB.png", UriKind.Relative));
            pieceImages["k"] = new BitmapImage(new Uri("pics/KingB.png", UriKind.Relative));
        }

        public BitmapImage GetPieceImage(string piece)
        {
            return pieceImages.ContainsKey(piece) ? pieceImages[piece] : null;
        }
    }   
}
