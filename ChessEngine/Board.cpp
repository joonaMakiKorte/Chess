#include "pch.h"
#include "Board.h"
#include "AttackTables.h"


Board::Board(): 
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

bool Board::isWhite() {
	return white;
}

char Board::getPieceType(uint64_t square) const {
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

std::string Board::getCastlingRightsString() const {
	std::string rights;
	if (castling_rights & 0x01) rights += 'K'; // White kingside
	if (castling_rights & 0x02) rights += 'Q'; // White queenside
	if (castling_rights & 0x04) rights += 'k'; // Black kingside
	if (castling_rights & 0x08) rights += 'q'; // Black queenside
	return rights.empty() ? "-" : rights;
}

std::string Board::getEnPassantString() const {
	std::string square;
	if (en_passant_target != UNASSIGNED) {
		square = squareToString(en_passant_target); // Transform to algebraic notation
	}
	else {
		square = "- "; // Represent as dash if none
	}
	return square;
}

int Board::getHalfMoveClock() const {
	return half_moves;
}

int Board::getFullMoveNumber() const {
	return full_moves;
}


uint64_t Board::getLegalMoves(uint64_t from) {
	uint64_t legal_moves = 0ULL;
	// Get the piece type at the source square
	char piece = getPieceType(from);
	switch (tolower(piece)) // Convert to lowercase, we use turn flag to determine if white or not
	{
	case 'p': legal_moves = getPawnMoves(from); break;
	default: throw std::invalid_argument("Invalid piece type");
	}

	return legal_moves;
}

const uint64_t Board::whitePieces() const {
	return white_pawns | white_rooks | white_knights |
		white_bishops | white_queen | white_king;

}

const uint64_t Board::blackPieces() const {
	return black_pawns | black_rooks | black_knights |
		black_bishops | black_queen | black_king;
}

const uint64_t Board::getOccupied() const {
	return whitePieces() | blackPieces();
}

std::string Board::squareToString(int square) const {
	char file = 'a' + (square % 8);
	char rank = '1' + (square / 8);
	return std::string() + file + rank;
}

uint64_t Board::getPawnMoves(uint64_t pawn) {
	if (pawn == 0) return 0; // No pawn present

    int square = std::bitset<64>(pawn).to_ullong(); // Get index of the pawn's position
	// Using bitwise OR operation get all moves from the location
	// Get moves from white or black pawn move table depending which turn is active
	return white ?
		(WHITE_PAWN_MOVES[square].single_push | WHITE_PAWN_MOVES[square].double_push | WHITE_PAWN_MOVES[square].captures)
	  : (BLACK_PAWN_MOVES[square].single_push | BLACK_PAWN_MOVES[square].double_push | BLACK_PAWN_MOVES[square].captures);
}

