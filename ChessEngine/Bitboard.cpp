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
	case 'k': legal_moves = getKingMoves(from); break;
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
	case 'r': white ? movePiece(white_rooks) : movePiece(black_rooks); break;
	case 'q': white ? movePiece(white_queen) : movePiece(black_queen); break;
	case 'k': white ? movePiece(white_king) : movePiece(black_king); break;
	default: throw std::invalid_argument("Invalid piece type");
	}

	// Early exit if piece was moved to an empty square
	if (empty) return;

	// Lambda for capturing operation
	auto capturePiece = [target_square](uint64_t& piece_bitboard) {
		piece_bitboard &= ~target_square; // Clear captured square
	};

	// Call capturePiece depending which turn ongoing
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
	half_moves = 0; // Captures reset halfmove-clock
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

	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	uint64_t occupied = white_pieces | black_pieces; // Combine white and black occupancy with OR

	// White bishop
	if (white_bishops & bishop_bitboard) {
		moves &= ~whitePieces(); // White bishop can't move onto white pieces

		// Prevent moving through or behind other white pieces
		if (BISHOP_MOVES[square].top_left & white_bishops) {
			moves &= ~BISHOP_MOVES[square].top_left; // Don't move past other white pieces on the top-left diagonal
		}
		if (BISHOP_MOVES[square].top_right & white_bishops) {
			moves &= ~BISHOP_MOVES[square].top_right; // Don't move past other white pieces on the top-right diagonal
		}
		if (BISHOP_MOVES[square].bottom_left & white_bishops) {
			moves &= ~BISHOP_MOVES[square].bottom_left; // Don't move past other white pieces on the bottom-left diagonal
		}
		if (BISHOP_MOVES[square].bottom_right & white_bishops) {
			moves &= ~BISHOP_MOVES[square].bottom_right; // Don't move past other white pieces on the bottom-right diagonal
		}
	}
	// Black bishop
	else if (black_bishops & bishop_bitboard) {
		moves &= ~blackPieces(); // Black bishop can't move onto black pieces

		// Prevent moving through or behind other black pieces
		if (BISHOP_MOVES[square].top_left & black_bishops) {
			moves &= ~BISHOP_MOVES[square].top_left; // Don't move past other black pieces on the top-left diagonal
		}
		if (BISHOP_MOVES[square].top_right & black_bishops) {
			moves &= ~BISHOP_MOVES[square].top_right; // Don't move past other black pieces on the top-right diagonal
		}
		if (BISHOP_MOVES[square].bottom_left & black_bishops) {
			moves &= ~BISHOP_MOVES[square].bottom_left; // Don't move past other black pieces on the bottom-left diagonal
		}
		if (BISHOP_MOVES[square].bottom_right & black_bishops) {
			moves &= ~BISHOP_MOVES[square].bottom_right; // Don't move past other black pieces on the bottom-right diagonal
		}
	}

	return moves;
}

uint64_t Bitboard::getRookMoves(int square) {
	uint64_t rook_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_rooks & rook_bitboard) == 0 && (black_rooks & rook_bitboard) == 0) {
		return 0ULL; // No rook exists at this square
	}

	// Combine moves
	uint64_t moves = (ROOK_MOVES[square].top |
		ROOK_MOVES[square].bottom |
		ROOK_MOVES[square].left |
		ROOK_MOVES[square].right);

	// White rook
	if (white_rooks & rook_bitboard) {
		moves &= ~whitePieces(); // White rook can't move onto white pieces

		// Prevent moving through or behind other white pieces
		if (ROOK_MOVES[square].top & white_rooks) {
			moves &= ~ROOK_MOVES[square].top; // Don't move past other white pieces in the top direction
		}
		if (ROOK_MOVES[square].bottom & white_rooks) {
			moves &= ~ROOK_MOVES[square].bottom; // Don't move past other white pieces in the bottom direction
		}
		if (ROOK_MOVES[square].left & white_rooks) {
			moves &= ~ROOK_MOVES[square].left; // Don't move past other white pieces on the left
		}
		if (ROOK_MOVES[square].right & white_rooks) {
			moves &= ~ROOK_MOVES[square].right; // Don't move past other white pieces on the right
		}
	}
	// Black rook
	else if (black_rooks & rook_bitboard) {
		moves &= ~blackPieces(); // Black rook can't move onto black pieces

		// Prevent moving through or behind other black pieces
		if (ROOK_MOVES[square].top & black_rooks) {
			moves &= ~ROOK_MOVES[square].top; // Don't move past other black pieces in the top direction
		}
		if (ROOK_MOVES[square].bottom & black_rooks) {
			moves &= ~ROOK_MOVES[square].bottom; // Don't move past other black pieces in the bottom direction
		}
		if (ROOK_MOVES[square].left & black_rooks) {
			moves &= ~ROOK_MOVES[square].left; // Don't move past other black pieces on the left
		}
		if (ROOK_MOVES[square].right & black_rooks) {
			moves &= ~ROOK_MOVES[square].right; // Don't move past other black pieces on the right
		}
	}





	return 0ULL; // Should never reach here
}

