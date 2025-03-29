#include "pch.h"
#include "ChessAI.h"
#include "Bitboard.h"

uint64_t ChessAI::nodes_evaluated = 0; // Init

uint32_t ChessAI::getBestMove(Bitboard& board, int depth, std::string& benchmark) {
    nodes_evaluated = 0;

    auto start = std::chrono::high_resolution_clock::now();

    std::array<uint32_t, MAX_MOVES> move_list;
    int move_count = 0;
    board.generateMoves(move_list, move_count, false); // Generate all legal moves for Black (AI player)

    if (move_count == 0) {
        return 0; // No legal moves available
    }

    int bestScore = INF; // Black wants to minimize White's evaluation
    uint32_t bestMove = 0;

    board.resetUndoStack(); // Reset undo stack before new search
    for (int i = 0; i < move_count; i++) {
        // Apply the move
        board.applyMoveAI(move_list[i], false);

        // Call minimax (assuming AI plays as Black)
        // Call with maximizingPlayer = true since AI wants to minimize White's score
        int score = minimax(board, depth - 1, -INF, INF, true);

        // Undo move
        board.undoMoveAI(move_list[i], false);

        // Black wants to minimize White's score
        if (score < bestScore) {
            bestScore = score;
            bestMove = move_list[i];
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end - start).count(); // Evaluation time
    benchmark = "Depth: " + std::to_string(depth) + " | Nodes: " + std::to_string(nodes_evaluated)
        + " | Time: " + std::to_string(duration) + "s"
        + " | Nodes/sec: " + std::to_string(nodes_evaluated / duration);

    return bestMove;
}

int ChessAI::minimax(Bitboard& board, int depth, int alpha, int beta, bool maximizingPlayer) {
	if (depth == 0 || board.isGameOver()) {
		return quiescenceSearch(board, alpha, beta, maximizingPlayer);
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

int ChessAI::quiescenceSearch(Bitboard& board, int alpha, int beta, bool maximizingPlayer) {
    const int DELTA_MARGIN = 900; // Value of a queen
    int eval = evaluateBoard(board, 0, maximizingPlayer);  // Get a static evaluation of the current position

    // Stand pat: if this position is already better than beta, cut off search (pruning)
    if (eval >= beta) return beta;
    if (eval > alpha) alpha = eval;  // Update alpha if we find a better move

    // Generate only capture moves (no quiet moves)
    std::array<uint32_t, MAX_MOVES> capture_list;
    int capture_count = 0;
    board.generateNoisyMoves(capture_list, capture_count, maximizingPlayer);

    for (int i = 0; i < capture_count; i++) {
        // Delta pruning - skip moves that can't possibly raise alpha
        int move_value = board.estimateCaptureValue(capture_list[i]);
        if (eval + move_value + DELTA_MARGIN <= alpha) {
            continue; // Skip this move as it can't improve alpha
        }
        board.applyMoveAI(capture_list[i], maximizingPlayer);

        int score = -quiescenceSearch(board, -beta, -alpha, !maximizingPlayer);  // Negamax approach

        board.undoMoveAI(capture_list[i], maximizingPlayer);

        if (score >= beta) return beta;  // Beta cutoff
        if (score > alpha) alpha = score;  // Improve alpha
    }

    return alpha;  // Best evaluation we found
}

int ChessAI::evaluateBoard(Bitboard& board, int depth, bool maximizingPlayer) {
    nodes_evaluated++; // Track evaluations

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