// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// Pre-compiled headers
#include "framework.h"
#include <cstdint>
#include <string>
#include <cstring>
#include <stdexcept>
#include <array>

// Pre-computed variables
constexpr int UNASSIGNED = -1; // Sentinel value for unassigned variables
constexpr int MAX_MOVES = 32; // Max number of legal moves for a piece
// in theory max would be 28 (for queen) but we use 32 for alignment and placement


// Pre-computed attack tables for chess pieces

// Computed at compile time since tables are small and frequently used
constexpr uint64_t WHITE_PAWN_ATTACK_TABLE[64];
constexpr uint64_t BLACK_PAWN_ATTACK_TABLE[64];
constexpr uint64_t KNIGHT_ATTACK_TABLE[64];
constexpr uint64_t KING_ATTACK_TABLE[64];

// Computed at runtime (still constant) since tables are large and less frequently used
extern const uint64_t WHITE_BISHOP_ATTACK_TABLE[64];
extern const uint64_t BLACK_BISHOP_ATTACK_TABLE[64];
extern const uint64_t ROOK_ATTACK_TABLE[64];
extern const uint64_t QUEEN_ATTACK_TABLE[64];

#endif //PCH_H