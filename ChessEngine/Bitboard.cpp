#include "pch.h"
#include "Bitboard.h"
#include "MoveTables.h"
#include <iostream>   // For std::cout, std::endl
#include <string>     // For std::string
#include <stdexcept>  // For std::invalid_argument
#include <cstdint>    // For uint64_t and other fixed-width integers
#include <algorithm>  // For std::tolower
#include <cmath>      // For std::abs


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
	white_queen = 0x0000000000000008;      // d1
	black_queen = 0x0800000000000000;      // d8
	white_king = 0x0000000000000010;       // e1
	black_king = 0x1000000000000000;       // e8
}

bool Bitboard::isWhite() {
	return white;
}

void Bitboard::switchTurn() {
	white = !white;
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
		square = "-"; // Represent as dash if none
	}
	return square;
}


bool Bitboard::isInCheck() {
	uint64_t king_bitboard = white ? white_king : black_king;
	return (king_bitboard & getAttackSquares());
}


bool Bitboard::isCheckmate() {
	if (!isInCheck()) {
		return false; // Not in check, so not checkmate

	}

	// Get currently possible king moves
	uint64_t king_bitboard = white ? white_king : black_king;
	int king_square = findFirstSetBit(king_bitboard);
	uint64_t king_moves = getKingMoves(king_square);
	uint64_t enemy_attacks = getAttackSquares(); // Squares attacked by enemy

	// Check if the king can escape (moves to a non-attacked square)
	if (king_moves & ~enemy_attacks) return false;

	return true;
}

int Bitboard::getHalfMoveClock() const {
	return half_moves;
}

int Bitboard::getFullMoveNumber() const {
	return full_moves;
}


uint64_t Bitboard::getLegalMoves(int from) {
	char piece = getPieceType(from); // Get piece type at square

	uint64_t legal_moves = 0ULL;
	switch (tolower(piece)) // Convert to lowercase
	{
	case 'p': legal_moves = getPawnMoves(from); break;
	case 'n': legal_moves = getKnightMoves(from); break;
	case 'b': legal_moves = getBishopMoves(from); break;
	case 'r': legal_moves = getRookMoves(from); break;
	case 'q': legal_moves = getQueenMoves(from); break;
	case 'k': 
		legal_moves = getKingMoves(from);
		
		// Add castling moves if the king is on its starting square
		if ((piece == 'K' && from == 4) || piece == 'k' && from == 60) {
			legal_moves |= getCastlingMoves();
		}
		break;
	default: throw std::invalid_argument("Invalid piece type");
	}
	return legal_moves;
}

