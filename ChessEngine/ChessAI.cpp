#include "pch.h"
#include "ChessAI.h"
#include "Bitboard.h"

uint32_t ChessAI::getBestMove(Bitboard& board, int depth) {
	std::array<uint32_t, MAX_MOVES> move_list;
	int move_count = 0;
	board.generateMoves(move_list, move_count, false); // Generate all legal moves for Black (AI player)

    if (move_count == 0) {
        return 0; // No legal moves available
    }

    int bestScore = -INF;
    uint32_t bestMove = 0;

    for (int i = 0; i < move_count; i++) {
        // Apply the move
        board.applyMoveAI(move_list[i], false);

        // Call minimax (assuming AI plays as Black)
        int score = minimax(board, depth - 1, -INF, INF, false);

        // Undo move
        board.undoMoveAI(move_list[i], false);

        // Check if the move is better
        if (score >= bestScore) {
            bestScore = score;
            bestMove = move_list[i];
        }
    }

    return bestMove;
}

int ChessAI::minimax(Bitboard& board, int depth, int alpha, int beta, bool maximizingPlayer) {
	if (depth == 0 || board.isGameOver(true) || board.isGameOver(false)) {
		return board.evaluateBoard(maximizingPlayer); // Evaluate the board
	}

    std::array<uint32_t, MAX_MOVES> move_list;
	int move_count = 0;
	board.generateMoves(move_list, move_count, maximizingPlayer);

    if (maximizingPlayer) { // AI (Black) tries to maximize
        int maxEval = -INF;
        for (int i = 0; i < move_count; i++) {
            board.applyMoveAI(move_list[i], maximizingPlayer);
            int eval = minimax(board, depth - 1, alpha, beta, !maximizingPlayer);
            board.undoMoveAI(move_list[i], maximizingPlayer);
			maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);
            if (beta <= alpha) break; // Alpha-beta pruning
        }
        return maxEval;
    }
    else { // Opponent (White) tries to minimize
        int minEval = INF;
        for (int i = 0; i < move_count; i++) {
            board.applyMoveAI(move_list[i], maximizingPlayer);
            int eval = minimax(board, depth - 1, alpha, beta, !maximizingPlayer);
            board.undoMoveAI(move_list[i], maximizingPlayer);
            minEval = min(minEval, eval);
            beta = min(beta, eval);
            if (beta <= alpha) break; // Alpha-beta pruning
        }
        return minEval;
    }
}

std::string ChessAI::getBestMoveString(Bitboard& board, int depth) {
    // Generate the best move
	uint32_t bestMove = getBestMove(board, depth);

	// Turn the move into algebraic notation
	int from = ChessAI::from(bestMove);
	int to = ChessAI::to(bestMove);

	std::string from_square = board.squareToString(from);
	std::string to_square = board.squareToString(to);

	return from_square + to_square;
}
