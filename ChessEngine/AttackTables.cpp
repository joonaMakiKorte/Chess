#include "pch.h"
#include "AttackTables.h"

// Init tables
PawnMoves WHITE_PAWN_MOVES[64];
PawnMoves BLACK_PAWN_MOVES[64];


void initWhitePawnMoves(int square) {
	uint64_t bitboard = 1ULL << square; // Cast square to bitboard
	uint64_t single_push = (bitboard << 8) & ~RANK_4;  // Single step, but not beyond rank 4
	uint64_t double_push = ((bitboard & RANK_2) ? (bitboard << 16) & ~RANK_4 : 0ULL); // Double step only from rank 2

	uint64_t captures = 0ULL; // Empty at first
	if ((bitboard & FILE_A) == 0) captures |= (bitboard << 7); // NW attack
	if ((bitboard & FILE_H) == 0) captures |= (bitboard << 9); // NE attack

	// Insert moveset at square
	WHITE_PAWN_MOVES[square] = { single_push, double_push, captures };
}

void initBlackPawnMoves(int square) {
	uint64_t bitboard = 1ULL << square; // Cast square to bitboard
	uint64_t single_push = (bitboard >> 8) & ~RANK_5;  // Single step, but not beyond rank 5
	uint64_t double_push = ((bitboard & RANK_7) ? (bitboard >> 16) & ~RANK_5 : 0ULL); // Double step only from rank 7

	uint64_t captures = 0ULL; // Store captures
	if ((bitboard & FILE_A) == 0) captures |= (bitboard >> 9); // SW attack
	if ((bitboard & FILE_H) == 0) captures |= (bitboard >> 7); // SE attack

	// Insert moveset at square
	BLACK_PAWN_MOVES[square] = { single_push, double_push, captures };
}

void initMoveTables() {
	for (int square = 0; square < 64; square++) {
		initWhitePawnMoves(square);
		initBlackPawnMoves(square);
	}
}