void Bitboard::applyMove(int source, int target) {
	uint64_t source_square = 1ULL << source; // Convert source to bitboard
	uint64_t target_square = 1ULL << target; // Convert target

	// Get piece types at squares
	char source_piece = getPieceType(source);
	char target_piece = getPieceType(target);

	// Update the bitboard of the piece moved
	// Clears source square using bitwise AND with the negation source_square
	// Sets the target using bitwise OR with target_square

	// Create lambda for the moving operations
	auto movePiece = [source_square, target_square](uint64_t& piece_bitboard) {
		piece_bitboard &= ~source_square; // Clear source square
		piece_bitboard |= target_square; // Set target square
	};

	// Before applying move check if target is an empty square
	bool empty = target_piece == '\0';

	++half_moves; // Increment half moves
	++full_moves; // Increment full moves

	// Call movePiece depending which turn ongoing
	switch (tolower(source_piece))
	{
	case 'p': white ? movePiece(white_pawns) : movePiece(black_pawns); half_moves = 0; break; // Resets halfmove-clock
	case 'n': white ? movePiece(white_knights) : movePiece(black_knights); break;
	case 'b': white ? movePiece(white_bishops) : movePiece(black_bishops); break;
	case 'q': white ? movePiece(white_queen) : movePiece(black_queen); break;
	case 'r': white ? movePiece(white_rooks) : movePiece(black_rooks);
		// Update Queen/Kingside castling depending on which rook moved
		if (castling_rights != 0) {
			if (white && (castling_rights & 0x01 || castling_rights & 0x02)) {
				if (source % 8 == 0) { // Queenside
					castling_rights &= ~0x02;
				}
				else if (source % 7 == 0) { // Kingside
					castling_rights &= ~0x01;
				}
			}
			else if (!white && (castling_rights & 0x04 || castling_rights & 0x08)) {
				if (source % 8 == 0) { // Queenside
					castling_rights &= ~0x08;
				}
				else if (source % 8 == 7) { // Kingside
					castling_rights &= ~0x04;
				}
			}
		}
		break;
	case 'k': white ? movePiece(white_king) : movePiece(black_king);
		// Update castling rights
		if (castling_rights != 0) {
			castling_rights &= ~(white ? 0x03 : 0x0C); // (0x03 = white, 0x0C black)
		}
		// Check if move was castling
		// If so, move the correct rook also
		if (abs(source - target) == 2) {
			uint64_t rook;
			if (white) {
				// White castling: Kingside (h1 -> f1), Queenside (a1 -> d1)
				if (target == 6) { // Kingside castling (g1)
					rook = (1ULL << 7); // White rook on h1
					white_rooks &= ~rook; // Remove rook from h1
					white_rooks |= (rook >> 2); // Move rook to f1
				}
				else if (target == 2) { // Queenside castling (c1)
					rook = (1ULL << 0); // White rook on a1
					white_rooks &= ~rook; // Remove rook from a1
					white_rooks |= (rook << 3); // Move rook to d1
				}
			}
			else {
				// Black castling: Kingside (h8 -> f8), Queenside (a8 -> d8)
				if (target == 62) { // Kingside castling (g8)
					rook = (1ULL << 63); // Black rook on h8
					black_rooks &= ~rook; // Remove rook from h8
					black_rooks |= (rook >> 2); // Move rook to f8
				}
				else if (target == 58) { // Queenside castling (c8)
					rook = (1ULL << 56); // Black rook on a8
					black_rooks &= ~rook; // Remove rook from a8
					black_rooks |= (rook << 3); // Move rook to d8
				}
			}
		}
		break;
	default: throw std::invalid_argument("Invalid piece type");
	}

	// Check if the move was a pawn double push
	// If so, set en passant target square
	if (tolower(source_piece) == 'p' && abs(source - target) == 16) {
		// Set en passant depending on which color moved
		en_passant_target = white ? (source + 8) : (target + 8);
	}
	else if (!(tolower(source_piece) == 'p' && target == en_passant_target)) { // Reset if was not a pawn moved to en passant
		en_passant_target = UNASSIGNED; 
	}

	// Early exit if piece was moved to an empty square
	// Except if it was moved to an en passant target square
	if (empty && target != en_passant_target) return;

	// Separate normal capturing logic and en passant capturing
	if (!empty) { 
		// Lambda for capturing operation
		auto capturePiece = [target_square](uint64_t& piece_bitboard) {
			piece_bitboard &= ~target_square; // Clear captured square
		};

		// Call capturePiece depending on which turn ongoing
		switch (tolower(target_piece))
		{
		case 'p': white ? capturePiece(black_pawns) : capturePiece(white_pawns); break;
		case 'n': white ? capturePiece(black_knights) : capturePiece(white_knights); break;
		case 'b': white ? capturePiece(black_bishops) : capturePiece(white_bishops); break;
		case 'r': white ? capturePiece(black_rooks) : capturePiece(white_rooks); break;
		case 'q': white ? capturePiece(black_queen) : capturePiece(white_queen); break;
		case 'k': white ? capturePiece(black_king) : capturePiece(white_king); break;
		default: throw std::invalid_argument("Invalid piece type");
		}
	}
	else if (tolower(source_piece) == 'p' && target == en_passant_target) { // Case where we capture a pawn by en passant
		// Captured pawn is one rank behind the target square
		uint64_t en_passant_pawn = white ? (1ULL << (target - 8)) : (1ULL << (target + 8));
		if (white) {
			black_pawns &= ~en_passant_pawn; // Capture black pawn
		}
		else {
			white_pawns &= ~en_passant_pawn; // Capture white pawn
		}
	}
	half_moves = 0; // Captures reset halfmove-clock
	en_passant_target = UNASSIGNED; // Reset en passant; 
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
	uint64_t occupied = white_pieces | black_pieces; // Combine white and black occupancy with OR

	// Initialize moves
	uint64_t singlePush = 0ULL;
	uint64_t doublePush = 0ULL;
	uint64_t captures = 0ULL;

	if (white_pawns & pawn_bitboard) { // White pawn
		singlePush = WHITE_PAWN_MOVES[square].single_push & ~occupied;
		doublePush = WHITE_PAWN_MOVES[square].double_push & ~occupied & (singlePush << 8); // Ensure single step is free
		captures = WHITE_PAWN_MOVES[square].captures & black_pieces; // Capture only black pieces
	}
	else if (black_pawns & pawn_bitboard) { // Black pawn
		singlePush = BLACK_PAWN_MOVES[square].single_push & ~occupied;
		doublePush = BLACK_PAWN_MOVES[square].double_push & ~occupied & (singlePush >> 8);
		captures = BLACK_PAWN_MOVES[square].captures & white_pieces; // Capture only white pieces
	}

	// Check if en passant is available and in capture moves of the current moved piece
	if ((en_passant_target != UNASSIGNED) &&
		((white ? WHITE_PAWN_MOVES[square].captures : BLACK_PAWN_MOVES[square].captures) & (1ULL << en_passant_target))) {
		captures |= 1ULL << en_passant_target;
	}

	return singlePush | doublePush | captures;
}

