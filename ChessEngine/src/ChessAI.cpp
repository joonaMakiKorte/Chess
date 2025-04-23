 #include "pch.h"
#include "ChessAI.hpp"
#include "Bitboard.hpp"
#include "Tables.hpp"
#include "Scoring.hpp"


uint32_t ChessAI::getBestMove(std::unique_ptr<Bitboard>& board, int depth, bool maximizing) {
    std::array<uint32_t, MAX_MOVES> move_list;
    int move_count = 0;
    // Generate all legal moves for the AI side
    board->generateMoves(move_list, move_count, 0, maximizing, NULL_MOVE_32);

    if (move_count == 0) {
        return 0; // No legal moves available
    }

    int bestScore = -INF;
    uint32_t bestMove = 0;

    board->startNewSearch(); // Clear previous search data

    for (int i = 0; i < move_count; i++) {
        board->applyMoveAI(move_list[i], maximizing);
        // Negamax: flip perspective by negating recursive result
        int score = -minimax(board, depth - 1, -INF, INF, !maximizing);
        board->undoMoveAI(move_list[i], maximizing);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move_list[i];
        }
    }

    return bestMove;
}

uint32_t ChessAI::getBestEndgameMove(std::unique_ptr<Bitboard>& board, int depth, bool maximizing) {
    std::array<uint32_t, MAX_MOVES> move_list;
    int move_count = 0;
    // Generate all legal endgame moves for the AI side
    board->generateEndgameMoves(move_list, move_count, 0, maximizing, NULL_MOVE_32);

    if (move_count == 0) {
        return 0;
    }

    int bestScore = -INF;
    uint32_t bestMove = 0;

    board->startNewSearch();

    for (int i = 0; i < move_count; i++) {
        board->applyMoveAI(move_list[i], maximizing);
        int score = -endgameMinimax(board, depth - 1, -INF, INF, !maximizing);
        board->undoMoveAI(move_list[i], maximizing);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move_list[i];
        }
    }

    return bestMove;
}


