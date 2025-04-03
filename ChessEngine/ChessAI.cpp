#include "pch.h"
#include "ChessAI.h"
#include "Bitboard.h"
#include "Tables.h"

// Initialize static members
uint64_t ChessAI::nodes_evaluated = 0;

uint32_t ChessAI::getBestMove(Bitboard& board, int depth, std::string& benchmark) {
    nodes_evaluated = 0; // Reset node count for benchmarking
    auto start = std::chrono::high_resolution_clock::now();

    std::array<uint32_t, MAX_MOVES> move_list;
    int move_count = 0;
    board.generateMoves(move_list, move_count, 0, false); // Generate all legal moves for Black (AI player)

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

uint32_t ChessAI::getBestEndgameMove(Bitboard& board, int depth, std::string& benchmark) {
    nodes_evaluated = 0; // Reset node count for benchmarking
    auto start = std::chrono::high_resolution_clock::now();

    std::array<uint32_t, MAX_MOVES> move_list;
    int move_count = 0;
    board.generateEndgameMoves(move_list, move_count, 0, false); // Generate all legal moves for Black (AI player)

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
        int score = endgameMinimax(board, depth - 1, -INF, INF, true);

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
    // Check for terminal conditions (mate/draw)
	if (board.isGameOver()) {
        return evaluateBoard(board, depth, maximizingPlayer);
	}

    // Quiescence search at depth 0 (with checks)
    if (depth <= 0) {
        return quiescenceSearch(board, alpha, beta, maximizingPlayer);
    }

    std::array<uint32_t, MAX_MOVES> move_list;
	int move_count = 0;

    board.generateMoves(move_list, move_count, depth, maximizingPlayer);

    if (maximizingPlayer) { // AI (Black) tries to maximize
        int maxEval = -INF;
        for (int i = 0; i < move_count; i++) {
            board.applyMoveAI(move_list[i], maximizingPlayer);

            // Prune out branches that result in draw
            if (board.state.isDraw()) {
                board.undoMoveAI(move_list[i], maximizingPlayer);
                return 0; 
            }

            int eval = minimax(board, depth - 1, alpha, beta, !maximizingPlayer);
            board.undoMoveAI(move_list[i], maximizingPlayer);

			maxEval = max(maxEval, eval);
            if (eval > alpha) {
                alpha = eval;
                // Update history heuristic for improving moves
                if (!isCapture(move_list[i])) {
                    updateHistory(move_list[i], depth);
                }
            }

            // Beta cutoff: store killer move (not in endgame)
            if (beta <= alpha) {
                if (!isCapture(move_list[i])) {
                    updateKillerMoves(move_list[i], depth);
                }
                break; // Prune remaining branches
            }
        }
        return maxEval;
    }
    else { // Opponent (White) tries to minimize
        int minEval = INF;
        for (int i = 0; i < move_count; i++) {
            board.applyMoveAI(move_list[i], maximizingPlayer);

            // Prune out branches that result in draw
            if (board.state.isDraw()) {
                board.undoMoveAI(move_list[i], maximizingPlayer);
                return 0;
            }

            int eval = minimax(board, depth - 1, alpha, beta, !maximizingPlayer);
            board.undoMoveAI(move_list[i], maximizingPlayer);

            minEval = min(minEval, eval);
            if (eval < beta) {
                beta = eval;
                // Update history heuristic for improving moves
                if (!isCapture(move_list[i])) {
                    updateHistory(move_list[i], depth);
                }
            }
            
            // Beta cutoff: store killer move (not in endgame
            if (beta <= alpha) {
                if (!isCapture(move_list[i])) {
                    updateKillerMoves(move_list[i], depth);
                }
                break; // Prune remaining branches
            }
        }
        return minEval;
    }
}

int ChessAI::quiescenceSearch(Bitboard& board, int alpha, int beta, bool maximizingPlayer) {
    int eval = evaluateBoard(board, 0, maximizingPlayer);  // Get a static evaluation of the current position

    // Stand pat: if this position is already better than beta, cut off search (pruning)
    if (eval >= beta) return beta;
    if (eval > alpha) alpha = eval;  // Update alpha if we find a better move

    // Generate captures + promotions (non quiet moves)
    std::array<uint32_t, MAX_MOVES> move_list;
    int move_count = 0;
    board.generateNoisyMoves(move_list, move_count, maximizingPlayer);

    for (int i = 0; i < move_count; i++) {
        int move_value = board.estimateCaptureValue(move_list[i]);

        // Delta pruning - skip moves that can't possibly raise alpha
        if (eval + move_value + DELTA_MARGIN <= alpha) {
            continue; // Skip this move as it can't improve alpha
        }
        board.applyMoveAI(move_list[i], maximizingPlayer);

        int score = -quiescenceSearch(board, -beta, -alpha, !maximizingPlayer);  // Negamax approach

        board.undoMoveAI(move_list[i], maximizingPlayer);

        if (score >= beta) return beta;  // Beta cutoff
        if (score > alpha) alpha = score;  // Improve alpha
    }

    return alpha;  // Best evaluation we found
}

int ChessAI::evaluateBoard(Bitboard& board, int depth, bool maximizingPlayer) {
    nodes_evaluated++; // Track evaluations

    if (board.state.isCheckmateWhite()) return -100000 + (depth * 1000);  // White loses
	if (board.state.isCheckmateBlack()) return 100000 - (depth * 1000); // Black loses
    if (board.state.isStalemate() || board.state.isDraw()) return 0; // Draw -> neutral outcome

    // Evaluate material and positional score of the board
    int score = board.evaluateBoard(maximizingPlayer);

	// King safety (mobility and attacks)
	int white_king_mobility = board.calculateKingMobility(true);
	int black_king_mobility = board.calculateKingMobility(false);
	score += (white_king_mobility - black_king_mobility) * 10; // Mobility bonus

    // Encourage attacking the opponent’s king
    if (board.state.isCheckWhite()) score -= 50; // White in check
    if (board.state.isCheckBlack()) score += 50; // Black in check

    // Return the score
    return score;
}

int ChessAI::endgameMinimax(Bitboard& board, int depth, int alpha, int beta, bool maximizingPlayer) {
    // Check for terminal conditions (mate/draw)
    if (board.isGameOver()) {
        return evaluateBoard(board, depth, maximizingPlayer);
    }

    // Check extension: Extend if current player is in check
    if (maximizingPlayer ? board.state.isCheckBlack() : board.state.isCheckWhite()) {
        depth += 1; // Standard extension
    }

    // Quiescence search at depth 0 (with checks)
    if (depth <= 0) {
        return quiescenceSearch(board, alpha, beta, maximizingPlayer);
    }

    std::array<uint32_t, MAX_MOVES> move_list;
    int move_count = 0;

    board.generateEndgameMoves(move_list, move_count, depth, maximizingPlayer);

    if (maximizingPlayer) { // AI (Black) tries to maximize
        int maxEval = -INF;
        for (int i = 0; i < move_count; i++) {
            board.applyMoveAI(move_list[i], maximizingPlayer);

            // Prune out branches that result in draw
            if (board.state.isDraw()) {
                board.undoMoveAI(move_list[i], maximizingPlayer);
                return 0;
            }

            int eval = minimax(board, depth - 1, alpha, beta, !maximizingPlayer);
            board.undoMoveAI(move_list[i], maximizingPlayer);

            maxEval = max(maxEval, eval);
            if (eval > alpha) {
                alpha = eval;
                // Update history heuristic for improving moves
                if (!isCapture(move_list[i])) {
                    updateHistory(move_list[i], depth);
                }
            }

            // Beta cutoff: store killer move (not in endgame)
            if (beta <= alpha) {
                if (!isCapture(move_list[i])) {
                    updateKillerMoves(move_list[i], depth);
                }
                break; // Prune remaining branches
            }
        }
        return maxEval;
    }
    else { // Opponent (White) tries to minimize
        int minEval = INF;
        for (int i = 0; i < move_count; i++) {
            board.applyMoveAI(move_list[i], maximizingPlayer);

            // Prune out branches that result in draw
            if (board.state.isDraw()) {
                board.undoMoveAI(move_list[i], maximizingPlayer);
                return 0;
            }

            int eval = minimax(board, depth - 1, alpha, beta, !maximizingPlayer);
            board.undoMoveAI(move_list[i], maximizingPlayer);

            minEval = min(minEval, eval);
            if (eval < beta) {
                beta = eval;
                // Update history heuristic for improving moves
                if (!isCapture(move_list[i])) {
                    updateHistory(move_list[i], depth);
                }
            }

            // Beta cutoff: store killer move (not in endgame
            if (beta <= alpha) {
                if (!isCapture(move_list[i])) {
                    updateKillerMoves(move_list[i], depth);
                }
                break; // Prune remaining branches
            }
        }
        return minEval;
    }
}

inline bool ChessAI::isCapture(uint32_t move) {
    return capturedPiece(move) != EMPTY || moveType(move) == EN_PASSANT;
}

uint16_t ChessAI::moveKey(uint32_t move) {
    int from = move & 0x3F;              // Extract from square (6 bits)
    int to = (move >> 6) & 0x3F;         // Extract to square (6 bits)
    int piece = ((move >> 12) & 0xF);    // Extract piece type (4 bits)

    return (from << 10) | (to << 4) | (piece << 0);
}

uint16_t ChessAI::moveKey(int from, int to, PieceType piece) {
    return (from << 10) | (to << 4) | (piece << 0);

}

void ChessAI::updateKillerMoves(uint32_t move, int depth) {
    uint16_t key = moveKey(move); // Generate key

    if (key != Tables::KILLER_MOVES[depth][0]) {
        Tables::KILLER_MOVES[depth][1] = Tables::KILLER_MOVES[depth][0]; // Shift old move
        Tables::KILLER_MOVES[depth][0] = key; // Store new move
    }
}

void ChessAI::updateHistory(uint32_t move, int depth) {
    uint16_t key = moveKey(move); // Generate key
    Tables::HISTORY_TABLE[key] += depth * depth; // Higher weight for deeper cutoffs
}

bool ChessAI::isKillerMove(int from, int to, PieceType piece, int depth) {
    uint16_t key = moveKey(from, to, piece); // Get key
    return key == Tables::KILLER_MOVES[depth][0] || key == Tables::KILLER_MOVES[depth][1];
}

int ChessAI::getHistoryScore(int from, int to, PieceType piece) {
    uint16_t key = moveKey(from, to, piece); // Get key
    return Tables::HISTORY_TABLE[key];
}
