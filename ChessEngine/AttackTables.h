#ifndef ATTACKTABLES_H
#define ATTACKTABLES_H

// Define move tables for pieces
struct PawnMoves {
	uint64_t single_push; // Single forward move
	uint64_t double_push; // Double forward move (only from rank 2 or 4)
	uint64_t captures;    // Both diagonal captures
};

struct BishopMoves {
	uint64_t top_left;    // Diagonal top-left direction
	uint64_t top_right;   // Diagonal top-right direction
	uint64_t bottom_left; // Diagonal bottom-left direction
	uint64_t bottom_right; // Diagonal bottom-right direction
};

// Declare tables
extern PawnMoves WHITE_PAWN_MOVES[64];
extern PawnMoves BLACK_PAWN_MOVES[64];
extern PawnMoves BISHOP_MOVES[64];


// Generate move tables at runtime
// Loops over the 64 squares of the chessboard and creates moveset for each piece at each location
// Calls every individual init function of each piece
void initMoveTables();

#endif // ATTACKTABLES_H