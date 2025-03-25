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

    int bestScore = INF; // Black wants to minimize White's evaluation
    uint32_t bestMove = 0;

    for (int i = 0; i < move_count; i++) {
        // Save en passant and castling rights for undoing
        uint8_t castling_rights = 0;
        int en_passant_target = UNASSIGNED;

		BoardState state = board.state; // Save the state before applying the move

        // Apply the move
        board.applyMoveAI(move_list[i], false, castling_rights, en_passant_target);

        // Call minimax (assuming AI plays as Black)
		// Call with maximizingPlayer = true since AI wants to minimize White's score
        int score = minimax(board, depth - 1, -INF, INF, true);

        // Undo move
        board.undoMoveAI(move_list[i], false, castling_rights, en_passant_target);
		board.state = state; // Restore the state

		// Black wants to minimize White's score
        if (score < bestScore) {
            bestScore = score;
            bestMove = move_list[i];
        }
    }

    return bestMove;
}

int ChessAI::minimax(Bitboard& board, int depth, int alpha, int beta, bool maximizingPlayer) {
	if (depth == 0 || board.isGameOver()) {
		return evaluateBoard(board, depth, maximizingPlayer); // Evaluate the board
	}

    std::array<uint32_t, MAX_MOVES> move_list;
	int move_count = 0;
	board.generateMoves(move_list, move_count, maximizingPlayer);

	BoardState state = board.state; // Save the state before applying the move

    if (maximizingPlayer) { // AI (Black) tries to maximize
        int maxEval = -INF;
        for (int i = 0; i < move_count; i++) {
            // Save en passant and castling rights for undoing
            uint8_t castling_rights = 0;
            int en_passant_target = UNASSIGNED;

            board.applyMoveAI(move_list[i], maximizingPlayer, castling_rights, en_passant_target);

            int eval = minimax(board, depth - 1, alpha, beta, !maximizingPlayer);

            board.undoMoveAI(move_list[i], maximizingPlayer, castling_rights, en_passant_target);
			board.state = state; // Restore the state

			maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);
            if (beta <= alpha) break; // Alpha-beta pruning
        }
        return maxEval;
    }
    else { // Opponent (White) tries to minimize
        int minEval = INF;
        for (int i = 0; i < move_count; i++) {
            // Save en passant and castling rights for undoing
            uint8_t castling_rights = 0;
            int en_passant_target = UNASSIGNED;

            board.applyMoveAI(move_list[i], maximizingPlayer, castling_rights, en_passant_target);

            int eval = minimax(board, depth - 1, alpha, beta, !maximizingPlayer);

            board.undoMoveAI(move_list[i], maximizingPlayer, castling_rights, en_passant_target);
			board.state = state; // Restore the state

            minEval = min(minEval, eval);
            beta = min(beta, eval);
            if (beta <= alpha) break; // Alpha-beta pruning
        }
        return minEval;
    }
}

int ChessAI::evaluateBoard(Bitboard& board, int depth, bool maximizingPlayer) {
    if (board.state.isCheckmateWhite()) return -100000 + (depth * 1000);  // White loses
	if (board.state.isCheckmateBlack()) return 100000 - (depth * 1000); // Black loses
    if (board.state.isStalemate()) return 0; // Draw -> neutral outcome

    // Evaluate material and positional score of the board
    int score = board.evaluateBoard(maximizingPlayer);

	// King safety (mobility and attacks)
	int white_king_mobility = board.calculateKingMobility(true);
	int black_king_mobility = board.calculateKingMobility(false);
	score += (white_king_mobility - black_king_mobility) * 10; // Mobility bonus

    // Encourage attacking the opponent’s king
    if (board.state.isCheckWhite()) score -= 30; // White in check
    if (board.state.isCheckBlack()) score += 30; // Black in check

    // Return the score
	// No need for additional negation since already done in board.evaluateBoard()
    return score;
}