#include "pch.h"
#include "Bitboard.h"
#include "MoveTables.h"


Bitboard::Bitboard():
	castling_rights(0x0F),                 // All castling rights (0b00001111)
	en_passant_target(UNASSIGNED),         // None
	white(true),                           // White starts
	half_moves(0),                         // Initially 0
	full_moves(0)                          // Initially 0
{
	// Standard little-endian rank-file mapping (LSB = a1, MSB = h8)
	white_pawns = 0x000000000000FF00;      // a2, b2, c2, d2, e2, f2, g2, h2
	black_pawns = 0x00FF000000000000;      // a7, b7, c7, d7, e7, f7, g7, h7
	white_rooks = 0x0000000000000081;      // a1, h1
	black_rooks = 0x8100000000000000;      // a8, h8
	white_knights = 0x0000000000000042;    // b1, g1
	black_knights = 0x4200000000000000;    // b8, g8
	white_bishops = 0x0000000000000024;    // c1, f1
	black_bishops = 0x2400000000000000;    // c8, f8
	white_queen = 0x0000000000000010;      // d1
	black_queen = 0x1000000000000000;      // d8
	white_king = 0x0000000000000008;       // e1
	black_king = 0x0800000000000000;       // e8
}

bool Bitboard::isWhite() {
	return white;
}

char Bitboard::getPieceType(int square_int) const {
	uint64_t square = 1ULL << square_int; // Cast to bitboard

	if (white_pawns & square) return 'P';
	if (black_pawns & square) return 'p';
	if (white_knights & square) return 'N';
	if (black_knights & square) return 'n';
	if (white_bishops & square) return 'B';
	if (black_bishops & square) return 'b';
	if (white_rooks & square) return 'R';
	if (black_rooks & square) return 'r';
	if (white_queen & square) return 'Q';
	if (black_queen & square) return 'q';
	if (white_king & square) return 'K';
	if (black_king & square) return 'k';
	return '\0'; // Empty square
}

std::string Bitboard::getCastlingRightsString() const {
	std::string rights;
	if (castling_rights & 0x01) rights += 'K'; // White kingside
	if (castling_rights & 0x02) rights += 'Q'; // White queenside
	if (castling_rights & 0x04) rights += 'k'; // Black kingside
	if (castling_rights & 0x08) rights += 'q'; // Black queenside
	return rights.empty() ? "-" : rights;
}

std::string Bitboard::getEnPassantString() const {
	std::string square;
	if (en_passant_target != UNASSIGNED) {
		square = squareToString(en_passant_target); // Transform to algebraic notation
	}
	else {
		square = "- "; // Represent as dash if none
	}
	return square;
}

int Bitboard::getHalfMoveClock() const {
	return half_moves;
}

int Bitboard::getFullMoveNumber() const {
	return full_moves;
}


uint64_t Bitboard::getLegalMoves(int from) {
	if (from == 0) return 0ULL; // Invalid source square

	char piece = getPieceType(from); // Get piece type at square

	uint64_t legal_moves = 0ULL;
	switch (tolower(piece)) // Convert to lowercase, we use turn flag to determine if white or not
	{
	case 'p': legal_moves = getPawnMoves(from); break;
	/*
	* TODO: IMPLEMENT THE REST
	*/
	case 'n': break;
	case 'b': break;
	case 'r': break;
	case 'q': break;
	case 'k': break;
	default: throw std::invalid_argument("Invalid piece type");
	}

	return legal_moves;
}

uint64_t Bitboard::whitePieces() {
	return white_pawns | white_rooks | white_knights |
		white_bishops | white_queen | white_king;
}

uint64_t Bitboard::blackPieces() {
	return black_pawns | black_rooks | black_knights |
		black_bishops | black_queen | black_king;
}

std::string Bitboard::squareToString(int square) const {
	char file = 'a' + (square % 8);
	char rank = '1' + (square / 8);
	return std::string() + file + rank;
}

uint64_t Bitboard::getPawnMoves(int square) {
	uint64_t pawn_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_pawns & pawn_bitboard) == 0 && (black_pawns & pawn_bitboard) == 0) {
		return 0ULL; // No pawn exists at this square
	}

	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	uint64_t occupied = white_pieces | black_pieces;

	if (white_pawns & pawn_bitboard) { // White pawn
		uint64_t singlePush = WHITE_PAWN_MOVES[square].single_push & ~occupied;
		uint64_t doublePush = WHITE_PAWN_MOVES[square].double_push & ~occupied & (singlePush << 8); // Ensure single step is free
		uint64_t captures = WHITE_PAWN_MOVES[square].captures & black_pieces; // Capture only black pieces

		return singlePush | doublePush | captures;
	}
	else if (black_pawns & pawn_bitboard) { // Black pawn
		uint64_t singlePush = BLACK_PAWN_MOVES[square].single_push & ~occupied;
		uint64_t doublePush = BLACK_PAWN_MOVES[square].double_push & ~occupied & (singlePush >> 8);
		uint64_t captures = BLACK_PAWN_MOVES[square].captures & white_pieces; // Capture only white pieces

		return singlePush | doublePush | captures;
	}

	return 0ULL; // Should never reach here
}

