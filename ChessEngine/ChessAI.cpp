#include "pch.h"
#include "ChessAI.h"

uint32_t ChessAI::getBestMove(Bitboard& board, int depth) {
	std::array<uint32_t, MAX_MOVES> moveList;
	int move_count = 0;
	board.generateMoves(moveList, move_count, false); // Generate all legal moves for Black (AI player)

    if (move_count == 0) {
        return 0; // No legal moves available
    }

    int bestScore = -INFINITY;
    uint32_t bestMove = 0;

    for (int i = 0; i < move_count; i++) {
        // Apply the move
        board.applyMoveAI(moveList[i], false);

        // Call minimax (assuming AI plays as Black)
        int score = minimax(board, depth - 1, -INFINITY, INFINITY, false);

        // Undo move
        board.undoMoveAI(moveList[i], false);

        // Check if the move is better
        if (score > bestScore) {
            bestScore = score;
            bestMove = moveList[i];
        }
    }

    return bestMove;
}
