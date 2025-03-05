#include "pch.h"
#include "ChessLogic.h"

ChessLogic::ChessLogic(): 
	castling_rights(0x0F),                 // All castling rights (0b00001111)
	en_passant_target(UNASSIGNED),         // None
	white(true),                           // White starts
	half_moves(0),                         // Initially 0
	full_moves(0)                          // Initially 0
{
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

bool ChessLogic::isWhite() {
	return white;
}

char ChessLogic::getPieceType(int square) const {
	char piece = '\0'; // Empty as default

	// Check each piece type
	if (white_pawns & (1ULL << square)) piece = 'P';
	else if (black_pawns & (1ULL << square)) piece = 'p';
	else if (white_knights & (1ULL << square)) piece = 'N';
	else if (black_knights & (1ULL << square)) piece = 'n';
	else if (white_bishops & (1ULL << square)) piece = 'B';
	else if (black_bishops & (1ULL << square)) piece = 'b';
	else if (white_rooks & (1ULL << square)) piece = 'R';
	else if (black_rooks & (1ULL << square)) piece = 'r';
	else if (white_queen & (1ULL << square)) piece = 'Q';
	else if (black_queen & (1ULL << square)) piece = 'q';
	else if (white_king & (1ULL << square)) piece = 'K';
	else if (black_king & (1ULL << square)) piece = 'k';

	return piece;
}

std::string ChessLogic::getCastlingRightsString() const {
	std::string rights;
	if (castling_rights & 0x01) rights += 'K'; // White kingside
	if (castling_rights & 0x02) rights += 'Q'; // White queenside
	if (castling_rights & 0x04) rights += 'k'; // Black kingside
	if (castling_rights & 0x08) rights += 'q'; // Black queenside
	return rights.empty() ? "-" : rights;
}

std::string ChessLogic::getEnPassantString() const {
	std::string square;
	if (en_passant_target != UNASSIGNED) {
		square = squareToString(en_passant_target); // Transform to algebraic notation
	}
	else {
		square = "- "; // Represent as dash if none
	}
	return square;
}

std::string ChessLogic::getHalfMoveString() const {
	return std::to_string(half_moves);
}

std::string ChessLogic::getFullMoveString() const {
	return std::to_string(full_moves);
}

const uint64_t ChessLogic::whitePieces() const {
	return white_pawns | white_rooks | white_knights |
		white_bishops | white_queen | white_king;

}

const uint64_t ChessLogic::blackPieces() const {
	return black_pawns | black_rooks | black_knights |
		black_bishops | black_queen | black_king;
}

const uint64_t ChessLogic::getOccupied() const {
	return whitePieces() | blackPieces();
}

std::string ChessLogic::squareToString(int square) const {
	char file = 'a' + (square % 8);
	char rank = '1' + (square / 8);
	return std::string() + file + rank;
}
