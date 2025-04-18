#ifndef MOVETABLES_H
#define MOVETABLES_H

namespace MoveTables {
	// Define move tables for non-sliding pieces
	struct PawnMoves {
		uint64_t single_push;  // Single forward move
		uint64_t double_push;  // Double forward move (only from rank 2 or 4)
		uint64_t captures;     // Both diagonal captures
	};

	struct KnightMoves {
		uint64_t moves;        // All possible jumps combined into one for simplicity
	};

	struct KingMoves {
		uint64_t moves;         // Can move one step in each direction
	};

	// Declare tables
	extern PawnMoves WHITE_PAWN_MOVES[64];
	extern PawnMoves BLACK_PAWN_MOVES[64];
	extern KnightMoves KNIGHT_MOVES[64];
	extern KingMoves KING_MOVES[64];

	// Bishops and rook moves are initialize on the heap because of the large size
	extern uint64_t(*ATTACKS_BISHOP)[512];
	extern uint64_t(*ATTACKS_ROOK)[4096];

	// Generate move tables at runtime
	// Loops over the 64 squares of the chessboard and creates moveset for each piece at each location
	// Calls every individual init function of each piece
	extern std::atomic<bool> initialized; // Flag to track need of re-initialization
	void initMoveTables();

	// Deallocate slider tables when program exits
	void teardownMoveTables();
}

#endif // MOVETABLES_H