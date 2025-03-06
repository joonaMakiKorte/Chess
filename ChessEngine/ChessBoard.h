#ifndef CHESSBOARD_H
#define CHESSBOARD_H	

#include "pch.h"
#include "Board.h"

class ChessBoard {
private:
    // Store the board logic
    Board board;

public:
    // Initialize the chessboard
    ChessBoard();

    // Return bool indicating if move is valid
    // If valid, update board accordingly
    // Takes the move as const char pointer parameter
    bool ValidateMove(const char* move);

    // Return board state as a FEN string (Forsyth-Edwards Notation)
    // For starting position the FEN string would be: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    // Explanation:
    // 8 ranks, '/' separates the ranks
    // 'p'=pawn, 'r'=rook, 'n'=knight, 'b'=bishop, 'q'=queen, 'k'=king
    // Lowercase = black, uppercase = white
    // 
    // 'w' = active color
    // 'KQkq' = catling rights
    // '-' = En Passant Target Square
    // '0' = half move clock
    // '1' = full move number
    std::string GetBoardState();

private:
    // Converts a square to its bitboard representation
    // Takes a C-style string of the square as the param
    uint64_t SquareToBitboard(const char* square);
};


#endif // CHESSBOARD_H