int ChessAI::minimax(std::unique_ptr<Bitboard>& board, int depth, int alpha, int beta, bool maximizing) {
    // --- Repetition and 50-Move Rule Checks (BEFORE TT Probe/Other Checks) ---
    // Check 50-move rule first (simple counter check)
    if (board->getHalfMoveClock() >= 50) {
        return 0; // Draw score
    }
    // Check threefold repetition using the path history
    // Important: Check the *current* node before exploring children
    if (board->isDrawByRepetition()) {
        return 0; // Draw score
    }

    // --- Mate Distance Pruning ---
    // If we have already found a mate score, check if the current depth can possibly improve it.
    // alpha and beta track the best scores found so far. Mate scores are typically large.
    // If alpha is already a mate score found sooner (higher value), don't search deeper if we can't beat it.
    alpha = std::max(alpha, -MATE_SCORE + board->getPlyCount()); // Adjust alpha based on ply from root
    beta = std::min(beta, MATE_SCORE - board->getPlyCount());   // Adjust beta based on ply from root
    if (alpha >= beta) {
        return alpha; // Mate distance pruning
    }

    // --- TT Probe ---
    // --- Transposition Table Probe ---
    uint64_t key = board->getHashKey();
    uint32_t tt_best_move = NULL_MOVE_32;
    int original_alpha = alpha; // Store original alpha for TT store flag logic
    bool tt_hit = false;
    int tt_score = -INF; // Initialize with a value indicating no valid score yet

    if (Tables::TT_NUM_ENTRIES > 0) {
        size_t index = key & Tables::TT_MASK;
        TTEntry& entry = Tables::TRANSPOSITION_TABLE[index];

        if (entry.zobrist_key_verify == key) { // Check if the entry belongs to the current position
            tt_hit = true;
            tt_best_move = entry.best_move; // Use this move first in move ordering

            if (entry.depth >= depth) { // Check if the stored depth is sufficient
                int stored_score = entry.score;

                // Adjust score from mate distance perspective if it's a mate score
                if (stored_score > MATE_SCORE - MAX_PLY_FROM_MATE) stored_score -= board->getPlyCount();
                if (stored_score < -MATE_SCORE + MAX_PLY_FROM_MATE) stored_score += board->getPlyCount();


                // Use stored information based on the flag
                if (entry.flag == FLAG_EXACT) {
                    return stored_score; // Exact score found
                }
                if (entry.flag == FLAG_LOWERBOUND) { // Failed high previously (score >= beta)
                    if (stored_score >= beta) {
                        // Update killer move based on TT cutoff before returning
                        if (!isCapture(tt_best_move) && tt_best_move != NULL_MOVE_32) {
                            updateKillerMoves(tt_best_move, depth);
                        }
                        return stored_score; // This stored lower bound causes a beta cutoff now
                    }
                    alpha = std::max(alpha, stored_score); // Tighten alpha
                }
                else if (entry.flag == FLAG_UPPERBOUND) { // Failed low previously (score <= alpha)
                    if (stored_score <= alpha) {
                        // Update killer move based on TT cutoff before returning
                        if (!isCapture(tt_best_move) && tt_best_move != NULL_MOVE_32) {
                            updateKillerMoves(tt_best_move, depth);
                        }
                        return stored_score; // This stored upper bound causes an alpha cutoff now (fail low)
                    }
                    beta = std::min(beta, stored_score); // Tighten beta
                }

                // Check if bounds crossed after tightening
                if (alpha >= beta) {
                    // Return the score that caused the cutoff (using alpha as it's a lower bound we achieved)
                    // Could return stored_score here too depending on exact fail-soft preference
                    return alpha;
                }
            }
        }
    }

    // --- Base Case: Check for Terminal Nodes or Reached Max Depth ---
    // Check game over *after* TT probe, as TT might have the result
    if (board->isGameOver()) {
        // Ensure evaluateBoard returns score relative to the current player
        return evaluateBoard(board, depth, maximizing);
    }

    // Call Quiescence Search at depth 0
    if (depth <= 0) {
        // Ensure quiescence returns score relative to the current player and uses Negamax
        return quiescence(board, alpha, beta, maximizing);
    }

    // --- Main Negamax Search Logic ---
    std::array<uint32_t, MAX_MOVES> move_list;
    int move_count = 0;
    // Generate moves, potentially using tt_best_move for ordering
    // The 'maximizing' flag might be needed here if generateMoves depends on it
    board->generateMoves(move_list, move_count, depth, maximizing, tt_best_move);

    // Check if no legal moves (Stalemate or Checkmate handled by isGameOver, but as safeguard)
    if (move_count == 0) {
        // If no moves but not game over (shouldn't happen with correct isGameOver) -> Draw?
        // Or evaluate should handle mate/stalemate based on check status
        return evaluateBoard(board, depth, maximizing);
    }


    int best_eval = -INF; // Best score found so far for the current player
    uint32_t best_move_found = NULL_MOVE_32; // Track best move at this node
    TTFlag flag = FLAG_UPPERBOUND; // Assume fail-low initially (score <= original_alpha)

    // --- Iterate Through Moves ---
    for (int i = 0; i < move_count; i++) {
        // Apply the move
        // 'maximizing' might be needed if apply/undo depend on it
        board->applyMoveAI(move_list[i], maximizing);

        // Recursive Negamax call: negate result, swap & negate bounds
        int eval = -minimax(board, depth - 1, -beta, -alpha, !maximizing);

        // Undo the move
        board->undoMoveAI(move_list[i], maximizing);


        // --- Update Best Score and Alpha ---
        if (eval > best_eval) {
            best_eval = eval; // Found a better move/score from current player's perspective

            if (best_eval > alpha) {
                alpha = best_eval; // Raise the lower bound (best score guaranteed for current player)
                best_move_found = move_list[i]; // This is currently the best move
                flag = FLAG_EXACT; // Score is potentially exact (within original alpha-beta window)

                // Update history heuristic for non-captures that improve alpha
                if (!isCapture(move_list[i])) {
                    updateHistory(move_list[i], depth);
                }
            }
        }

        // --- Beta Cutoff Check (Fail High) ---
        // If alpha >= beta, opponent has a better option earlier in the tree.
        // The current player's score is guaranteed to be at least beta.
        if (alpha >= beta) {
            // Store killer move *before* storing TT entry (only non-captures)
            if (!isCapture(move_list[i])) {
                updateKillerMoves(move_list[i], depth);
            }

            flag = FLAG_LOWERBOUND; // Indicates the score is at least beta (failed high)
            best_eval = beta; // Return the cutoff bound score (fail-soft)

            // --- TT Store on Beta Cutoff ---
            if (Tables::TT_NUM_ENTRIES > 0) {
                size_t index = key & Tables::TT_MASK;
                // Store only if entry is empty, shallower, or same depth (preference)
                if (Tables::TRANSPOSITION_TABLE[index].depth <= depth || Tables::TRANSPOSITION_TABLE[index].zobrist_key_verify != key) {
                    TTEntry& entry_to_store = Tables::TRANSPOSITION_TABLE[index];
                    entry_to_store.zobrist_key_verify = key;
                    // Adjust score for mate distance before storing
                    int store_score = best_eval;
                    if (store_score > MATE_SCORE - MAX_PLY_FROM_MATE) store_score += board->getPlyCount(); // Store relative to root

                    entry_to_store.score = (int16_t)std::clamp(store_score, -32767, 32767);
                    entry_to_store.depth = (int8_t)depth;
                    entry_to_store.flag = flag; // FLAG_LOWERBOUND
                    entry_to_store.best_move = move_list[i]; // Store the move causing cutoff
                }
            }
            return best_eval; // Prune the rest of the moves at this node
        }
    } // End of move loop

    // --- Final TT Store (if no cutoff occurred) ---
    // We explored all moves and didn't get a beta cutoff.
    // The best score found is 'alpha' (if it improved) or the initial 'best_eval' (if it didn't raise alpha).
    // The flag is either FLAG_EXACT (if alpha > original_alpha) or FLAG_UPPERBOUND (if alpha <= original_alpha).
    if (Tables::TT_NUM_ENTRIES > 0) {
        size_t index = key & Tables::TT_MASK;
        // Store only if entry is empty, shallower, or same depth and better flag (Exact > Bounds)
        bool should_replace = Tables::TRANSPOSITION_TABLE[index].zobrist_key_verify != key ||
            Tables::TRANSPOSITION_TABLE[index].depth < depth ||
            (Tables::TRANSPOSITION_TABLE[index].depth == depth && flag == FLAG_EXACT && Tables::TRANSPOSITION_TABLE[index].flag != FLAG_EXACT);

        if (should_replace) {
            TTEntry& entry_to_store = Tables::TRANSPOSITION_TABLE[index];
            entry_to_store.zobrist_key_verify = key;
            // Adjust score for mate distance before storing
            int store_score = alpha; // alpha holds the best score found within the bounds
            if (store_score > MATE_SCORE - MAX_PLY_FROM_MATE) store_score += board->getPlyCount(); // Store relative to root
            if (store_score < -MATE_SCORE + MAX_PLY_FROM_MATE) store_score -= board->getPlyCount(); // Store relative to root


            entry_to_store.score = (int16_t)std::clamp(store_score, -32767, 32767);
            entry_to_store.depth = (int8_t)depth;
            entry_to_store.flag = flag; // Will be FLAG_EXACT or FLAG_UPPERBOUND
            entry_to_store.best_move = best_move_found; // Store the best move found
        }
    }

    // Return the best score found for the current player within the alpha-beta bounds
    // In Negamax fail-soft, this is typically 'alpha'.
    return alpha;
}