uint64_t Bitboard::getKnightMoves(uint64_t square)
{
	uint64_t knight_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_knights & knight_bitboard) == 0 && (black_knights & knight_bitboard) == 0) {
		return 0ULL; // No knight exists at this square
	}

	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	uint64_t occupied = white_pieces | black_pieces;

	uint64_t moves = Knight_Moves[square].moves & ~occupied; // Remove occupied squares

	if (white_knights & knight_bitboard) {
		moves &= ~white_pieces; // White knight can't move onto white pieces
	}
	else if (black_knights & knight_bitboard) {
		moves &= ~black_pieces; // Black knight can't move onto black pieces
	}

	return moves;
}

uint64_t Bitboard::getBishopMoves(uint64_t square)
{
	uint64_t bishop_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_bishops & bishop_bitboard) == 0 && (black_bishops & bishop_bitboard) == 0) {
		return 0ULL; // No bishop exists at this square
	}

	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	uint64_t occupied = white_pieces | black_pieces;

	uint64_t moves = (BISHOP_MOVES[square].top_left |
		BISHOP_MOVES[square].top_right |
		BISHOP_MOVES[square].bottom_left |
		BISHOP_MOVES[square].bottom_right) & ~occupied; // Remove occupied squares

	if (white_bishops & bishop_bitboard) {
		moves &= ~white_pieces; // White bishop can't move onto white pieces
	}
	else if (black_bishops & bishop_bitboard) {
		moves &= ~black_pieces; // Black bishop can't move onto black pieces
	}

	return moves;
}

uint64_t Bitboard::getRookMoves(uint64_t square)
{
	uint64_t rook_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_rooks & rook_bitboard) == 0 && (black_rooks & rook_bitboard) == 0) {
		return 0ULL; // No rook exists at this square
	}

	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	uint64_t occupied = white_pieces | black_pieces;

	uint64_t moves = (Rook_Moves[square].upwards & ~occupied |
		Rook_Moves[square].downwards & ~occupied |
		Rook_Moves[square].right & ~occupied |
		Rook_Moves[square].left & ~occupied);

	if (white_rooks & rook_bitboard) {
		moves &= ~white_pieces; // White rook can't move onto white pieces
	}
	else if (black_rooks & rook_bitboard) {
		moves &= ~black_pieces; // Black rook can't move onto black pieces
	}

	return moves;
}

uint64_t Bitboard::getQueenMoves(uint64_t square)
{
	uint64_t queen_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_queen & queen_bitboard) == 0 && (black_queen & queen_bitboard) == 0) {
		return 0ULL; // No queen exists at this square
	}

	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	uint64_t occupied = white_pieces | black_pieces;

	uint64_t moves = 0ULL;

	if (white_queen & queen_bitboard) { // White queen
		// For white queen, combine all possible moves (top, bottom, left, right, etc.)
		moves |= (Queen_Moves[square].top & ~occupied);
		moves |= (Queen_Moves[square].bottom & ~occupied);
		moves |= (Queen_Moves[square].left & ~occupied);
		moves |= (Queen_Moves[square].right & ~occupied);
		moves |= (Queen_Moves[square].top_left & ~occupied);
		moves |= (Queen_Moves[square].top_right & ~occupied);
		moves |= (Queen_Moves[square].bottom_left & ~occupied);
		moves |= (Queen_Moves[square].bottom_right & ~occupied);
	}
	else if (black_queen & queen_bitboard) { // Black queen
		// For black queen, combine all possible moves (top, bottom, left, right, etc.)
		moves |= (Queen_Moves[square].top & ~occupied);
		moves |= (Queen_Moves[square].bottom & ~occupied);
		moves |= (Queen_Moves[square].left & ~occupied);
		moves |= (Queen_Moves[square].right & ~occupied);
		moves |= (Queen_Moves[square].top_left & ~occupied);
		moves |= (Queen_Moves[square].top_right & ~occupied);
		moves |= (Queen_Moves[square].bottom_left & ~occupied);
		moves |= (Queen_Moves[square].bottom_right & ~occupied);
	}

	return moves;
}



uint64_t Bitboard::getKingMoves(uint64_t square)
{
	uint64_t king_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_king & king_bitboard) == 0 && (black_king & king_bitboard) == 0) {
		return 0ULL; // No king exists at this square
	}

	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	uint64_t occupied = white_pieces | black_pieces;

	uint64_t moves = King_Moves[square].moves & ~occupied; // Remove occupied squares

	if (white_king & king_bitboard) {
		moves &= ~white_pieces; // White king can't move onto white pieces
	}
	else if (black_king & king_bitboard) {
		moves &= ~black_pieces; // Black king can't move onto black pieces
	}

	return moves;
}