uint64_t Bitboard::getQueenMoves(int square) {
	uint64_t queen_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_queen & queen_bitboard) == 0 && (black_queen & queen_bitboard) == 0) {
		return 0ULL; // No queen exists at this square
	}

	// Combine moves
	uint64_t moves = (QUEEN_MOVES[square].top |
		QUEEN_MOVES[square].bottom |
		QUEEN_MOVES[square].left |
		QUEEN_MOVES[square].right |
		QUEEN_MOVES[square].top_left |
		QUEEN_MOVES[square].top_right |
		QUEEN_MOVES[square].bottom_left |
		QUEEN_MOVES[square].bottom_right);

	// White queen
	if (white_queen & queen_bitboard) {
		moves &= ~whitePieces(); // White queen can't move onto white pieces

		// Prevent moving through or behind other white pieces
		if (QUEEN_MOVES[square].top & white_queen) {
			moves &= ~QUEEN_MOVES[square].top; // Don't move past other white pieces in the top direction
		}
		if (QUEEN_MOVES[square].bottom & white_queen) {
			moves &= ~QUEEN_MOVES[square].bottom; // Don't move past other white pieces in the bottom direction
		}
		if (QUEEN_MOVES[square].left & white_queen) {
			moves &= ~QUEEN_MOVES[square].left; // Don't move past other white pieces on the left
		}
		if (QUEEN_MOVES[square].right & white_queen) {
			moves &= ~QUEEN_MOVES[square].right; // Don't move past other white pieces on the right
		}
		if (QUEEN_MOVES[square].top_left & white_queen) {
			moves &= ~QUEEN_MOVES[square].top_left; // Don't move past other white pieces on the top-left diagonal
		}
		if (QUEEN_MOVES[square].top_right & white_queen) {
			moves &= ~QUEEN_MOVES[square].top_right; // Don't move past other white pieces on the top-right diagonal
		}
		if (QUEEN_MOVES[square].bottom_left & white_queen) {
			moves &= ~QUEEN_MOVES[square].bottom_left; // Don't move past other white pieces on the bottom-left diagonal
		}
		if (QUEEN_MOVES[square].bottom_right & white_queen) {
			moves &= ~QUEEN_MOVES[square].bottom_right; // Don't move past other white pieces on the bottom-right diagonal
		}
	}
	// Black queen
	else if (black_queen & queen_bitboard) {
		moves &= ~blackPieces(); // Black queen can't move onto black pieces

		// Prevent moving through or behind other black pieces
		if (QUEEN_MOVES[square].top & black_queen) {
			moves &= ~QUEEN_MOVES[square].top; // Don't move past other black pieces in the top direction
		}
		if (QUEEN_MOVES[square].bottom & black_queen) {
			moves &= ~QUEEN_MOVES[square].bottom; // Don't move past other black pieces in the bottom direction
		}
		if (QUEEN_MOVES[square].left & black_queen) {
			moves &= ~QUEEN_MOVES[square].left; // Don't move past other black pieces on the left
		}
		if (QUEEN_MOVES[square].right & black_queen) {
			moves &= ~QUEEN_MOVES[square].right; // Don't move past other black pieces on the right
		}
		if (QUEEN_MOVES[square].top_left & black_queen) {
			moves &= ~QUEEN_MOVES[square].top_left; // Don't move past other black pieces on the top-left diagonal
		}
		if (QUEEN_MOVES[square].top_right & black_queen) {
			moves &= ~QUEEN_MOVES[square].top_right; // Don't move past other black pieces on the top-right diagonal
		}
		if (QUEEN_MOVES[square].bottom_left & black_queen) {
			moves &= ~QUEEN_MOVES[square].bottom_left; // Don't move past other black pieces on the bottom-left diagonal
		}
		if (QUEEN_MOVES[square].bottom_right & black_queen) {
			moves &= ~QUEEN_MOVES[square].bottom_right; // Don't move past other black pieces on the bottom-right diagonal
		}
	}

	return 0ULL; // Should never reach here
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

int Bitboard::findFirstSetBit(uint64_t value) {
	unsigned long index;
	if (_BitScanForward64(&index, value)) {
		return static_cast<int>(index);
	}
	return -1; // No bits are set
}

int Bitboard::findLastSetBit(uint64_t value) {
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