int ChessAI::quiescence(std::unique_ptr<Bitboard>& board, int alpha, int beta, bool maximizing) {
    // --- Repetition and 50-Move Rule Checks (BEFORE TT Probe/Other Checks) ---
    // Check 50-move rule first (simple counter check)
    if (board->getHalfMoveClock() >= 50) {
        return 0; // Draw score
    }
    // Check threefold repetition using the path history
    // Important: Check the *current* node before exploring children
    if (board->isDrawByRepetition()) {
        return 0; // Draw score
    }

    int eval = evaluateBoard(board, 0, maximizing);  // Get a static evaluation of the current position

    // Stand pat: if this position is already better than beta, cut off search (pruning)
    if (eval >= beta) return beta;
    if (eval > alpha) alpha = eval;  // Update alpha if we find a better move

    // Generate captures + promotions (non quiet moves)
    std::array<uint32_t, MAX_MOVES> move_list;
    int move_count = 0;
    board->generateNoisyMoves(move_list, move_count, maximizing);

    for (int i = 0; i < move_count; i++) {
        int move_value = board->estimateCaptureValue(move_list[i]);

        // Delta pruning - skip moves that can't possibly raise alpha
        // Skip delta pruning for all promotions
        if (!isPromotion(move_list[i]) && eval + move_value + DELTA_MARGIN_MIDGAME <= alpha) {
            continue; // Skip this move as it can't improve alpha
        }
        board->applyMoveAI(move_list[i], maximizing);

        int score = -quiescence(board, -beta, -alpha, !maximizing);  // Negamax approach

        board->undoMoveAI(move_list[i], maximizing);

        if (score >= beta) return beta;  // Beta cutoff
        if (score > alpha) alpha = score;  // Improve alpha
    }

    return alpha;  // Best evaluation we found
}


