#include "pch.h"
#include "Tables.h"
#include "Utils.h"

namespace Tables {
	// Declare tables
	uint64_t BETWEEN[64][64];
	uint64_t LINE[64][64];
	Direction DIR[64][64];

	uint16_t KILLER_MOVES[MAX_DEPTH][2] = { NULL_MOVE };
	int* HISTORY_TABLE = new int[MAX_HISTORY_KEY](); // Zero-initialized array

	uint64_t PIECE_KEYS[2][6][64];
	uint64_t SIDE_TO_MOVE_KEY;
	uint64_t CASTLING_KEYS[16];
	uint64_t EN_PASSANT_KEYS[8];

	// Compute direction between squares
	Direction getDirection(int sq1, int sq2) {
		int dx = Utils::getFile(sq2) - Utils::getFile(sq1);
		int dy = Utils::getRank(sq2) - Utils::getRank(sq1);

		// Same square
		if (dx == 0 && dy == 0) return NONE;

		// Cardinal directions
		if (dx == 0) return dy > 0 ? NORTH : SOUTH;
		if (dy == 0) return dx > 0 ? EAST : WEST;

		// Diagonal directions
		if (dx == dy)  return dx > 0 ? NORTH_EAST : SOUTH_WEST;
		if (dx == -dy) return dx > 0 ? SOUTH_EAST : NORTH_WEST;

		// Not aligned
		return NONE;
	}

	// Compute squares between two squares (exclusive)
	uint64_t computeBetween(int sq1, int sq2) {
		Direction d = getDirection(sq1, sq2);
		if (d == NONE) return 0ULL;

		uint64_t result = 0ULL;
		int current = sq1 + d;
		while (current != sq2) {
			result |= 1ULL << current;

			int prev_rank = Utils::getRank(current);
			current += d;
			int new_rank = Utils::getRank(current);

			// If moving EAST/WEST but changed rank, stop (wraparound check)
			if ((d == EAST || d == WEST) && prev_rank != new_rank) return 0ULL;

			// If moved outside the board, stop
			if (current < 0 || current >= 64) return 0ULL;
		}
		return result;
	}

	// Compute entire line through two squares (inclusive)
	uint64_t computeLine(int sq1, int sq2) {
		Direction d = getDirection(sq1, sq2);
		if (d == NONE) return 1ULL << sq1;  // Single square

		uint64_t result = 0ULL;

		// Scan in both directions from sq1
		for (int current = sq1; current >= 0 && current < 64; current += d) {
			result |= (1ULL << current);
			if (current == sq2) break;
			if ((d == EAST && Utils::getFile(current) == 7) ||
				(d == WEST && Utils::getFile(current) == 0) ||
				(d == NORTH_EAST && Utils::getFile(current) == 7) ||
				(d == NORTH_WEST && Utils::getFile(current) == 0) ||
				(d == SOUTH_EAST && Utils::getFile(current) == 7) ||
				(d == SOUTH_WEST && Utils::getFile(current) == 0)) break;
		}

		for (int current = sq1; current >= 0 && current < 64; current -= d) {
			result |= (1ULL << current);
			if (current == sq2) break;
			if ((d == EAST && Utils::getFile(current) == 0) ||
				(d == WEST && Utils::getFile(current) == 7) ||
				(d == NORTH_EAST && Utils::getFile(current) == 0) ||
				(d == NORTH_WEST && Utils::getFile(current) == 7) ||
				(d == SOUTH_EAST && Utils::getFile(current) == 0) ||
				(d == SOUTH_WEST && Utils::getFile(current) == 7)) break;
		}

		return result;
	}

	void initZobristKeys() {
		// Random number generator
		std::mt19937_64 rng(123456789);
		std::uniform_int_distribution<uint64_t> dist;

		// Init piece keys
		for (int color = BLACK; color <= WHITE; ++color) { // 0 = Black, 1 = White
			for (int piece = PAWN; piece <= KING; ++piece) {
				for (int square = 0; square < 64; ++square) {
					PIECE_KEYS[color][piece][square] = dist(rng);
				}
			}
		}

		// Init castling keys (one per bitmask value)
		for (int i = 0; i < 16; i++) {
			CASTLING_KEYS[i] = dist(rng);
		}

		// Init en passant keys (one per file)
		for (int i = 0; i < 8; i++) {
			EN_PASSANT_KEYS[i] = dist(rng);
		}

		// Side to move key
		SIDE_TO_MOVE_KEY = dist(rng);
	}

	void initTables() {
		// Initialize rays
		for (int sq1 = 0; sq1 < 64; sq1++) {
			for (int sq2 = 0; sq2 < 64; sq2++) {
				DIR[sq1][sq2] = getDirection(sq1, sq2);
				BETWEEN[sq1][sq2] = uint64_t(computeBetween(sq1, sq2));
				LINE[sq1][sq2] = uint64_t(computeLine(sq1, sq2));
			}
		}

		initZobristKeys();
	}

	void teardownTables() {
		delete[] HISTORY_TABLE;
	}
}
