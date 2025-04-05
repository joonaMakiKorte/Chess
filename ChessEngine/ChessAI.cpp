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
    board.generateMoves(move_list, move_count, 0, false, NULL_MOVE_32); // Generate all legal moves for Black (AI player)

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
    // Lookup the current position in the transposition table using the incremental Zobrist hash key
    uint64_t key = board.hash_key;
    uint32_t tt_best_move = NULL_MOVE_32; // Hint for move ordering
    int original_alpha = alpha; // Store original alpha for TT store logic
    int original_beta = beta; // Store original beta as well for clarity

    // --- TT Probe ---
    // Check if TT is initialized and probe only if TT_NUM_ENTRIES > 0
    if (Tables::TT_NUM_ENTRIES > 0) {
        size_t index = key & Tables::TT_MASK;
        TTEntry& entry = Tables::TRANSPOSITION_TABLE[index];

        // Verify hash key
        if (entry.zobrist_key_verify == key) {
            // Entry matches the current position
            tt_best_move = entry.best_move; // Store best move hint for move ordering

            // Check if depth is sufficient for reuse
            if (entry.depth >= depth) {
                int stored_score = entry.score; // Score is stored relative to the side to move

                // Use stored information based on the flag
                if (entry.flag == FLAG_EXACT) {
                    return stored_score; // Exact score found
                }
                if (entry.flag == FLAG_LOWERBOUND) { // Failed high previously (score >= beta)
                    if (stored_score >= beta) {
                        // This stored lower bound is sufficient to cause a beta cutoff now
                        if (!isCapture(tt_best_move) && tt_best_move != NULL_MOVE_32) {
                            updateKillerMoves(tt_best_move, depth); // Update based on TT cutoff
                        }
                        return stored_score;
                    }
                    // If it doesn't cause cutoff, we can still tighten alpha
                    alpha = max(alpha, stored_score);
                }
                else if (entry.flag == FLAG_UPPERBOUND) { // Failed low previously (score <= alpha)
                    if (stored_score <= alpha) {
                        // This stored upper bound is sufficient to cause an alpha cutoff now
                        if (!isCapture(tt_best_move) && tt_best_move != NULL_MOVE_32) {
                             updateKillerMoves(tt_best_move, depth); // Update based on TT cutoff
                        }
                        return stored_score;
                    }
                    // If it doesn't cause cutoff, we can still tighten beta
                    beta = min(beta, stored_score);
                }

                // Check if bounds crossed after tightening
                if (alpha >= beta) {
                    // A cutoff occurred due to TT bounds tightening. Return the score that caused it.
                    // The logic above should already return in these cases, but as a fallback:
                    // If alpha was raised by LOWERBOUND, return alpha. If beta was lowered by UPPERBOUND, return beta.
                    // Returning the bound that was *not* crossed seems appropriate for fail-soft.
                    // If FLAG_LOWERBOUND caused alpha >= beta, return alpha.
                    // If FLAG_UPPERBOUND caused beta <= alpha, return beta.
                    // Let's return alpha as it represents a score we know we can achieve (lower bound).
                    return alpha;
                }
            }

        }
    }
    
    // Check for terminal conditions after TT probe (mate/draw)
	if (board.isGameOver()) {
        return evaluateBoard(board, depth, maximizingPlayer);
	}

    // Quiescence search at depth 0 (with checks)
    if (depth <= 0) {
        // Results from Q-search are generally not stored in the main TT here,
        // though Q-search itself could potentially use the TT or its own smaller cache
        return quiescence(board, alpha, beta, maximizingPlayer);
    }

    // --- Main search logic ---
    std::array<uint32_t, MAX_MOVES> move_list;
	int move_count = 0;
    board.generateMoves(move_list, move_count, depth, maximizingPlayer, tt_best_move); // Passing best move from tt to movegen

    // Check if no moves were generated (safeguard)
    if (move_count == 0) {
        return evaluateBoard(board, depth, maximizingPlayer);
    }

    uint32_t best_move_found = NULL_MOVE_32; // Track best move at this ply
    TTFlag flag = FLAG_UPPERBOUND; // Assume fail-low initially (score <= original_alpha)
    int best_eval; // Holds the best score found

    if (maximizingPlayer) { // AI (Black) tries to maximize
        best_eval = -INF;
        for (int i = 0; i < move_count; i++) {
            board.applyMoveAI(move_list[i], maximizingPlayer);

            int eval;
            // Check child node for draw *before* recursive call
            if (board.state.isDraw()) {
                eval = 0; // Assign draw score
            }
            else {
                eval = minimax(board, depth - 1, alpha, beta, !maximizingPlayer);
            }

            board.undoMoveAI(move_list[i], maximizingPlayer);

			// --- Update best score and alpha ---
            if (eval > best_eval) {
                best_eval = eval;
                best_move_found = move_list[i]; // Found a better move
                if (best_eval > alpha) {
                    alpha = best_eval;
                    flag = FLAG_EXACT; // Score is now within the original alpha-beta window
                    // Update history heuristic for improving moves
                    if (!isCapture(move_list[i])) {
                        updateHistory(move_list[i], depth);
                    }
                }
            }
            

            // --- Beta cutoff check ---
            if (alpha >= beta) {
                // Store killer move *before* storing TT entry
                if (!isCapture(move_list[i])) {
                    updateKillerMoves(move_list[i], depth);
                }
                flag = FLAG_LOWERBOUND; // Failed high (score >= beta)
                best_eval = beta; // Return the cutoff bound score (fail-soft)

                // --- TT Store on Cutoff ---
                if (Tables::TT_NUM_ENTRIES > 0) {
                    size_t index = key & Tables::TT_MASK;
                    // Depth-preferred replacement (or always replace)
                    if (Tables::TRANSPOSITION_TABLE[index].depth <= depth) {
                        Tables::TRANSPOSITION_TABLE[index].zobrist_key_verify = key;
                        Tables::TRANSPOSITION_TABLE[index].score = (int16_t)best_eval;
                        Tables::TRANSPOSITION_TABLE[index].depth = (int8_t)depth;
                        Tables::TRANSPOSITION_TABLE[index].flag = flag;
                        Tables::TRANSPOSITION_TABLE[index].best_move = move_list[i]; // Store the move causing cutoff
                    }
                }
                return best_eval; // Prune
            }
        }
    }
    else { // Opponent (White) tries to minimize
        best_eval = INF; // Initialize appropriately for minimization
        for (int i = 0; i < move_count; i++) {
            board.applyMoveAI(move_list[i], maximizingPlayer);

            int eval;
            if (board.state.isDraw()) {
                eval = 0;
            }
            else {
                eval = minimax(board, depth - 1, alpha, beta, !maximizingPlayer);
            }

            board.undoMoveAI(move_list[i], maximizingPlayer);

            // --- Update best score and beta ---
            if (eval < best_eval) {
                best_eval = eval;
                best_move_found = move_list[i]; // Found a better move
                if (best_eval < beta) {
                    beta = best_eval;
                    flag = FLAG_EXACT; // Score is now within the original alpha-beta window
                    // Update history heuristic for this new best move
                    if (!isCapture(move_list[i])) {
                        updateHistory(move_list[i], depth);
                    }
                }
            }

            // --- Alpha cutoff check (beta <= alpha) ---
            if (alpha >= beta) {
                // Store killer move *before* storing TT entry
                if (!isCapture(move_list[i])) {
                    updateKillerMoves(move_list[i], depth);
                }
                // Flag remains FLAG_UPPERBOUND (initial assumption, score <= original_alpha)
                best_eval = alpha; // Return the cutoff bound score (fail-soft)

                // --- TT Store on Cutoff ---
                if (Tables::TT_NUM_ENTRIES > 0) {
                    size_t index = key & Tables::TT_MASK;
                    if (Tables::TRANSPOSITION_TABLE[index].depth <= depth) {
                        Tables::TRANSPOSITION_TABLE[index].zobrist_key_verify = key;
                        Tables::TRANSPOSITION_TABLE[index].score = (int16_t)best_eval;
                        Tables::TRANSPOSITION_TABLE[index].depth = (int8_t)depth;
                        Tables::TRANSPOSITION_TABLE[index].flag = FLAG_UPPERBOUND; // Explicitly store UPPERBOUND flag
                        Tables::TRANSPOSITION_TABLE[index].best_move = move_list[i]; // Store the move causing cutoff
                    }
                }
                return best_eval; // Prune
            }
        }
    }

    // --- Final TT Store (if no cutoff occurred) ---
    if (Tables::TT_NUM_ENTRIES > 0) {
        size_t index = key & Tables::TT_MASK;
        // Depth-preferred replacement (Only replace if new search is deeper or equal)
        if (Tables::TRANSPOSITION_TABLE[index].depth <= depth) {
            Tables::TRANSPOSITION_TABLE[index].zobrist_key_verify = key;
            // Ensure score is within int16_t range if necessary, although INF/-INF might be large
            Tables::TRANSPOSITION_TABLE[index].score = (int16_t)std::clamp(best_eval, -32767, 32767); // Clamp if score can exceed int16_t
            Tables::TRANSPOSITION_TABLE[index].depth = (int8_t)depth;
            Tables::TRANSPOSITION_TABLE[index].flag = flag; // Flag determined during search (UPPER, EXACT)
            Tables::TRANSPOSITION_TABLE[index].best_move = best_move_found;
        }
    }
    return best_eval; // Return the final evaluation for this node
}

