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

    // Get legal moves from the parameter square as a bitboard
    uint64_t LegalMoves(int square);

    bool isInCheck();

    bool isCheckmate();

    // Move a chessboard piece according to given parameter
    // Update chessboard status accordingly
    // Is called in C# if validating move successful
    void MovePiece(int source, int target);

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



 

    // Get debug message
    std::string GetDebugMessage() const;

private:
    // Set debug messages dynamically
    void UpdateDebugMessage(const std::string& message);
};


#endif // CHESSBOARD_H