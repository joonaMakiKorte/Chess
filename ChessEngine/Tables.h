#ifndef TABLES_H
#define TABLES_H

#include "BitboardConstants.h"
#include "CustomTypes.h"

namespace Tables {
	// Pre-compute all attack rays between squares
	extern uint64_t BETWEEN[64][64];
	extern uint64_t LINE[64][64];
	extern Direction DIR[64][64];

	// Killer move table: stores two best non-capture moves per depth
	extern uint16_t KILLER_MOVES[MAX_DEPTH][2];

	// History heuristic: assigns a score to quiet moves
	extern int* HISTORY_TABLE; // Use move keys for lookup (uint16_t)
	// Initialized on the heap for the large size

	// Tables for zobrist hashing key generation
	extern uint64_t PIECE_KEYS[2][6][64]; // Piece position keys
	extern uint64_t SIDE_TO_MOVE_KEY;     // Side to move key
	extern uint64_t CASTLING_KEYS[16];    // Castling rights
	extern uint64_t EN_PASSANT_KEYS[8];   // En passant file

	// Generate all precomputed tables 
	void initTables();

	void teardownTables();
}

#endif