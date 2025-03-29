#include "pch.h"
#include "MoveTables.h"
#include "Magic.h"
#include "Utils.h"

// Static allocation of tables
// Declared as global arrays
PawnMoves WHITE_PAWN_MOVES[64];
PawnMoves BLACK_PAWN_MOVES[64];
KnightMoves KNIGHT_MOVES[64];
KingMoves KING_MOVES[64];

// Define the large tables
uint64_t(*ATTACKS_BISHOP)[512] = new uint64_t[64][512];
uint64_t(*ATTACKS_ROOK)[4096] = new uint64_t[64][4096];

void initWhitePawnMoves(int square) {
	uint64_t bitboard = 1ULL << square; // Cast square to bitboard

	uint64_t single_push = bitboard << 8; // Single step
	uint64_t double_push = ((bitboard & RANK_2) ? (bitboard << 16) : 0ULL); // Double step only from rank 2

	uint64_t captures = 0ULL;
	// Ensure that only bits that are not on the border files can shift left/right
	if (!(bitboard & FILE_A)) captures |= (bitboard << 7); // NW attack
	if (!(bitboard & FILE_H)) captures |= (bitboard << 9); // NE attack

	// Insert moveset at square
	WHITE_PAWN_MOVES[square] = { single_push, double_push, captures };
}

void initBlackPawnMoves(int square) {
	uint64_t bitboard = 1ULL << square; // Cast square to bitboard

	uint64_t single_push = bitboard >> 8; // Single step
	uint64_t double_push = ((bitboard & RANK_7) ? (bitboard >> 16) : 0ULL); // Double step only from rank 7

	uint64_t captures = 0ULL;
	// Ensure that only bits that are not on the border files can shift left/right
	if (!(bitboard & FILE_A)) captures |= (bitboard >> 9); // SW attack
	if (!(bitboard & FILE_H)) captures |= (bitboard >> 7); // SE attack

	// Insert moveset at square
	BLACK_PAWN_MOVES[square] = { single_push, double_push, captures };
}

void initKnightMoves(int square) {
	uint64_t bitboard = 1ULL << square; // Place knight on the square
	uint64_t moves = 0ULL;

	// Possible knight jumps (relative shifts)
	int jumps[8] = { 6, 10, 15, 17, -6, -10, -15, -17 };
	// Left-top, right-top, top-left, top-right, right-bottom, left-bottom, bottom-right, bottom-left

	for (int jump : jumps) {
		// Prevent wrapping
		if (((jump == 15 || jump == -17 || jump == 6 || jump == -10) && (bitboard & FILE_A)) ||
			((jump == 6 || jump == -10) && (bitboard & FILE_B)) ||
			((jump == 10 || jump == -6) && (bitboard & FILE_G)) ||
			((jump == 17 || jump == -15 || jump == 10 || jump == -6) && (bitboard & FILE_H)) ||
			((jump == -10 || jump == -6 || jump == -17 || jump == -15) && (bitboard & RANK_1)) ||
			((jump == -17 || jump == -15) && (bitboard & RANK_2)) ||
			((jump == 15 || jump == 17) && (bitboard & RANK_7)) ||
			((jump == 6 || jump == 10 || jump == 15 || jump == 17) && (bitboard & RANK_8))) {
			continue;
		}

		// Shift correctly depending on direction
		moves |= (jump < 0) ? (bitboard >> -jump) : (bitboard << jump);
	}

	// Store the calculated moves for this square
	KNIGHT_MOVES[square] = { moves };
}

void initKingMoves(int square) {
	uint64_t bitboard = 1ULL << square; // Convert square index to bitboard
	uint64_t moves = 0ULL;

	// Directions for king (one step in all 8 directions)
	int directions[8] = { 8, -8, -1, 1, 7, 9, -9, -7 }; // Top, bottom, left, right, top-left, top-right, bottom-left, bottom-right

	for (int direction : directions) {
		// Check for board edges to prevent wrapping
		if (((direction == 7 || direction == -9) && (bitboard & FILE_A)) ||  // Left-border
			((direction == 9 || direction == -7) && (bitboard & FILE_H)) ||  // Right-border
			((direction == 7 || direction == 9) && (bitboard & RANK_8)) ||   // Top-border
			((direction == -9 || direction == -7) && (bitboard & RANK_1)) || // Bottom-border
			((direction == 1 && (bitboard & FILE_H)) ||					     // Prevent left wrap
		    (direction == -1 && (bitboard & FILE_A)) ||                      // Prevent right wrap
			(direction == 8 && (bitboard & RANK_8)) ||                       // Prevent bottom wrap
			(direction == -8 && (bitboard & RANK_1)))) {                     // Prevent top wrap
			continue;
		}

		// Shift correctly depending on direction
		moves |= (direction < 0) ? (bitboard >> -direction) : (bitboard << direction);
	}

	KING_MOVES[square] = { moves };
}

void initMoveTables() {
	// Set flag to prevent unnecessary reinitialization
	static bool initialized = false; 
	if (initialized) return;
	initialized = true;

	// Non-sliding pieces
	for (int square = 0; square < 64; square++) {
		initWhitePawnMoves(square);
		initBlackPawnMoves(square);
		initKnightMoves(square);
		initKingMoves(square);
	}

	// Initialize magic tables
	initMagicTables();

	// Bishop
	for (int sq = 0; sq < 64; sq++) {
		int occupancy_indices = 1 << RELEVANT_BITS_COUNT_BISHOP[sq];
		for (int i = 0; i < occupancy_indices; i++) {
			uint64_t occupancy = Utils::setOccupancy(i, RELEVANT_BITS_COUNT_BISHOP[sq], MAGIC_TABLE_BISHOP[sq].mask);
			int index = (int)((occupancy * MAGIC_TABLE_BISHOP[sq].magic) >> MAGIC_TABLE_BISHOP[sq].shift);
			ATTACKS_BISHOP[sq][index] = maskBishopXrayAttacks(sq, occupancy);
		}
	}
	
	// Rook
	for (int sq = 0; sq < 64; sq++) {
		int occupancy_indices = 1 << RELEVANT_BITS_COUNT_ROOK[sq];
		for (int i = 0; i < occupancy_indices; i++) {
			uint64_t occupancy = Utils::setOccupancy(i, RELEVANT_BITS_COUNT_ROOK[sq], MAGIC_TABLE_ROOK[sq].mask);
			int index = (int)((occupancy * MAGIC_TABLE_ROOK[sq].magic) >> MAGIC_TABLE_ROOK[sq].shift);
			ATTACKS_ROOK[sq][index] = maskRookXrayAttacks(sq, occupancy);
		}
	}
}

void teardown() {
	delete[] ATTACKS_BISHOP;
	delete[] ATTACKS_ROOK;
}
