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

	// Compute direction between squares
	Direction get_direction(int sq1, int sq2) {
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
	uint64_t compute_between(int sq1, int sq2) {
		Direction d = get_direction(sq1, sq2);
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
	uint64_t compute_line(int sq1, int sq2) {
		Direction d = get_direction(sq1, sq2);
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

	void initTables() {
		// Initialize rays
		for (int sq1 = 0; sq1 < 64; sq1++) {
			for (int sq2 = 0; sq2 < 64; sq2++) {
				DIR[sq1][sq2] = get_direction(sq1, sq2);
				BETWEEN[sq1][sq2] = uint64_t(compute_between(sq1, sq2));
				LINE[sq1][sq2] = uint64_t(compute_line(sq1, sq2));
			}
		}
	}

	void teardownTables() {
		delete[] HISTORY_TABLE;
	}
}
