#ifndef CHESSAI_H
#define CHESSAI_H

#include "BitboardConstants.hpp"
#include "CustomTypes.hpp"

// Forward declaration of Bitboard class
class Bitboard;

/*
The ChessAI structures a move in 32 bits:

/*
The ChessAI structures a move in 32 bits:

0000 0000 0000 0000 0000 0000 0011 1111  -> from (6 bits)
0000 0000 0000 0000 0000 1111 1100 0000  -> to (6 bits)
0000 0000 0000 0000 1111 0000 0000 0000  -> piece type (4 bits)
0000 0000 0000 1111 0000 0000 0000 0000  -> captured piece type (4 bits)
0000 0000 1111 0000 0000 0000 0000 0000  -> move type (4 bits)
0000 1111 0000 0000 0000 0000 0000 0000  -> promotion type (4 bits)
0001 0000 0000 0000 0000 0000 0000 0000  -> check move (1 bit) (only considered in endgame)

*/

class ChessAI {
private:
    static uint64_t nodes_evaluated;

public:
    // Pack a move into an integer
	// Must be static since used in Bitboard.cpp without an instance
    static uint32_t encodeMove(int from, int to, PieceType piece, PieceType captured, MoveType type, PieceType promotion, bool check) {
        return (from & 0x3F) |
            ((to & 0x3F) << 6) |
            ((piece & 0xF) << 12) |
            ((captured & 0xF) << 16) |
            ((type & 0xF) << 20) |
            ((promotion & 0xF) << 24) |
            ((check & 0x1) << 28);
    }

    // Extract data from a packed move
    static int from(uint32_t move) { return move & 0x3F; }
    static int to(uint32_t move) { return (move >> 6) & 0x3F; }
    static PieceType piece(uint32_t move) { return static_cast<PieceType>((move >> 12) & 0xF); }
    static PieceType capturedPiece(uint32_t move) { return static_cast<PieceType>((move >> 16) & 0xF); }
    static MoveType moveType(uint32_t move) { return static_cast<MoveType>((move >> 20) & 0xF); }
    static PieceType promotion(uint32_t move) { return static_cast<PieceType>((move >> 24) & 0xF); }
    static bool isCheck(uint32_t move) { return (move >> 28) & 0x1; }

    static bool isKillerMove(int from, int to, PieceType piece, int depth); // Check if move is a killer move by depth
    static int getHistoryScore(int from, int to, PieceType piece); // Get history score of a move

private:
    /*************************
    Midgame specific algorithm
    *************************/

	// Minimax algorithm with alpha-beta pruning
	// Recursively evaluates the board by simulating moves and choosing the best one
	// Alpha-beta pruning is used to reduce the number of nodes evaluated in the search tree
    static int minimax(Bitboard& board, int depth, int alpha, int beta, bool maximizingPlayer);

	// Quiescence search algorithm
	// Searches for the best move in a noisy position (captures and promotions)
	// Reduces the horizon effect by searching deeper in capturing positions
	static int quiescence(Bitboard& board, int alpha, int beta, bool maximizingPlayer);

	// Get evaluation of the current board score
	// Detect checkmate, stalemate, and evaluate the board based on material and positional advantages
	// Advantegeous positions are assigned higher scores for prioritization
	static int evaluateBoard(Bitboard& board, int depth, bool maximizingPlayer);


    /*************************
    Endgame specific algorithm
    *************************/

    // Minimax algorithm with alpha-beta pruning
    // Differs from midgame by extending search for moves that chech the opponent
    // Also moves are sorted with different heuristic
    static int endgameMinimax(Bitboard& board, int depth, int alpha, int beta, bool maximizingPlayer);

    // Quiescence search algorithm
    // Searches for the best move in a noisy position (captures and promotions)
    // Reduces the horizon effect by searching deeper in capturing positions
    static int endgameQuiescence(Bitboard& board, int alpha, int beta, bool maximizingPlayer);

    // Get evaluation of the current board score
    // Detect checkmate, stalemate, and evaluate the board based on material and positional advantages
    // Advantegeous positions are assigned higher scores for prioritization
    static int evaluateEndgameBoard(Bitboard& board, int depth, bool maximizingPlayer);

private: 
    // Helper to determine if a move is capture
    static inline bool isCapture(uint32_t move);

    // Helper to determine if promotion
    static inline bool isPromotion(uint32_t move);

    // Extract a compressed key derived from the move structure for killer-move/history heuristics hashing
    // 16-bit key, efficient and avoids collisions
    static uint16_t moveKey(uint32_t move);
    static uint16_t moveKey(int from, int to, PieceType piece); // Overloaded to generate move without decoding move

    // Update killer moves and history heuristics
    static void updateKillerMoves(uint32_t move, int depth);
    static void updateHistory(uint32_t move, int depth);

public:
    // Get the best move for the current board state in midgame
    static uint32_t getBestMove(Bitboard& board, int depth, std::string& benchmark);

    // Get the best move for the current board state in endgame
    static uint32_t getBestEndgameMove(Bitboard& board, int depth, std::string& benchmark);
};

#endif // !CHESSAI_H
