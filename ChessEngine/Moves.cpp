#include "pch.h"
#include "Moves.h"
#include "Magic.h"
#include "MoveTables.h"

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
