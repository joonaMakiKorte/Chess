#ifndef CHESSBOARD_H
#define CHESSBOARD_H	

#include "Bitboard.h"

class ChessBoard {
private:
    // Store the board logic
    std::unique_ptr<Bitboard> board; // Smart pointer

    std::string debugMessage;

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

    // Set debug messages dynamically
    void UpdateDebugMessage(const std::string& message);

    // Get debug message
    std::string GetDebugMessage() const;

private:
    // Converts a square to its integer representation
    // Takes a C-style string of the square as the param
    int SquareToInt(const char* square);
};


#endif // CHESSBOARD_H