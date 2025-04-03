#ifndef CHESSBOARD_H
#define CHESSBOARD_H	

#include "Bitboard.h"

class ChessBoard {
private:
    // Store the board logic
    Bitboard board; // Smart pointer

    // Keep track of player turns
    bool white;

    // Flag to track game state
    // Once endgame, calls different AI function for getting best move
    bool isEndgame;

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

    // Get best move for black pieces and apply it
    // Determined by search depth
    void MakeMoveAI(int depth);

	// Promote the pawn at target square to a piece of choice
	void MakePromotion(int target, char promotion);

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