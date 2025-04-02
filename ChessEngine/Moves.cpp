#include "pch.h"
#include "Moves.h"
#include "Magic.h"
#include "MoveTables.h"
#include "Tables.h"
#include "Utils.h"

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
		singlePush = MoveTables::WHITE_PAWN_MOVES[pawn].single_push & ~occupied;
		doublePush = MoveTables::WHITE_PAWN_MOVES[pawn].double_push & ~occupied & (singlePush << 8); // Ensure single step is free
		captures = MoveTables::WHITE_PAWN_MOVES[pawn].captures & black_pieces; // Capture only black pieces
	}
	else { // Black pawn
		singlePush = MoveTables::BLACK_PAWN_MOVES[pawn].single_push & ~occupied;
		doublePush = MoveTables::BLACK_PAWN_MOVES[pawn].double_push & ~occupied & (singlePush >> 8);
		captures = MoveTables::BLACK_PAWN_MOVES[pawn].captures & white_pieces; // Capture only white pieces
	}

	// Check if en passant is available and in capture moves of the current moved piece
	if ((en_passant != UNASSIGNED) &&
		((white ? MoveTables::WHITE_PAWN_MOVES[pawn].captures : MoveTables::BLACK_PAWN_MOVES[pawn].captures) & (1ULL << en_passant))) {
		captures |= 1ULL << en_passant;
	}

	return singlePush | doublePush | captures;
}

uint64_t Moves::getPawnCaptures(int pawn, bool white) {
	uint64_t pawn_bb = 1ULL << pawn; // Convert index to bitboard

	// Initialize captures
	uint64_t captures = 0ULL;

	if (white) { // White pawn
		captures |= MoveTables::WHITE_PAWN_MOVES[pawn].captures;
	}
	else { // Black pawn
		captures |= MoveTables::BLACK_PAWN_MOVES[pawn].captures;
	}

	return captures;
}

uint64_t Moves::getKnightMoves(int knight) {
	uint64_t knight_bb = 1ULL << knight; // Convert index to bitboard

	// Get the precomputed knight moves for the square
	return MoveTables::KNIGHT_MOVES[knight].moves;
}

uint64_t Moves::getKingMoves(int king) {
	uint64_t king_bb = 1ULL << king; // Convert index to bitboard

	// Get the precomputed moves
	return MoveTables::KING_MOVES[king].moves;
}

uint64_t Moves::getBishopMoves(int bishop, uint64_t occ) {
	uint64_t bishop_bb = 1ULL << bishop;

	// Get possible attacks
	occ &= Magic::MAGIC_TABLE_BISHOP[bishop].mask;
	occ *= Magic::MAGIC_TABLE_BISHOP[bishop].magic;
	occ >>= Magic::MAGIC_TABLE_BISHOP[bishop].shift;
	return MoveTables::ATTACKS_BISHOP[bishop][occ];
}

uint64_t Moves::getRookMoves(int rook, uint64_t occ) {
	uint64_t rook_bb = 1ULL << rook;

	// Get possible attacks
	occ &= Magic::MAGIC_TABLE_ROOK[rook].mask;
	occ *= Magic::MAGIC_TABLE_ROOK[rook].magic;
	occ >>= Magic::MAGIC_TABLE_ROOK[rook].shift;
	return MoveTables::ATTACKS_ROOK[rook][occ];
}

uint64_t Moves::getQueenMoves(int queen, uint64_t occupied) {
	uint64_t queen_bb = 1ULL << queen;

	uint64_t occ = occupied;

	// Get rook moves
	occ &= Magic::MAGIC_TABLE_ROOK[queen].mask;
	occ *= Magic::MAGIC_TABLE_ROOK[queen].magic;
	occ >>= Magic::MAGIC_TABLE_ROOK[queen].shift;
	uint64_t moves = MoveTables::ATTACKS_ROOK[queen][occ];

	// Get bishop moves
	occ = occupied;
	occ &= Magic::MAGIC_TABLE_BISHOP[queen].mask;
	occ *= Magic::MAGIC_TABLE_BISHOP[queen].magic;
	occ >>= Magic::MAGIC_TABLE_BISHOP[queen].shift;
	moves |= MoveTables::ATTACKS_BISHOP[queen][occ]; // Combine with rook

	return moves;
}

void Moves::computePinnedPieces(PinData& pin_data, const int& king_sq,
	const uint64_t& occupied, const uint64_t& bishops, const uint64_t& rooks, const uint64_t& queen) {
	// Reset pin data
	pin_data.pinned = 0;
	for (int i = 0; i < 64; i++) {
		pin_data.pin_rays[i] = 0xFFFFFFFFFFFFFFFFULL;  // Default to allow all moves
	}

	// All potential pinners
	uint64_t sliders = bishops | rooks | queen;

	// Get all rays from the square king is at
	// Meaning possible rays enemy could attack from
	// Done by getting possible queen moves at this square
	while (sliders) {
		int slider_sq = Utils::findFirstSetBit(sliders);
		Utils::popBit(sliders, slider_sq);
		Direction direction = Tables::DIR[king_sq][slider_sq];

		if (!direction) continue; // Not aligned

		// Bishop cannot pin vertically/horizontally
		// Rook cannot pin orthogonally
		// Queen can pin in every direction
		if ((direction == NORTH || direction == WEST || direction == SOUTH || direction == EAST) && bishops & (1ULL << slider_sq)) continue;
		if ((direction == NORTH_EAST || direction == NORTH_WEST || direction == SOUTH_EAST || direction == SOUTH_WEST) && rooks & (1ULL << slider_sq)) continue;


		uint64_t between_mask = Tables::BETWEEN[king_sq][slider_sq]; // Bitmask of squares between king and slider
		uint64_t blockers = between_mask & occupied; // Get pieces that align with between mask

		// Check if exactly one blocker exists and extract it in one step
		if (Utils::countSetBits(blockers) == 1) {
			int pinned_sq = Utils::findFirstSetBit(blockers); // Get the pinned piece
			pin_data.pinned |= (1ULL << pinned_sq); // Add to pinned
			pin_data.pin_rays[pinned_sq] = Tables::LINE[king_sq][slider_sq]; // Get the pin ray
		}
	}
}
