#include "pch.h"
#include "Moves.h"
#include "Magic.h"
#include "MoveTables.h"

uint64_t Moves::getPseudoLegalMoves(int square, PieceType piece, uint64_t occupied) {
	switch (piece)
	{
	case KNIGHT: return getKnightMoves(square);
	case BISHOP: return getBishopMoves(square, occupied);
	case ROOK: return getRookMoves(square, occupied);
	case QUEEN: return getQueenMoves(square, occupied);
	case KING: return getKingMoves(square);
	default: throw std::invalid_argument("Invalid piece type");
	}

	return 0ULL; // Should never reach here
}

uint64_t Moves::getPawnMoves(int pawn, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white, int en_passant) {
	uint64_t pawn_bb = 1ULL << pawn; // Convert index to bitboard

	uint64_t occupied = white_pieces | black_pieces; // Combine white and black occupancy with OR

	// Initialize moves
	uint64_t singlePush = 0ULL;
	uint64_t doublePush = 0ULL;
	uint64_t captures = 0ULL;

	if (white) { // White pawn
		singlePush = WHITE_PAWN_MOVES[pawn].single_push & ~occupied;
		doublePush = WHITE_PAWN_MOVES[pawn].double_push & ~occupied & (singlePush << 8); // Ensure single step is free
		captures = WHITE_PAWN_MOVES[pawn].captures & black_pieces; // Capture only black pieces
	}
	else { // Black pawn
		singlePush = BLACK_PAWN_MOVES[pawn].single_push & ~occupied;
		doublePush = BLACK_PAWN_MOVES[pawn].double_push & ~occupied & (singlePush >> 8);
		captures = BLACK_PAWN_MOVES[pawn].captures & white_pieces; // Capture only white pieces
	}

	// Check if en passant is available and in capture moves of the current moved piece
	if ((en_passant != UNASSIGNED) &&
		((white ? WHITE_PAWN_MOVES[pawn].captures : BLACK_PAWN_MOVES[pawn].captures) & (1ULL << en_passant))) {
		captures |= 1ULL << en_passant;
	}

	return singlePush | doublePush | captures;
}

uint64_t Moves::getPawnCaptures(int pawn, bool white) {
	uint64_t pawn_bb = 1ULL << pawn; // Convert index to bitboard

	// Initialize captures
	uint64_t captures = 0ULL;

	if (white) { // White pawn
		captures |= WHITE_PAWN_MOVES[pawn].captures;
	}
	else { // Black pawn
		captures |= BLACK_PAWN_MOVES[pawn].captures;
	}

	return captures;
}

uint64_t Moves::getKnightMoves(int knight) {
	uint64_t knight_bb = 1ULL << knight; // Convert index to bitboard

	// Get the precomputed knight moves for the square
	return KNIGHT_MOVES[knight].moves;
}

uint64_t Moves::getKingMoves(int king) {
	uint64_t king_bb = 1ULL << king; // Convert index to bitboard

	// Get the precomputed moves
	return KING_MOVES[king].moves;
}

uint64_t Moves::getBishopMoves(int bishop, uint64_t occ) {
	uint64_t bishop_bb = 1ULL << bishop;

	// Get possible attacks
	occ &= MAGIC_TABLE_BISHOP[bishop].mask;
	occ *= MAGIC_TABLE_BISHOP[bishop].magic;
	occ >>= MAGIC_TABLE_BISHOP[bishop].shift;
	return ATTACKS_BISHOP[bishop][occ];
}

uint64_t Moves::getRookMoves(int rook, uint64_t occ) {
	uint64_t rook_bb = 1ULL << rook;

	// Get possible attacks
	occ &= MAGIC_TABLE_ROOK[rook].mask;
	occ *= MAGIC_TABLE_ROOK[rook].magic;
	occ >>= MAGIC_TABLE_ROOK[rook].shift;
	return ATTACKS_ROOK[rook][occ];
}

uint64_t Moves::getQueenMoves(int queen, uint64_t occupied) {
	uint64_t queen_bb = 1ULL << queen;

	uint64_t occ = occupied;

	// Get rook moves
	occ &= MAGIC_TABLE_ROOK[queen].mask;
	occ *= MAGIC_TABLE_ROOK[queen].magic;
	occ >>= MAGIC_TABLE_ROOK[queen].shift;
	uint64_t moves = ATTACKS_ROOK[queen][occ];

	// Get bishop moves
	occ = occupied;
	occ &= MAGIC_TABLE_BISHOP[queen].mask;
	occ *= MAGIC_TABLE_BISHOP[queen].magic;
	occ >>= MAGIC_TABLE_BISHOP[queen].shift;
	moves |= ATTACKS_BISHOP[queen][occ]; // Combine with rook

	return moves;
}