uint64_t Bitboard::getPawnCaptures(int square) {
	uint64_t pawn_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_pawns & pawn_bitboard) == 0 && (black_pawns & pawn_bitboard) == 0) {
		return 0ULL; // No pawn exists at this square
	}

	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	uint64_t occupied = white_pieces | black_pieces; // Combine white and black occupancy with OR

	// Initialize captures
	uint64_t captures = 0ULL;

	if (white_pawns & pawn_bitboard) { // White pawn
		captures = WHITE_PAWN_MOVES[square].captures & black_pieces; // Capture only black pieces
	}
	else if (black_pawns & pawn_bitboard) { // Black pawn
		captures = BLACK_PAWN_MOVES[square].captures & white_pieces; // Capture only white pieces
	}

	// Check if en passant is available and in capture moves of the current moved piece
	if ((en_passant_target != UNASSIGNED) &&
		((white ? WHITE_PAWN_MOVES[square].captures : BLACK_PAWN_MOVES[square].captures) & (1ULL << en_passant_target))) {
		captures |= 1ULL << en_passant_target;
	}

	return captures;
}


uint64_t Bitboard::getKnightMoves(int square) {
	uint64_t knight_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_knights & knight_bitboard) == 0 && (black_knights & knight_bitboard) == 0) {
		return 0ULL; // No knight exists at this square
	}

	if (white_knights & knight_bitboard) {
		return KNIGHT_MOVES[square].moves &= ~whitePieces(); // White knight can't move onto white pieces
	}
	else if (black_knights & knight_bitboard) {
		return KNIGHT_MOVES[square].moves &= ~blackPieces(); // Black knight can't move onto black pieces
	}

	return 0ULL; // Should never reach here
}

uint64_t Bitboard::getBishopMoves(int square) {
	uint64_t bishop_bitboard = 1ULL << square; // Convert index to bitboard

	if ((white_bishops & bishop_bitboard) == 0 && (black_bishops & bishop_bitboard) == 0) {
		return 0ULL; // No bishop exists at this square
	}

	// Initialize moves
	uint64_t moves = 0ULL;

	// Combine legal moves
	moves |= getSlidingMoves(BISHOP_MOVES[square].top_left, true);
	moves |= getSlidingMoves(BISHOP_MOVES[square].top_right, true);
	moves |= getSlidingMoves(BISHOP_MOVES[square].bottom_left, false);
	moves |= getSlidingMoves(BISHOP_MOVES[square].bottom_right, false);

	return moves;
}

