#include "pch.h"
#include "Moves.h"
#include "Magic.h"
#include "MoveTables.h"
#include "BitboardConstants.h"

uint64_t Moves::getPawnMoves(int pawn, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white, int en_passant) {
	uint64_t pawn_bb = 1ULL << pawn; // Convert index to bitboard
	// Make sure exists
	const uint64_t& friendly = white ? white_pieces : black_pieces;
	if (!(pawn_bb & friendly)) return 0ULL;

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

uint64_t Moves::getPawnCaptures(int pawn, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white, int en_passant) {
	uint64_t pawn_bb = 1ULL << pawn; // Convert index to bitboard
	// Make sure exists
	const uint64_t& friendly = white ? white_pieces : black_pieces;
	if (!(pawn_bb & friendly)) return 0ULL;

	// Initialize captures
	uint64_t captures = 0ULL;

	if (white) { // White pawn
		captures |= WHITE_PAWN_MOVES[pawn].captures;
	}
	else { // Black pawn
		captures |= BLACK_PAWN_MOVES[pawn].captures;
	}

	// Check if en passant is available and in capture moves of the current moved piece
	if ((en_passant != UNASSIGNED) &&
		((white ? WHITE_PAWN_MOVES[pawn].captures : BLACK_PAWN_MOVES[pawn].captures) & (1ULL << en_passant))) {
		captures |= 1ULL << en_passant;
	}

	return captures;
}

uint64_t Moves::getKnightMoves(int knight, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white) {
	uint64_t knight_bb = 1ULL << knight; // Convert index to bitboard
	// Make sure exists
	const uint64_t& friendly = white ? white_pieces : black_pieces;
	if (!(knight_bb & friendly)) return 0ULL;

	// Get the precomputed knight moves for the square
	uint64_t moves = KNIGHT_MOVES[knight].moves;

	// Exclude friendly pieces from moves
	moves &= ~friendly;

	return moves;
}

uint64_t Moves::getKingMoves(int king, uint64_t white_pieces, uint64_t black_pieces, bool white) {
	return 0;
}

uint64_t Moves::getBishopMoves(int bishop, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white) {
	uint64_t bishop_bb = 1ULL << bishop;
	// Make sure exists
	const uint64_t& friendly = white ? white_pieces : black_pieces;
	if (!(bishop_bb & friendly)) return 0ULL;

	uint64_t occ = white_pieces | black_pieces;

	// Get possible attacks
	occ &= MAGIC_TABLE_BISHOP[bishop].mask;
	occ *= MAGIC_TABLE_BISHOP[bishop].magic;
	occ >>= MAGIC_TABLE_BISHOP[bishop].shift;
	uint64_t moves = ATTACKS_BISHOP[bishop][occ];

	// Exclude friendly pieces from moves
	moves &= ~friendly;

	return moves;
}

uint64_t Moves::getRookMoves(int rook, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white) {
	uint64_t rook_bb = 1ULL << rook;
	// Make sure exists
	const uint64_t& friendly = white ? white_pieces : black_pieces;
	if (!(rook_bb & friendly)) return 0ULL;

	uint64_t occ = white_pieces | black_pieces;

	// Get possible attacks
	occ &= MAGIC_TABLE_ROOK[rook].mask;
	occ *= MAGIC_TABLE_ROOK[rook].magic;
	occ >>= MAGIC_TABLE_ROOK[rook].shift;
	uint64_t moves = ATTACKS_ROOK[rook][occ];

	// Exclude friendly pieces from moves
	moves &= ~friendly;

	return moves;
}

uint64_t Moves::getQueenMoves(int queen, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white) {
	uint64_t queen_bb = 1ULL << queen;
	// Make sure exists
	const uint64_t& friendly = white ? white_pieces : black_pieces;
	if (!(queen_bb & friendly)) return 0ULL;

	// Get rook moves
	uint64_t occ = white_pieces | black_pieces;
	occ &= MAGIC_TABLE_ROOK[queen].mask;
	occ *= MAGIC_TABLE_ROOK[queen].magic;
	occ >>= MAGIC_TABLE_ROOK[queen].shift;
	uint64_t moves = ATTACKS_ROOK[queen][occ];

	// Get bishop moves
	occ = white_pieces | black_pieces;
	occ &= MAGIC_TABLE_BISHOP[queen].mask;
	occ *= MAGIC_TABLE_BISHOP[queen].magic;
	occ >>= MAGIC_TABLE_BISHOP[queen].shift;
	moves |= ATTACKS_BISHOP[queen][occ]; // Combine with rook

	// Exclude friendly pieces from moves
	moves &= ~friendly;

	return moves;
}