int ChessAI::evaluateBoard(std::unique_ptr<Bitboard>& board, int depth, bool maximizing) {
    int score = 0;
    if (board->state.isCheckmateWhite()) score = -MATE_SCORE + (depth * 1000);  // White loses
	else if (board->state.isCheckmateBlack()) score = MATE_SCORE - (depth * 1000); // Black loses
    else if (board->state.isStalemate()) score = 0; // Draw -> neutral outcome
    else {
        score = board->evaluateBoard(); // Material+positional score relative to white
        // King safety evaluation for both sides
        // Acts as a penalty more than a bonus
        score -= static_cast<int>(board->evaluateKingSafety() * KING_SAFETY_WEIGHT);
        // Encourage attacking the opponents king
        if (board->state.isCheckWhite()) score -= 50; // White in check
        if (board->state.isCheckBlack()) score += 50; // Black in check
    }

    // Return the score (negate for black)
    return maximizing ? score : -score;
}

int ChessAI::endgameMinimax(std::unique_ptr<Bitboard>& board, int depth, int alpha, int beta, bool maximizing) {
    // --- Repetition and 50-Move Rule Checks (BEFORE TT Probe/Other Checks) ---
    if (board->getHalfMoveClock() >= 50) {
        return 0; // Draw score
    }

    if (board->isDrawByRepetition()) {
        return 0; // Draw score
    }

    // --- Mate Distance Pruning ---
    alpha = std::max(alpha, -MATE_SCORE + board->getPlyCount()); // Adjust alpha based on ply from root
    beta = std::min(beta, MATE_SCORE - board->getPlyCount());   // Adjust beta based on ply from root
    if (alpha >= beta) {
        return alpha; // Mate distance pruning
    }

    // --- TT Probe ---
    // --- Transposition Table Probe ---
    uint64_t key = board->getHashKey();
    uint32_t tt_best_move = NULL_MOVE_32;
    int original_alpha = alpha;
    bool tt_hit = false;
    int tt_score = -INF; 

    if (Tables::TT_NUM_ENTRIES > 0) {
        size_t index = key & Tables::TT_MASK;
        TTEntry& entry = Tables::TRANSPOSITION_TABLE[index];

        if (entry.zobrist_key_verify == key) {
            tt_hit = true;
            tt_best_move = entry.best_move; 

            if (entry.depth >= depth) {
                int stored_score = entry.score;

                if (stored_score > MATE_SCORE - MAX_PLY_FROM_MATE) stored_score -= board->getPlyCount();
                if (stored_score < -MATE_SCORE + MAX_PLY_FROM_MATE) stored_score += board->getPlyCount();


                if (entry.flag == FLAG_EXACT) {
                    return stored_score; 
                }
                if (entry.flag == FLAG_LOWERBOUND) {
                    if (stored_score >= beta) {
                        if (!isCapture(tt_best_move) && tt_best_move != NULL_MOVE_32) {
                            updateKillerMoves(tt_best_move, depth);
                        }
                        return stored_score;
                    }
                    alpha = std::max(alpha, stored_score);
                }
                else if (entry.flag == FLAG_UPPERBOUND) {
                    if (stored_score <= alpha) {
                        if (!isCapture(tt_best_move) && tt_best_move != NULL_MOVE_32) {
                            updateKillerMoves(tt_best_move, depth);
                        }
                        return stored_score;
                    }
                    beta = std::min(beta, stored_score);
                }

                if (alpha >= beta) {
                    return alpha;
                }
            }
        }
    }

    // --- Base Case: Check for Terminal Nodes or Reached Max Depth ---
    if (board->isGameOver()) {
        return evaluateEndgameBoard(board, depth, maximizing);
    }

    // Call Quiescence Search at depth 0
    if (depth <= 0) {
        return endgameQuiescence(board, alpha, beta, maximizing);
    }

    // --- Main Negamax Search Logic ---
    std::array<uint32_t, MAX_MOVES> move_list;
    int move_count = 0;
    board->generateEndgameMoves(move_list, move_count, depth, maximizing, tt_best_move);

    // Check if no legal moves (Stalemate or Checkmate handled by isGameOver, but as safeguard)
    if (move_count == 0) {
        return evaluateEndgameBoard(board, depth, maximizing);
    }


    int best_eval = -INF; 
    uint32_t best_move_found = NULL_MOVE_32; 
    TTFlag flag = FLAG_UPPERBOUND; 

    // --- Iterate Through Moves ---
    for (int i = 0; i < move_count; i++) {
        board->applyMoveAI(move_list[i], maximizing);

        int eval = -endgameMinimax(board, depth - 1, -beta, -alpha, !maximizing);

        board->undoMoveAI(move_list[i], maximizing);

        // --- Update Best Score and Alpha ---
        if (eval > best_eval) {
            best_eval = eval; 

            if (best_eval > alpha) {
                alpha = best_eval; 
                best_move_found = move_list[i];
                flag = FLAG_EXACT; 

                if (!isCapture(move_list[i])) {
                    updateHistory(move_list[i], depth);
                }
            }
        }

        // --- Beta Cutoff Check (Fail High) ---
        if (alpha >= beta) {
            if (!isCapture(move_list[i])) {
                updateKillerMoves(move_list[i], depth);
            }

            flag = FLAG_LOWERBOUND; 
            best_eval = beta; 

            // --- TT Store on Beta Cutoff ---
            if (Tables::TT_NUM_ENTRIES > 0) {
                size_t index = key & Tables::TT_MASK;
                if (Tables::TRANSPOSITION_TABLE[index].depth <= depth || Tables::TRANSPOSITION_TABLE[index].zobrist_key_verify != key) {
                    TTEntry& entry_to_store = Tables::TRANSPOSITION_TABLE[index];
                    entry_to_store.zobrist_key_verify = key;

                    int store_score = best_eval;
                    if (store_score > MATE_SCORE - MAX_PLY_FROM_MATE) store_score += board->getPlyCount(); 

                    entry_to_store.score = (int16_t)std::clamp(store_score, -32767, 32767);
                    entry_to_store.depth = (int8_t)depth;
                    entry_to_store.flag = flag;
                    entry_to_store.best_move = move_list[i]; 
                }
            }
            return best_eval; // Prune the rest of the moves at this node
        }
    } // End of move loop

    // --- Final TT Store (if no cutoff occurred) ---
    if (Tables::TT_NUM_ENTRIES > 0) {
        size_t index = key & Tables::TT_MASK;
        bool should_replace = Tables::TRANSPOSITION_TABLE[index].zobrist_key_verify != key ||
            Tables::TRANSPOSITION_TABLE[index].depth < depth ||
            (Tables::TRANSPOSITION_TABLE[index].depth == depth && flag == FLAG_EXACT && Tables::TRANSPOSITION_TABLE[index].flag != FLAG_EXACT);

        if (should_replace) {
            TTEntry& entry_to_store = Tables::TRANSPOSITION_TABLE[index];
            entry_to_store.zobrist_key_verify = key;

            int store_score = alpha; 
            if (store_score > MATE_SCORE - MAX_PLY_FROM_MATE) store_score += board->getPlyCount(); 
            if (store_score < -MATE_SCORE + MAX_PLY_FROM_MATE) store_score -= board->getPlyCount(); 


            entry_to_store.score = (int16_t)std::clamp(store_score, -32767, 32767);
            entry_to_store.depth = (int8_t)depth;
            entry_to_store.flag = flag; 
            entry_to_store.best_move = best_move_found; 
        }
    }

    return alpha;
}

