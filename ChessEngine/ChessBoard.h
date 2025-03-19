#ifndef CHESSBOARD_H
#define CHESSBOARD_H	

#include "Bitboard.h"

class ChessBoard {
private:
    // Store the board logic
    Bitboard board; // Smart pointer

    // Keep track of player turns
    bool white;

    // Debug
    std::string debugMessage;

public:
    // Initialize the chessboard
    ChessBoard();

    // Get legal moves from the parameter square as a bitboard
    uint64_t LegalMoves(int square);

    // Move a chessboard piece according to given parameter
    // Update chessboard status accordingly
    // Is called in C# if validating move successful
    void MovePiece(int source, int target);

    // Return board state as a FEN string (Forsyth-Edwards Notation)
    // For starting position the FEN string would be: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 -
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
    // '-' = Game state (Check/checkmate/stalemate/normal)
    std::string GetBoardState();

    // Get debug message
    std::string GetDebugMessage() const;

private:
    // Set debug messages dynamically
    void UpdateDebugMessage(const std::string& message);

    // Print bitboard in algebraic notation for debug
    std::string printBitboardAsSquares(uint64_t bitboard);
};


#endif // CHESSBOARD_H