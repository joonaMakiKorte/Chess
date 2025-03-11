#ifndef MOVETABLES_H
#define MOVETABLES_H

// Define move tables for pieces
struct PawnMoves {
	uint64_t single_push;  // Single forward move
	uint64_t double_push;  // Double forward move (only from rank 2 or 4)
	uint64_t captures;     // Both diagonal captures
};

struct BishopMoves {
	uint64_t top_left;     // Diagonal top-left direction
	uint64_t top_right;    // Diagonal top-right direction
	uint64_t bottom_left;  // Diagonal bottom-left direction
	uint64_t bottom_right; // Diagonal bottom-right direction
};

struct RookMoves {
	uint64_t top;          // Moving upwards
	uint64_t bottom;       // Moving downwards
	uint64_t left;         // Moving left
	uint64_t right;        // Moving right
};

struct KnightMoves {
	uint64_t moves;        // All possible jumps combined into one for simplicity
};

struct QueenMoves {
	// Has the combined moves of Rook and Bishop
	// Doesn't require it's own init function
	uint64_t top;         
	uint64_t bottom;
	uint64_t left;
	uint64_t right;
	uint64_t top_left;
	uint64_t top_right;
	uint64_t bottom_left;
	uint64_t bottom_right;
};

struct KingMoves {
	uint64_t moves;         // Can move one step in each direction
};


// Declare tables
extern PawnMoves WHITE_PAWN_MOVES[64];
extern PawnMoves BLACK_PAWN_MOVES[64];
extern BishopMoves BISHOP_MOVES[64];
extern RookMoves ROOK_MOVES[64];
extern KnightMoves KNIGHT_MOVES[64];
extern QueenMoves QUEEN_MOVES[64];
extern KingMoves KING_MOVES[64];

// Generate move tables at runtime
// Loops over the 64 squares of the chessboard and creates moveset for each piece at each location
// Calls every individual init function of each piece
void initMoveTables();

#endif // MOVETABLES_H