int ChessAI::endgameQuiescence(std::unique_ptr<Bitboard>& board, int alpha, int beta, bool maximizing) {
    // --- Repetition and 50-Move Rule Checks (BEFORE TT Probe/Other Checks) ---
    if (board->getHalfMoveClock() >= 50) {
        return 0; // Draw score
    }
    // Check threefold repetition using the path history
    if (board->isDrawByRepetition()) {
        return 0; // Draw score
    }

    int eval = evaluateEndgameBoard(board, 0, maximizing);  // Get a static evaluation of the current position

    // Stand pat: if this position is already better than beta, cut off search (pruning)
    if (eval >= beta) return beta;
    if (eval > alpha) alpha = eval;  // Update alpha if we find a better move

    // Generate captures + promotions (non quiet moves)
    std::array<uint32_t, MAX_MOVES> move_list;
    int move_count = 0;
    board->generateEndgameNoisyMoves(move_list, move_count, maximizing);

    for (int i = 0; i < move_count; i++) {
        int move_value = board->estimateCaptureValue(move_list[i]);

        // Delta pruning - skip moves that can't possibly raise alpha
        // Skip delta pruning for all promotions and checks
        if (!isPromotion(move_list[i]) && !isCheck(move_list[i]) && eval + move_value + DELTA_MARGIN_ENDGAME <= alpha) {
            continue; // Skip this move as it can't improve alpha
        }
        board->applyMoveAI(move_list[i], maximizing);

        int score = -quiescence(board, -beta, -alpha, !maximizing);  // Negamax approach

        board->undoMoveAI(move_list[i], maximizing);

        if (score >= beta) return beta;  // Beta cutoff
        if (score > alpha) alpha = score;  // Improve alpha
    }

    return alpha;  // Best evaluation we found
}

