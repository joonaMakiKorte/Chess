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
constexpr int MAX_MOVES = 32; // Max number of legal moves for a piece, in theory max would be 28 (for queen) but we use 32 for alignment and placement

constexpr uint64_t FILE_A = 0x0101010101010101ULL; // Mask for file A
constexpr uint64_t FILE_H = 0x8080808080808080ULL; // Mask for file H
// Masks for ranks
constexpr uint64_t RANK_2 = 0x000000000000FF00ULL; 
constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;

#endif //PCH_H