uint64_t Bitboard::getRookMoves(int square) {
	uint64_t rook_bitboard = 1ULL << square; // Convert index to bitboard

	if ((white_rooks & rook_bitboard) == 0 && (black_rooks & rook_bitboard) == 0) {
		return 0ULL; // No rook exists at this square
	}

	// Initialize moves
	uint64_t moves = 0ULL;

	// Combine legal moves
	moves |= getSlidingMoves(ROOK_MOVES[square].top, true);
	moves |= getSlidingMoves(ROOK_MOVES[square].bottom, false);
	moves |= getSlidingMoves(ROOK_MOVES[square].left, false);
	moves |= getSlidingMoves(ROOK_MOVES[square].right, true);

	return moves;
}

uint64_t Bitboard::getQueenMoves(int square) {
	uint64_t queen_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_queen & queen_bitboard) == 0 && (black_queen & queen_bitboard) == 0) {
		return 0ULL; // No queen exists at this square
	}

	// Initialize moves
	uint64_t moves = 0ULL;

	// Combine legal moves
	moves |= getSlidingMoves(QUEEN_MOVES[square].top, true);
	moves |= getSlidingMoves(QUEEN_MOVES[square].bottom, false);
	moves |= getSlidingMoves(QUEEN_MOVES[square].left, false);
	moves |= getSlidingMoves(QUEEN_MOVES[square].right, true);
	moves |= getSlidingMoves(QUEEN_MOVES[square].top_left, true);
	moves |= getSlidingMoves(QUEEN_MOVES[square].top_right, true);
	moves |= getSlidingMoves(QUEEN_MOVES[square].bottom_left, false);
	moves |= getSlidingMoves(QUEEN_MOVES[square].bottom_right, false);

	return moves;
}

uint64_t Bitboard::getKingMoves(int square) {
	uint64_t king_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_king & king_bitboard) == 0 && (black_king & king_bitboard) == 0) {
		return 0ULL; // No king exists at this square
	}

	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();

	// Initialize moves
	uint64_t moves = KING_MOVES[square].moves;

	if (white_king & king_bitboard) {
		moves &= ~white_pieces; // White king can't move onto white pieces
	}
	else if (black_king & king_bitboard) {
		moves &= ~black_pieces; // Black king can't move onto black pieces
	}

	return moves; 
}

uint64_t Bitboard::getSlidingMoves(uint64_t direction_moves, bool reverse) {
	uint64_t same_color = white ? whitePieces() : blackPieces(); // Determine which color is being moved
	uint64_t occupied = whitePieces() | blackPieces(); // All occupied squares

	uint64_t valid_moves = 0ULL; // Initialize moves
	while (direction_moves) {
		uint64_t next_square = 0ULL;
		if (reverse) {
			next_square = 1ULL << findFirstSetBit(direction_moves); // Isolate FSB
		}
		else {
			next_square = 1ULL << findLastSetBit(direction_moves); // Isolate LSB
		}
		if (same_color & next_square) break; // Stop if a same-color piece is encountered
		valid_moves |= next_square; // Add the square to valid moves
		if (occupied & next_square) break; // Stop if a piece is encountered (opposite color, capture)
		direction_moves ^= next_square; // Remove the processed square with bitwise XOR
	}
	return valid_moves;
}