int ChessAI::quiescence(Bitboard& board, int alpha, int beta, bool maximizingPlayer) {
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

        int score = -quiescence(board, -beta, -alpha, !maximizingPlayer);  // Negamax approach

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
    int score = board.evaluateBoard();
    if (!maximizingPlayer) score = -score; // If black, negate score

	// King safety evaluation for both sides
    // Acts as a penalty more than a bonus
    int king_safety = static_cast<int>(board.evaluateKingSafety() * KING_SAFETY_WEIGHT);
    score -= maximizingPlayer ? king_safety : -king_safety;

    // Encourage attacking the opponent’s king
    if (board.state.isCheckWhite()) score -= 50; // White in check
    if (board.state.isCheckBlack()) score += 50; // Black in check

    // Return the score
    return score;
}

int ChessAI::endgameMinimax(Bitboard& board, int depth, int alpha, int beta, bool maximizingPlayer) {
    // Check for terminal conditions (mate/draw)
    if (depth == 0 || board.isGameOver()) {
        return evaluateBoard(board, depth, maximizingPlayer);
    }

    // Check extension: Extend if current player is in check
    if (maximizingPlayer ? board.state.isCheckBlack() : board.state.isCheckWhite()) {
        depth += 1; // Standard extension
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

            int eval = endgameMinimax(board, depth - 1, alpha, beta, !maximizingPlayer);
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

            int eval = endgameMinimax(board, depth - 1, alpha, beta, !maximizingPlayer);
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
