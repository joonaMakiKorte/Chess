#ifndef TABLES_H
#define TABLES_H

#include "BitboardConstants.hpp"
#include "CustomTypes.hpp"

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

	// Transposition Table for efficient alpha-beta pruning in minimax
	extern TTEntry* TRANSPOSITION_TABLE; 
	// Initialized on the heap for the large size
	extern size_t TT_NUM_ENTRIES; // Number of entries (will be power of 2)
	extern size_t TT_MASK;        // Mask for indexing (num_entries - 1)

	// Tables for zobrist hashing key generation
	extern uint64_t PIECE_KEYS[2][6][64]; // Piece position keys
	extern uint64_t SIDE_TO_MOVE_KEY;     // Side to move key
	extern uint64_t CASTLING_KEYS[16];    // Castling rights
	extern uint64_t EN_PASSANT_KEYS[8];   // En passant file

	// Generate all precomputed tables 
	extern std::atomic<bool> initialized; // Track re-initialization need
	void initTables();

	void teardownTables();
}

#endif