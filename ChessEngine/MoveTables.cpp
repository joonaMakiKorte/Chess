#include "pch.h"
#include "MoveTables.h"
#include "BitboardConstants.h"

// Init tables
PawnMoves WHITE_PAWN_MOVES[64];
PawnMoves BLACK_PAWN_MOVES[64];
BishopMoves BISHOP_MOVES[64];


void initWhitePawnMoves(int square) {
	uint64_t bitboard = 1ULL << square; // Cast square to bitboard

	uint64_t single_push = (bitboard << 8);  // Single step
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

	uint64_t single_push = (bitboard >> 8);  // Single step
	uint64_t double_push = ((bitboard & RANK_7) ? (bitboard >> 16) : 0ULL); // Double step only from rank 7

	uint64_t captures = 0ULL;
	// Ensure that only bits that are not on the border files can shift left/right
	if (!(bitboard & FILE_A)) captures |= (bitboard >> 9); // SW attack
	if (!(bitboard & FILE_H)) captures |= (bitboard >> 7); // SE attack

	// Insert moveset at square
	BLACK_PAWN_MOVES[square] = { single_push, double_push, captures };
}


void initBishopMoves(int square) {
	uint64_t bitboard = 1ULL << square; // Cast a square to bitboard

	// Different bishop routes
	uint64_t top_left = 0ULL;
	uint64_t top_right = 0ULL;
	uint64_t bottom_left = 0ULL;
	uint64_t bottom_right = 0ULL;

	int directions[] = { -7, 7, -9, 9 };  // Top-left, bottom-left, top-right, bottom-right


	// next we iterate over the diagonal directions
	for (int direction : directions) {
		uint64_t current_square = bitboard;

		// loop and explore the bitboard in the current direction
		while (true) {
			// Check if the move has gone beyond the edge of the board
			// The conditions handle edge cases where the move crosses the board's boundary
			// Each direction is checked against the corresponding edge of the board
			if ((direction == -7 && (current_square & FILE_A)) ||
				(direction == 7 && (current_square & FILE_H)) ||
				(direction == -9 && (current_square & FILE_H)) ||
				(direction == 9 && (current_square & FILE_A))) {
				break;
			}

			// Shift in the given direction
			current_square = (direction < 0) ? (current_square >> -direction) : (current_square << direction);

			// Ensure square remains on the board
			if (current_square == 0) break;

			// Add the current square to the corresponding diagonal move bitboard
			// Depending on the direction, the move will be added to the appropriate diagonal
			if (direction == -7) top_left |= current_square;
			else if (direction == 7) bottom_left |= current_square;
			else if (direction == -9) top_right |= current_square;
			else if (direction == 9) bottom_right |= current_square;
		}
	}

	// Store the calculated moves for this square
	BISHOP_MOVES[square] = { top_left, top_right, bottom_left, bottom_right };
}


void initMoveTables() {
	// Set flag to prevent unnecessary reinitialization
	static bool initialized = false; 
	if (initialized) return;
	initialized = true;

	for (int square = 0; square < 64; square++) {
		initWhitePawnMoves(square);
		initBlackPawnMoves(square);
		initBishopMoves(square);
	}
}
