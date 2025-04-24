#ifndef BITBOARD_CONSTANTS_H
#define BITBOARD_CONSTANTS_H

#include <cstdint>

// File masks
constexpr uint64_t FILE_A = 0x0101010101010101ULL;
constexpr uint64_t FILE_B = 0x0202020202020202ULL;
constexpr uint64_t FILE_G = 0x4040404040404040ULL;
constexpr uint64_t FILE_H = 0x8080808080808080ULL;

// Rank masks
constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
constexpr uint64_t RANK_2 = 0x000000000000FF00ULL;
constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;
constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;

// Masks for castling rights
constexpr uint64_t WHITE_KINGSIDE_CASTLE_SQUARES = (1ULL << 5) | (1ULL << 6); // (f1, g1)
constexpr uint64_t WHITE_QUEENSIDE_CASTLE_SQUARES = (1ULL << 1) | (1ULL << 2) | (1ULL << 3); // (b1, c1, d1)
constexpr uint64_t BLACK_KINGSIDE_CASTLE_SQUARES = (1ULL << 61) | (1ULL << 62); // (f8, g8)
constexpr uint64_t BLACK_QUEENSIDE_CASTLE_SQUARES = (1ULL << 57) | (1ULL << 58) | (1ULL << 59); // (b8, c8, d8)

// Masks for castling operations
constexpr uint64_t WHITE_KING = (1ULL << 4); // (e1)
constexpr uint64_t BLACK_KING = (1ULL << 60); // (e8)
constexpr uint64_t ROOK_A1 = (1ULL << 0); // (a1)
constexpr uint64_t ROOK_D1 = (1ULL << 3); // (d1)
constexpr uint64_t ROOK_F1 = (1ULL << 5); // (f1)
constexpr uint64_t ROOK_H1 = (1ULL << 7); // (h1)
constexpr uint64_t ROOK_A8 = (1ULL << 56); // (a8)
constexpr uint64_t ROOK_D8 = (1ULL << 59); // (d8)
constexpr uint64_t ROOK_F8 = (1ULL << 61); // (f8)
constexpr uint64_t ROOK_H8 = (1ULL << 63); // (h8)

// Pre-computed variables
constexpr int INF = 2147483647; // Infinity

// Desired size for transposition table in MB
constexpr size_t DESIRED_TT_SIZE_MB = 128;

constexpr int UNASSIGNED = -1; // Sentinel value for unassigned variables

constexpr uint16_t NULL_MOVE = 0xFFFF; // An impossible move (all bits set)
constexpr uint32_t NULL_MOVE_32 = 0xFFFFFFFF; // 32-bit version
constexpr int MAX_HISTORY_KEY = 65536; // Max value for history table key (max 16-bit int)

constexpr int MAX_MOVES = 256; // Max number of legal moves for a player turn
// Absolute theoretical maximum would be 218 but we use 256 (a power of 2) for efficient memory alignment

constexpr int MAX_GAME_PHASE = 24; // Maximum game phase (total number of pieces on board)
// Game phase is used to determine if we are in the middle or endgame

constexpr int MAX_DEPTH = 64;  // Maximum playsible search depth for minimax

constexpr int MAX_SEARCH_DEPTH = 128; // Covers maximum plausible search depth for minimax + quiescence
// 128 for alignment + would be an extreme case which is near impossible

constexpr int MAX_PLY_FROM_MATE = 128; // Max ply num to reach mate (64 turns) (adjustable)

constexpr int MAX_QUIET_MOVES = 4; // Cap to limit the number of quiet moves stored

// Margin for delta pruning in q-search
constexpr int DELTA_MARGIN_MIDGAME = 200;
constexpr int DELTA_MARGIN_ENDGAME = 250;


// Game phase recalculation threshold in range of 0-1
// Meaning a phase change greater than this in percentages after applying a move results in full positional score recalculation
// This way we can avoid unnecessary recalculations to prioritize evaluation speed
constexpr float FULL_RECALC_THRESHOLD = 0.1f;

// Threshold for game phase score for which after we start using the endgame board evaluation in quiescence
// Opening 22-24     // Most pieces still on the board
// Middlegame 10-21 // Some exhanges, queens often present
// Endgame 0-9     // Few pieces left, kings active
constexpr int ENDGAME_THRESHOLD = 6;

// Weight factor for king safety penalty in board evaluation
// Integrated for fine-tuning factor
constexpr float KING_SAFETY_WEIGHT = 1.0f; 

constexpr int CENTRALITY_DISTANCE[64] = {
    // Precomputed min Chebyshev distance to center (d4,e4,d5,e5)
    4, 4, 4, 3, 3, 4, 4, 4,  // a1-h1
    4, 3, 3, 2, 2, 3, 3, 4,  // a2-h2
    4, 3, 2, 1, 1, 2, 3, 4,  // a3-h3
    3, 2, 1, 0, 0, 1, 2, 3,  // a4-h4 (d4=0, e4=0)
    3, 2, 1, 0, 0, 1, 2, 3,  // a5-h5 (d5=0, e5=0)
    4, 3, 2, 1, 1, 2, 3, 4,  // a6-h6
    4, 3, 3, 2, 2, 3, 3, 4,  // a7-h7
    4, 4, 4, 3, 3, 4, 4, 4   // a8-h8
};

#endif // BITBOARD_CONSTANTS_H