uint64_t Bitboard::getCastlingMoves() {
	// Initialize castling moves
	uint64_t castling_moves = 0ULL;
	uint64_t occupied = whitePieces() | blackPieces();
	uint64_t attack_squares = getAttackSquares();

	uint64_t king_square = white ? WHITE_KING : BLACK_KING;
	if (attack_squares & king_square) {
		// King is in check, cannot castle
		return 0ULL;
	}

	uint64_t critical_squares;
	// Depending on player turn and castling availability add available castling moves
	if (white) {
		if (castling_rights & 0x01) { // White Kingside
			if ((occupied & WHITE_KINGSIDE_CASTLE_SQUARES) == 0) { // f1 and g1 must be free
				// King cannot castle out of, throught, or into check
				// Get squares that can't be under attack
				critical_squares = WHITE_KING | WHITE_KINGSIDE_CASTLE_SQUARES;

				// Now compare with opponents attack squares and make sure no squares alignt (bitwise AND)
				// // Ensure the king does not move thorught or into check
				// If castling available, add to moves
				if (!(critical_squares & attack_squares)) {
					castling_moves |= 1ULL << 6; // King moves to g1
				}
			}
		}
		if (castling_rights & 0x02) { // White Queenside
			if ((occupied & WHITE_QUEENSIDE_CASTLE_SQUARES) == 0) { // b1, c1 and d1 must be free
				critical_squares = WHITE_KING | WHITE_QUEENSIDE_CASTLE_SQUARES;
				if (!(critical_squares & attack_squares)) {
					castling_moves |= 1ULL << 2; // King moves to c1
				}
			}
		}
	}
	else {
		if (castling_rights & 0x04) { // Black Kingside
			if ((occupied & BLACK_KINGSIDE_CASTLE_SQUARES) == 0) { // f8 and g8 must be free
				critical_squares = BLACK_KING | BLACK_KINGSIDE_CASTLE_SQUARES; // e8, f8, g8
				if (!(critical_squares & attack_squares)) {
					castling_moves |= 1ULL << 62; // King moves to g8
				}
			}
		}
		if (castling_rights & 0x08) { // Black Queenside
			if ((occupied & BLACK_QUEENSIDE_CASTLE_SQUARES) == 0) { // c8 and d8 must be free
				critical_squares = BLACK_KING | BLACK_QUEENSIDE_CASTLE_SQUARES; // e8, d8, c8
				if (!(critical_squares & attack_squares)) {
					castling_moves |= 1ULL << 58; // King moves to c8
				}
			}
		}
	}

	return castling_moves;
}

uint64_t Bitboard::getAttackSquares() {
	// Initialize squares
	uint64_t attack_squares = 0ULL;

	// Iterate over opponent pieces
	// Done by isolating FSB, processing the piece type at square, and removing the processed square (XOR)
	// At each occupied square we get the moves and combine in the attack squares (OR)
	uint64_t opponent = white ? blackPieces() : whitePieces();
	// Bruteforce the turn flag to get the correct attack squares
	// This is done because the getMoves() functions use the flag to determine which pieces we can move onto
	// And since we discover the opponents moves, we must tell the functions that it would be the opponent moving
	white = !white;
	while (opponent != 0) { 
		int current_square = findFirstSetBit(opponent); // Isolate FSB and get as index
		char piece_type = getPieceType(current_square); // Get piece type
		// Get moves depending on the piece type
		switch (tolower(piece_type))
		{
		case 'p': attack_squares |= getPawnCaptures(current_square); break;
		case 'n': attack_squares |= getKnightMoves(current_square); break;
		case 'b': attack_squares |= getBishopMoves(current_square); break;
		case 'r': attack_squares |= getRookMoves(current_square); break;
		case 'q': attack_squares |= getQueenMoves(current_square); break;
		case 'k': attack_squares |= getKingMoves(current_square); break;
		default: throw std::invalid_argument("Invalid piece type");
		}
		opponent ^= 1ULL << current_square; // Remove the processed square
	}
	white = !white; // Force the correct turn back
	return attack_squares;
}

inline int Bitboard::findFirstSetBit(uint64_t value) {
#if defined(_MSC_VER) // MSVC
	unsigned long index;
	if (_BitScanForward64(&index, value)) {
		return static_cast<int>(index);
	}
	return -1; // No bits are set
#else // GCC and Clang
	return value ? __builtin_ctzll(value) : -1;
#endif
}

inline int Bitboard::findLastSetBit(uint64_t value) {
#if defined(_MSC_VER) // MSVC
	unsigned long index;
	if (_BitScanReverse64(&index, value)) {
		return static_cast<int>(index);
	}
	return -1;
#else // GCC and Clang
	return value ? (63 - __builtin_clzll(value)) : -1;
#endif
}
