#ifndef CHESSAI_H
#define CHESSAI_H

#include "BitboardConstants.h"

// Forward declaration of Bitboard class
class Bitboard;

/*
The ChessAI structures a move in 32 bits:

0000 0000 0000 0000 0000 0000 0011 1111  -> from (6 bits)
0000 0000 0000 0000 0000 1111 1100 0000  -> to (6 bits)
0000 0000 0000 0000 1111 0000 0000 0000  -> piece type (4 bits)
0000 0000 0000 1111 0000 0000 0000 0000  -> captured piece type (4 bits)
0000 0000 1111 0000 0000 0000 0000 0000  -> move type (4 bits)
0000 1111 0000 0000 0000 0000 0000 0000  -> promotion type (4 bits)
0001 0000 0000 0000 0000 0000 0000 0000  -> special flags (1 bit for en passant)

*/

class ChessAI {
public:
    // Pack a move into an integer
	// Must be static since used in Bitboard.cpp without an instance
    static uint32_t encodeMove(int from, int to, PieceType piece, PieceType captured, MoveType type, PieceType promotion, bool enPassant) {
        return (from & 0x3F) |
            ((to & 0x3F) << 6) |
            ((piece & 0xF) << 12) |
            ((captured & 0xF) << 16) |
            ((type & 0xF) << 20) |
            ((promotion & 0xF) << 24) |
            ((enPassant ? 1 : 0) << 28);
    }

    // Extract data from a packed move
    static int from(uint32_t move) { return move & 0x3F; }
    static int to(uint32_t move) { return (move >> 6) & 0x3F; }
    static PieceType piece(uint32_t move) { return static_cast<PieceType>((move >> 12) & 0xF); }
    static PieceType capturedPiece(uint32_t move) { return static_cast<PieceType>((move >> 16) & 0xF); }
    static MoveType moveType(uint32_t move) { return static_cast<MoveType>((move >> 20) & 0xF); }
    static PieceType promotion(uint32_t move) { return static_cast<PieceType>((move >> 24) & 0xF); }
    static bool isEnPassant(uint32_t move) { return (move >> 28) & 1; }

private:
	// Minimax algorithm with alpha-beta pruning
	// Recursively evaluates the board by simulating moves and choosing the best one
	// Alpha-beta pruning is used to reduce the number of nodes evaluated in the search tree
    static int minimax(Bitboard& board, int depth, int alpha, int beta, bool maximizingPlayer);

	// Helper evaluation function for the minimax algorithm
	// Detect checkmate, stalemate, and evaluate the board based on material and positional advantages
	// Advantegeous positions are assigned higher scores for prioritization
	static int evaluateBoard(Bitboard& board, int depth, bool maximizingPlayer);

public:
    // Get the best move for the current board state
    static uint32_t getBestMove(Bitboard& board, int depth);
};

#endif // !CHESSAI_H