int ChessAI::evaluateEndgameBoard(std::unique_ptr<Bitboard>& board, int depth, bool maximizing) {
    int score = 0;
    if (board->state.isCheckmateWhite()) score = -MATE_SCORE + (depth * 1000);  // White loses
    else if (board->state.isCheckmateBlack()) score = MATE_SCORE - (depth * 1000); // Black loses
    else if (board->state.isStalemate()) score = 0; // Draw -> neutral outcome
    else {
        // Evaluate material and positional score of the board
        score = board->evaluateBoard();

        // Evaluate passed pawns
        score += board->evaluatePassedPawns(true) - board->evaluatePassedPawns(false); // Passed pawn delta between white and black

        // Encourage closing distance between kings by giving bonus
        score += 10 * (7 - board->calculateKingDistance());

        // Award king centralization (if opponent is more centralized acts as a penalty)
        score += board->getKingCentralization();
    }

    // Return the score (negate for black)
    return maximizing ? score : -score;
}

inline bool ChessAI::isCapture(uint32_t move) {
    return capturedPiece(move) != EMPTY || moveType(move) == EN_PASSANT;
}

inline bool ChessAI::isPromotion(uint32_t move) {
    return moveType(move) == PROMOTION || moveType(move) == PROMOTION_CAPTURE;
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
