#include "pch.h"
#include "Bitboard.h"
#include "MoveTables.h"


Bitboard::Bitboard():
	castling_rights(0x0F),                 // All castling rights (0b00001111)
	en_passant_target(UNASSIGNED),         // None
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

char Bitboard::getPieceTypeChar(int square_int) const {
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

std::string Bitboard::getGameState(bool white) {
	std::string game_state;
	if (isCheckmate(white)) {
		game_state = "M";
	}
	else if (isInCheck(white)) {
		game_state = "C";
	}
	else if (isStalemate(white)) {
		game_state = "S";
	}
	else {
		game_state = "-";
	}
	return game_state;
}


bool Bitboard::isInCheck(bool white) {
	uint64_t king_bitboard = white ? white_king : black_king;
	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();

	return (king_bitboard & getAttackSquares(white_pieces, black_pieces, white));
}


bool Bitboard::isCheckmate(bool white) {
	if (!isInCheck(white)) {
		return false; // Not in check, so not checkmate

	}
	uint64_t king_bitboard = white ? white_king : black_king;
	int king_square = findFirstSetBit(king_bitboard);

	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();

	// Get currently possible king moves
	uint64_t king_moves = getKingMoves(king_square, white_pieces, black_pieces, white);

	// Clear location from pieces to also get moves that leap over the king (king can't move to a square that would be attacked)
	(white ? white_pieces : black_pieces) &= ~king_bitboard;
	uint64_t enemy_attacks = getAttackSquares(white_pieces, black_pieces, white); // Squares attacked by enemy

	// Check if the king can escape (moves to a non-attacked square)
	if (king_moves & ~enemy_attacks) return false;

	// Reset pieces bitboards
	white_pieces = whitePieces();
	black_pieces = blackPieces();

	// Now we must check the direct attacks to king and if those can be blocked
	// If not possibility to block (with a piece other than king), results in checkmate
	// 
	// First we check the possibility of a double check (two or more attackers)
	// This results in immediate checkmate since cannot be blocked
	uint64_t attackers = getAttackers(king_bitboard, white_pieces, black_pieces, white); // Get attackers
	if (attackers == 0) return false; // Make sure there are attackers

	// Remove the least significant bit (LSB)
	attackers &= attackers - 1;

	// If any bits remain, it's a double check
	if (attackers != 0) return true;

	// Get square index of attacker
	uint64_t attacker = getAttackers(king_bitboard, white_pieces, black_pieces, white);
	int attacker_square = findLastSetBit(attacker);

	// Get the attacking ray and determine if can be blocked
	uint64_t attacking_ray = getAttackingRay(attacker_square, king_square);
	if (attacking_ray == 0) return false; // Validate ray

	// We reached the final part of checking for checkmate
	// If cannot be blocked -> checkmate
	return !canBlock(attacking_ray, white);
}

bool Bitboard::isStalemate(bool white) {
	// Loop over each friendly piece and check if there are legal moves
	// If none of the pieces have legal moves, results in stalemate
	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	uint64_t friendly = white ? white_pieces : black_pieces;
	uint64_t possible_moves = 0ULL;
	while (friendly != 0) {
		int current_square = findLastSetBit(friendly); // Isolate LSB and get as index
		PieceType piece_type = getPieceType(current_square);
		// Get moves depending on the piece type
		switch (piece_type)
		{
		case PieceType::PAWN: possible_moves = getPawnMoves(current_square, white_pieces, black_pieces, white); break;
		case PieceType::KNIGHT: possible_moves = getKnightMoves(current_square, white_pieces, black_pieces); break;
		case PieceType::BISHOP: possible_moves = getBishopMoves(current_square, white_pieces, black_pieces, white); break;
		case PieceType::ROOK: possible_moves = getRookMoves(current_square, white_pieces, black_pieces, white); break;
		case PieceType::QUEEN: possible_moves = getQueenMoves(current_square, white_pieces, black_pieces, white); break;
		case PieceType::KING: possible_moves = getKingMoves(current_square, white_pieces, black_pieces, white); break;
		default: throw std::invalid_argument("Invalid piece type");
		}
		friendly ^= 1ULL << current_square; // Remove the processed square

		// Check for ability to block
		if (possible_moves != 0) return false;
	}
	return true;
}

int Bitboard::getHalfMoveClock() const {
	return half_moves;
}

int Bitboard::getFullMoveNumber() const {
	return full_moves;
}

uint64_t Bitboard::getLegalMoves(int from, bool white) {
	PieceType piece = getPieceType(from); // Get piece type at square

	uint64_t legal_moves = 0ULL;
	uint64_t enemy_attacks = 0ULL;
	uint64_t king_captures = 0ULL;

	// Get both pieces as bitboards
	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();

	switch (piece)
	{
	case PieceType::PAWN: legal_moves = getPawnMoves(from, white_pieces, black_pieces, white); break;
	case PieceType::KNIGHT: legal_moves = getKnightMoves(from, white_pieces, black_pieces); break;
	case PieceType::BISHOP: legal_moves = getBishopMoves(from, white_pieces, black_pieces, white); break;
	case PieceType::ROOK: legal_moves = getRookMoves(from, white_pieces, black_pieces, white); break;
	case PieceType::QUEEN: legal_moves = getQueenMoves(from, white_pieces, black_pieces, white); break;
	case PieceType::KING: legal_moves = getKingMoves(from, white_pieces, black_pieces, white); break;
	default: throw std::invalid_argument("Invalid piece type");
	}

	// King cannot be captured
	// Remove from moves if in
	if (legal_moves & white_king) {
		legal_moves &= ~white_king;
	}
	else if (legal_moves & black_king) {
		legal_moves &= ~black_king;
	}

	// When we move a piece that is not the king, the move must not get the king in check
	if (piece != PieceType::KING) {
		uint64_t king_bitboard = white ? white_king : black_king;
		int king_square = findLastSetBit(king_bitboard);
		// If king is in check, move must get the king out of check
		// We have already made sure at this point that the king is not in checkmate,
		// So there must be moves that get the king out of checkmate
		// Exclude king from this check, since we have already calculated the squares where it could move
		uint64_t attacker;
		int attacker_square;
		uint64_t attacking_ray;
		if (isInCheck(white)) {
			// Legal moves are the ones that overlap with attacking ray
			attacker = getAttackers(king_bitboard, white_pieces, black_pieces, white);
			attacker_square = findLastSetBit(attacker);
			attacking_ray = getAttackingRay(attacker_square, king_square);
			legal_moves &= attacking_ray;
		}
		else {
			// Exclude the piece from its sides bitboard and check if the removal results in check
			(white ? white_pieces : black_pieces) &= ~(1ULL << from);
			uint64_t new_enemy_attacks = getAttackSquares(white_pieces, black_pieces, white);
			if (king_bitboard & new_enemy_attacks) { // King is attacked after move -> results in check
				// The legal moves cannot differ from the attacking ray
				attacker = getAttackers(king_bitboard, white_pieces, black_pieces, white);
				attacker_square = findLastSetBit(attacker);
				attacking_ray = getAttackingRay(attacker_square, king_square);
				legal_moves &= attacking_ray;
			}
		}
	}
	return legal_moves;
}

void Bitboard::applyMove(int source, int target, bool white) {
	uint64_t source_square = 1ULL << source; // Convert source to bitboard
	uint64_t target_square = 1ULL << target; // Convert target

	// Get piece types at squares
	PieceType source_piece = getPieceType(source);
	PieceType target_piece = getPieceType(target);

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
	switch (source_piece)
	{
	case PieceType::PAWN: white ? movePiece(white_pawns) : movePiece(black_pawns); half_moves = 0; break; // Resets halfmove-clock
	case PieceType::KNIGHT: white ? movePiece(white_knights) : movePiece(black_knights); break;
	case PieceType::BISHOP: white ? movePiece(white_bishops) : movePiece(black_bishops); break;
	case PieceType::QUEEN: white ? movePiece(white_queen) : movePiece(black_queen); break;
	case PieceType::ROOK: white ? movePiece(white_rooks) : movePiece(black_rooks);
		// Update Queen/Kingside castling depending on which rook moved
		if (castling_rights != 0) updateRookCastling(white, source);
		break;
	case PieceType::KING: white ? movePiece(white_king) : movePiece(black_king);
		// Update castling rights
		if (castling_rights != 0) {
			castling_rights &= ~(white ? 0x03 : 0x0C); // (0x03 = white, 0x0C black)
			// Check if move was castling
			// If so, move the correct rook also
			if (abs(source - target) == 2) handleCastling(white, target);
		}
		break;
	default: throw std::invalid_argument("Invalid piece type");
	}

	// Check if the move was a pawn double push
	// If so, set en passant target square
	if (source_piece == PieceType::PAWN && abs(source - target) == 16) {
		// Set en passant depending on which color moved
		en_passant_target = white ? (source + 8) : (target + 8);
	}
	else if (!(source_piece == PieceType::PAWN && target == en_passant_target)) { // Reset if was not a pawn moved to en passant
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
		switch (target_piece)
		{
		case PieceType::PAWN: white ? capturePiece(black_pawns) : capturePiece(white_pawns); break;
		case PieceType::KNIGHT: white ? capturePiece(black_knights) : capturePiece(white_knights); break;
		case PieceType::BISHOP: white ? capturePiece(black_bishops) : capturePiece(white_bishops); break;
		case PieceType::ROOK: white ? capturePiece(black_rooks) : capturePiece(white_rooks);
			// Update castling rights if rook was captured
			if (castling_rights != 0) updateRookCastling(!white, target);
			break;
		case PieceType::QUEEN: white ? capturePiece(black_queen) : capturePiece(white_queen); break;
		case PieceType::KING: white ? capturePiece(black_king) : capturePiece(white_king); break;
		default: throw std::invalid_argument("Invalid piece type");
		}
	}
	else if (source_piece == PieceType::PAWN && target == en_passant_target) { // Case where we capture a pawn by en passant
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

void Bitboard::applyPromotion(int target, char promotion, bool white) {
	uint64_t target_square = 1ULL << target; // Convert target to bitboard
	// Update the bitboard of the piece moved
	// Clears target square using bitwise AND with the negation target_square
	// Sets the target using bitwise OR with target_square
	// Create lambda for the moving operations
	auto movePiece = [target_square](uint64_t& piece_bitboard) {
		piece_bitboard |= target_square; // Set target square
		};
	// Call movePiece depending which turn ongoing
	switch (promotion)
	{
	case 'n': white ? movePiece(white_knights) : movePiece(black_knights); break;
	case 'b': white ? movePiece(white_bishops) : movePiece(black_bishops); break;
	case 'r': white ? movePiece(white_rooks) : movePiece(black_rooks); break;
	case 'q': white ? movePiece(white_queen) : movePiece(black_queen); break;
	default: throw std::invalid_argument("Invalid promotion type");
	}
	// Clear pawn from target square
	(white ? white_pawns : black_pawns) &= ~target_square;
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

uint64_t Bitboard::getPawnMoves(int square, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white) {
	uint64_t pawn_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_pawns & pawn_bitboard) == 0 && (black_pawns & pawn_bitboard) == 0) {
		return 0ULL; // No pawn exists at this square
	}

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

uint64_t Bitboard::getPawnCaptures(int square, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white) {
	uint64_t pawn_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_pawns & pawn_bitboard) == 0 && (black_pawns & pawn_bitboard) == 0) {
		return 0ULL; // No pawn exists at this square
	} 

	// Initialize captures
	uint64_t captures = 0ULL;

	if (white_pawns & pawn_bitboard) { // White pawn
		captures = WHITE_PAWN_MOVES[square].captures;
	}
	else if (black_pawns & pawn_bitboard) { // Black pawn
		captures = BLACK_PAWN_MOVES[square].captures;
	}

	// Check if en passant is available and in capture moves of the current moved piece
	if ((en_passant_target != UNASSIGNED) &&
		((white ? WHITE_PAWN_MOVES[square].captures : BLACK_PAWN_MOVES[square].captures) & (1ULL << en_passant_target))) {
		captures |= 1ULL << en_passant_target;
	}

	return captures;
}


uint64_t Bitboard::getKnightMoves(int square, const uint64_t& white_pieces, const uint64_t& black_pieces) {
	uint64_t knight_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_knights & knight_bitboard) == 0 && (black_knights & knight_bitboard) == 0) {
		return 0ULL; // No knight exists at this square
	}

	// Get the precomputed knight moves for the square
	uint64_t moves = KNIGHT_MOVES[square].moves;

	if (white_knights & knight_bitboard) {
		moves &= ~white_pieces; // White knight can't move onto white pieces
	}
	else if (black_knights & knight_bitboard) {
		moves &= ~black_pieces; // Black knight can't move onto black pieces
	}

	return moves; // Should never reach here
}

uint64_t Bitboard::getBishopMoves(int square, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white) {
	uint64_t bishop_bitboard = 1ULL << square; // Convert index to bitboard

	if ((white_bishops & bishop_bitboard) == 0 && (black_bishops & bishop_bitboard) == 0) {
		return 0ULL; // No bishop exists at this square
	}

	// Initialize moves
	uint64_t moves = 0ULL;

	// Combine legal moves
	moves |= getSlidingMoves(BISHOP_MOVES[square].top_left, true, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(BISHOP_MOVES[square].top_right, true, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(BISHOP_MOVES[square].bottom_left, false, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(BISHOP_MOVES[square].bottom_right, false, white_pieces, black_pieces, white);

	return moves;
}

uint64_t Bitboard::getRookMoves(int square, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white) {
	uint64_t rook_bitboard = 1ULL << square; // Convert index to bitboard

	if ((white_rooks & rook_bitboard) == 0 && (black_rooks & rook_bitboard) == 0) {
		return 0ULL; // No rook exists at this square
	}

	// Initialize moves
	uint64_t moves = 0ULL;

	// Combine legal moves
	moves |= getSlidingMoves(ROOK_MOVES[square].top, true, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(ROOK_MOVES[square].bottom, false, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(ROOK_MOVES[square].left, false, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(ROOK_MOVES[square].right, true, white_pieces, black_pieces, white);

	return moves;
}

uint64_t Bitboard::getQueenMoves(int square, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white) {
	uint64_t queen_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_queen & queen_bitboard) == 0 && (black_queen & queen_bitboard) == 0) {
		return 0ULL; // No queen exists at this square
	}

	// Initialize moves
	uint64_t moves = 0ULL;

	// Combine legal moves
	moves |= getSlidingMoves(QUEEN_MOVES[square].top, true, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(QUEEN_MOVES[square].bottom, false, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(QUEEN_MOVES[square].left, false, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(QUEEN_MOVES[square].right, true, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(QUEEN_MOVES[square].top_left, true, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(QUEEN_MOVES[square].top_right, true, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(QUEEN_MOVES[square].bottom_left, false, white_pieces, black_pieces, white);
	moves |= getSlidingMoves(QUEEN_MOVES[square].bottom_right, false, white_pieces, black_pieces, white);

	return moves;
}

uint64_t Bitboard::getKingMoves(int square, uint64_t white_pieces, uint64_t black_pieces, bool white) {
	uint64_t king_bitboard = 1ULL << square; // Convert index to bitboard
	if ((white_king & king_bitboard) == 0 && (black_king & king_bitboard) == 0) {
		return 0ULL; // No king exists at this square
	}

	// Initialize moves
	uint64_t moves = KING_MOVES[square].moves;

	// Block moving onto friendly pieces and add castling moves if king is on its starting square
	if (white_king & king_bitboard) {
		moves &= ~white_pieces; // White king can't move onto white pieces
		if (square == 4) moves |= getCastlingMoves(white);
	}
	else if (black_king & king_bitboard) {
		moves &= ~black_pieces; // Black king can't move onto black pieces
		if (square == 60) moves |= getCastlingMoves(white);
	}

	// Exlude enemy king's adjacent squares
	uint64_t enemy_king = white ? black_king : white_king;
	moves &= ~KING_MOVES[findFirstSetBit(enemy_king)].moves;

	// King can't move into check
	// Get potential squares where enemy could attack (results in check)
	// This also includes the squares where king moves to capture an opposite piece and ends up in check

	// Get the squares where enemy could move with the current board state and minus the king
	// Meaning moves that leap over the king
	(white ? white_pieces : black_pieces) &= ~(1ULL << square); // Clear king square
	uint64_t enemy_attacks = getAttackSquares(white_pieces, black_pieces, white);

	// Get the king captures that would result in check
	uint64_t king_captures = (white ? black_pieces : white_pieces) & moves;
	(white ? black_pieces : white_pieces) &= ~king_captures; // Capture possible pieces
	(white ? white_pieces : black_pieces) |= king_captures; // Move in own sides bitboard

	// Get all the potential enemy attacks after the captures and combine with currently possible attacks
	enemy_attacks |= getAttackSquares(white_pieces, black_pieces, white);
	moves &= ~enemy_attacks;

	return moves; 
}

uint64_t Bitboard::getSlidingMoves(uint64_t direction_moves, bool reverse, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white) {
	uint64_t same_color = white ? white_pieces : black_pieces; // Determine which color is being moved
	uint64_t occupied = white_pieces | black_pieces; // All occupied squares

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

uint64_t Bitboard::getCastlingMoves(bool white) {
	// Initialize castling moves
	uint64_t castling_moves = 0ULL;
	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	uint64_t occupied = white_pieces | black_pieces;
	uint64_t attack_squares = getAttackSquares(white_pieces, black_pieces, white);

	// If in check, cannot castle
	if (isInCheck(white)) return 0ULL;

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

void Bitboard::updateRookCastling(bool white, int source) {
	if (white && (castling_rights & 0x01 || castling_rights & 0x02)) {
		if (source % 8 == 0) { // Queenside
			castling_rights &= ~0x02;
		}
		else if (source % 8 == 7) { // Kingside
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

void Bitboard::handleCastling(bool white, int target) {
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

uint64_t Bitboard::getAttackSquares(const uint64_t& white_pieces, const uint64_t& black_pieces, bool white) {
	// Initialize squares
	uint64_t attack_squares = 0ULL;

	// Iterate over opponent pieces
	// Done by isolating FSB, processing the piece type at square, and removing the processed square (XOR)
	// At each occupied square we get the moves and combine in the attack squares (OR)
	uint64_t opponent = white ? black_pieces : white_pieces;
	// We flip the turn flag for move generation to capture the correct squares
	while (opponent != 0) { 
		int current_square = findFirstSetBit(opponent); // Isolate FSB and get as index
		PieceType piece_type = getPieceType(current_square); // Get piece type
		// Get moves depending on the piece type
		switch (piece_type)
		{
		case PieceType::PAWN: attack_squares |= getPawnCaptures(current_square, white_pieces, black_pieces, !white); break;
		case PieceType::KNIGHT: attack_squares |= getKnightMoves(current_square, white_pieces, black_pieces); break;
		case PieceType::BISHOP: attack_squares |= getBishopMoves(current_square, white_pieces, black_pieces, !white); break;
		case PieceType::ROOK: attack_squares |= getRookMoves(current_square, white_pieces, black_pieces, !white); break;
		case PieceType::QUEEN: attack_squares |= getQueenMoves(current_square, white_pieces, black_pieces, !white); break;
		case PieceType::KING: break;
		default: throw std::invalid_argument("Invalid piece type");
		}
		opponent ^= 1ULL << current_square; // Remove the processed square
	}
	return attack_squares;
}

uint64_t Bitboard::getAttackers(uint64_t king, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white) {
	// Initialize squares
	uint64_t attackers = 0ULL;

	// Determine attacking side
	uint64_t opponent = white ? black_pieces : white_pieces;
	// Iterate over opponent pieces with bitwise OR
	// We flip the turn flag for move generation to be able to capture correct pieces
	while(opponent != 0) {
		int current_square = findFirstSetBit(opponent); // Isolate FSB and get as index
		PieceType piece_type = getPieceType(current_square);
		// Get moves of the piece and check if any of the moves land on the king
		// If true, add current square to attackers as a bitboard
		switch (piece_type)
		{
		case PieceType::PAWN: if (getPawnCaptures(current_square, white_pieces, black_pieces, !white) & king) attackers |= 1ULL << current_square; break;
		case PieceType::KNIGHT: if (getKnightMoves(current_square, white_pieces, black_pieces) & king) attackers |= 1ULL << current_square; break;
		case PieceType::BISHOP: if (getBishopMoves(current_square, white_pieces, black_pieces, !white) & king) attackers |= 1ULL << current_square; break;
		case PieceType::ROOK: if (getRookMoves(current_square, white_pieces, black_pieces, !white) & king) attackers |= 1ULL << current_square; break;
		case PieceType::QUEEN: if (getQueenMoves(current_square, white_pieces, black_pieces, !white) & king) attackers |= 1ULL << current_square; break;
		case PieceType::KING: break;
		default: throw std::invalid_argument("Invalid piece type");
		}
		opponent ^= 1ULL << current_square; // Remove the processed square
	}
	return attackers;
}

uint64_t Bitboard::getAttackingRay(int attacker, int king) {
	// Find out the piece type attacking king
	PieceType piece_type = getPieceType(attacker);

	uint64_t attack_ray = 1ULL << attacker; // The attacker itself is included in the ray, for the case if able to capture it
	// If the attacker is a pawn or knight, the attack can't be blocked, so the attacker must be captured
	// So in that case we only return the attacker square
	switch (piece_type)
	{
	case PieceType::PAWN: break; // Must be captured
	case PieceType::KNIGHT: break; // Must be captured
	case PieceType::BISHOP: attack_ray |= formAttackingRay(attacker, king); break;
	case PieceType::ROOK: attack_ray |= formAttackingRay(attacker, king); break;
	case PieceType::QUEEN: attack_ray |= formAttackingRay(attacker, king); break;
	case PieceType::KING: break; // King can't capture another king
	default: throw std::invalid_argument("Invalid piece type");
	}
	return attack_ray;
}

uint64_t Bitboard::formAttackingRay(int attacker, int king) {
	// Calculate the difference between the attacker and king squares
	// This determines the direction the king is attacked from
	// If positive, attacked from left, down, bottom-left or bottom-right
	// If negative, attacked from right, up, top-left or top-right
	int diff = king - attacker;

	// If the difference is 0, the attacker and king are on the same square (invalid)
	if (diff == 0) return 0ULL;

	// Normalize the difference to get the direction
	int direction = get_direction(diff);
	if (direction == 0) return 0ULL; // Validate direction

	// We build the ray starting from attacker and ending in the square next to king
	uint64_t attacking_ray = 0ULL;
	int square = attacker;
	while (true) {
		square += direction; // Move in the attack direction

		// Bounds check: Ensure the new square doesn't wrap unexpectedly
		if (square < 0 || square > 63) break;
		//if ((direction == -1 || direction == 1) && (square % 8 == 0 || (square + 1) % 8 == 0)) break; // Prevent rank wrap
		if ((direction == -1 || direction == 1) && (square / 8 != (square - direction) / 8)) break; // If the above doesn't work :DD
		if (square == king) break; // Stop at king square

		attacking_ray |= (1ULL << square); // Add square to ray
	}

	return attacking_ray;
}

bool Bitboard::canBlock(const uint64_t& attack_ray, bool white) {
	// Get own pieces depending on the turn
	uint64_t friendly = white ? whitePieces() : blackPieces();
	friendly &= ~(white ? white_king : black_king); // Exclude own king

	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();;

	// Loop over own pieces and get their possible attacks at the current square
	// If the move is able to block the attack ray returns true
	uint64_t possible_moves = 0ULL;
	while (friendly != 0) {
		int current_square = findLastSetBit(friendly); // Isolate LSB and get as index
		PieceType piece_type = getPieceType(current_square);
		// Get moves depending on the piece type
		switch (piece_type)
		{
		case PieceType::PAWN: possible_moves = getPawnMoves(current_square, white_pieces, black_pieces, white); break;
		case PieceType::KNIGHT: possible_moves = getKnightMoves(current_square, white_pieces, black_pieces); break;
		case PieceType::BISHOP: possible_moves = getBishopMoves(current_square, white_pieces, black_pieces, white); break;
		case PieceType::ROOK: possible_moves = getRookMoves(current_square, white_pieces, black_pieces, white); break;
		case PieceType::QUEEN: possible_moves = getQueenMoves(current_square, white_pieces, black_pieces, white); break;
		default: throw std::invalid_argument("Invalid piece type");
		}
		friendly ^= 1ULL << current_square; // Remove the processed square

		// Check for ability to block
		if (possible_moves & attack_ray) return true;
	}
	return false; // No blocks were found
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

inline int Bitboard::get_direction(int diff) {
	if (diff % 8 == 0) return (diff > 0) ? 8 : -8;  // Vertical
	if (diff % 7 == 0) return (diff > 0) ? 7 : -7;  // Diagonal
	if (diff % 9 == 0) return (diff > 0) ? 9 : -9;  // Diagonal
	if (diff % 1 == 0) return (diff > 0) ? 1 : -1;  // Horizontal

	return 0;  // Invalid (should not happen if called correctly)
}

inline int Bitboard::count_set_bits(const uint64_t& bitboard) {
#if defined(_MSC_VER) // MSVC
	return __popcnt64(bitboard);
#else // GCC and Clang
	return __builtin_popcountll(bitboard);
#endif
}

/*
* The functions below are used for move generation and move encoding in chessAI
* The functions are used by the AI to generate all legal moves for a given position
* 
* Move generation is done by looping over all pieces and getting their legal moves
* The moves are then encoded using the chessAI encoding functions,
* and encoded moves are then added to the move list array
* 
* We only need to worry about the move encoding here, chessAI handles the rest
* ChessAI uses the move list to determine the best move to play
* 
*/

void Bitboard::generateMoves(std::array<uint32_t, MAX_MOVES>& move_list, int& move_count, bool white) {
	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	// Loop over all pieces and get all their legal moves
	uint64_t friendly_pieces = white ? white_pieces : black_pieces;
	while (friendly_pieces != 0) {
		int from = findFirstSetBit(friendly_pieces); // Isolate FSB and get as index
		PieceType piece = getPieceType(from);
		uint64_t legal_moves = getLegalMoves(from, white);
		while (legal_moves != 0) {
			int to = findFirstSetBit(legal_moves); // Isolate FSB and get as index
			// Get piece type at target square
			PieceType target_piece = getPieceType(to);
			// Get move type
			MoveType move_type = getMoveType(from, to, piece, target_piece, white);
			// If move was promotion, add all possible promotions to move list
			if (move_type == MoveType::PROMOTION || move_type == MoveType::PROMOTION_CAPTURE) {
				// Add all possible promotions to move list
				move_list[move_count++] = ChessAI::encodeMove(from, to, piece, target_piece, move_type, PieceType::QUEEN, false);
				move_list[move_count++] = ChessAI::encodeMove(from, to, piece, target_piece, move_type, PieceType::ROOK, false);
				move_list[move_count++] = ChessAI::encodeMove(from, to, piece, target_piece, move_type, PieceType::BISHOP, false);
				move_list[move_count++] = ChessAI::encodeMove(from, to, piece, target_piece, move_type, PieceType::KNIGHT, false);
			}
			else {
				// Encode the move and add to move list
				bool en_passant = (move_type == MoveType::EN_PASSANT);
				move_list[move_count++] = ChessAI::encodeMove(from, to, piece, target_piece, move_type, PieceType::EMPTY, en_passant);
			}
			legal_moves ^= 1ULL << to; // Remove the processed square
		}
		friendly_pieces ^= 1ULL << from; // Remove the processed square
	}
}

void Bitboard::applyMoveAI(uint32_t move, bool white, uint8_t& prev_castling_rights, int& prev_en_passant) {
	// Decode source and target squares
	int source = ChessAI::from(move);
	int target = ChessAI::to(move);

	// Save en passant and castling rights
	prev_castling_rights = castling_rights;
	prev_en_passant = en_passant_target;

	// Call applyMove depending which turn ongoing
	// applyMove handles all the move logic
	applyMove(source, target, white);

	// Handle promotion
	if (ChessAI::moveType(move) == MoveType::PROMOTION || ChessAI::moveType(move) == MoveType::PROMOTION_CAPTURE) {
		// Get promotion piece and promote pawn
		PieceType promotion = ChessAI::promotion(move);
		switch (promotion) {
		case PieceType::QUEEN: applyPromotion(target, 'q', white); break;
		case PieceType::ROOK: applyPromotion(target, 'r', white); break;
		case PieceType::BISHOP: applyPromotion(target, 'b', white); break;
		case PieceType::KNIGHT: applyPromotion(target, 'n', white); break;
		default: throw std::invalid_argument("Invalid promotion piece");
		}
	}
}

void Bitboard::applyMoveAI(uint32_t move, bool white) {
	// Decode source and target squares
	int source = ChessAI::from(move);
	int target = ChessAI::to(move);

	// Call applyMove depending which turn ongoing
	// applyMove handles all the move logic
	applyMove(source, target, white);

	// Handle promotion
	if (ChessAI::moveType(move) == MoveType::PROMOTION || ChessAI::moveType(move) == MoveType::PROMOTION_CAPTURE) {
		// Get promotion piece and promote pawn
		PieceType promotion = ChessAI::promotion(move);
		switch (promotion) {
		case PieceType::QUEEN: applyPromotion(target, 'q', white); break;
		case PieceType::ROOK: applyPromotion(target, 'r', white); break;
		case PieceType::BISHOP: applyPromotion(target, 'b', white); break;
		case PieceType::KNIGHT: applyPromotion(target, 'n', white); break;
		default: throw std::invalid_argument("Invalid promotion piece");
		}
	}
}

void Bitboard::undoMoveAI(uint32_t move, bool white, uint8_t prev_castling_rights, int prev_en_passant) {
	// Decode the move
	int source = ChessAI::from(move);
	int target = ChessAI::to(move);
	PieceType source_piece = ChessAI::piece(move);
	PieceType target_piece = ChessAI::capturedPiece(move);
	MoveType move_type = ChessAI::moveType(move);
	PieceType promotion = ChessAI::promotion(move);

	// Restore castling rights and en passant target
	castling_rights = prev_castling_rights;
	en_passant_target = prev_en_passant;

	// Move source piece back to source square
	// Doesn't differ for any move type
	uint64_t& source_bitboard = getPieceBitboard(source_piece, white);
	source_bitboard |= 1ULL << source; // Move to original position

	// Handle special cases

	// Restore captured piece if move was a capture
	if (move_type == MoveType::CAPTURE || move_type == MoveType::PROMOTION_CAPTURE) {
		uint64_t& target_bitboard = getPieceBitboard(target_piece, !white);
		target_bitboard |= 1ULL << target; // Restore captured piece
	}

	// Restore en passant pawn if move was en passant
	if (move_type == MoveType::EN_PASSANT) {
		// Determine en passant square
		int en_passant_square = white ? (target - 8) : (target + 8);
		uint64_t& en_passant_pawn = getPieceBitboard(PieceType::PAWN, !white);
		en_passant_pawn |= 1ULL << en_passant_square; // Restore captured pawn
	}

	// Restore rook to original position if move was castling
	if (move_type == MoveType::CASTLING) {
		// Determine if kingside or queenside castling
		bool kingside = target == 6 || target == 62;
		undoCastling(white, kingside);
	}

	// Restore promotion piece if move was promotion
	if (move_type == MoveType::PROMOTION || move_type == MoveType::PROMOTION_CAPTURE) {
		uint64_t& promoted_piece = getPieceBitboard(promotion, white);
		promoted_piece &= ~(1ULL << target); // Clear promotion square
	}

	// Remove the piece from the target square (applies to all non-promotion moves)
	if (move_type != MoveType::PROMOTION && move_type != MoveType::PROMOTION_CAPTURE) {
		source_bitboard &= ~(1ULL << target);
	}
}

int Bitboard::evaluateBoard(bool white) {
	// Calculate material and positional score
	int material_score = calculateMaterialScore(white);
	int positional_score = calculatePositionalScore(white);

	// Return the total score
	return material_score + positional_score;
}

bool Bitboard::isGameOver(bool white) {
	return isCheckmate(white) || isStalemate(white);
}

PieceType Bitboard::getPieceType(int square) const {
	uint64_t bitboard = 1ULL << square;
	// Determine piece type at square
	if (bitboard & (white_pawns | black_pawns)) return PieceType::PAWN;
	if (bitboard & (white_knights | black_knights)) return PieceType::KNIGHT;
	if (bitboard & (white_bishops | black_bishops)) return PieceType::BISHOP;
	if (bitboard & (white_rooks | black_rooks)) return PieceType::ROOK;
	if (bitboard & (white_queen | black_queen)) return PieceType::QUEEN;
	if (bitboard & (white_king | black_king)) return PieceType::KING;
	return PieceType::EMPTY;
}

uint64_t& Bitboard::getPieceBitboard(PieceType piece, bool white) {
	// Return the correct piece bitboard depending on the piece and color
	switch (piece)
	{
	case PieceType::PAWN: return white ? white_pawns : black_pawns;
	case PieceType::KNIGHT: return white ? white_knights : black_knights;
	case PieceType::BISHOP: return white ? white_bishops : black_bishops;
	case PieceType::ROOK: return white ? white_rooks : black_rooks;
	case PieceType::QUEEN: return white ? white_queen : black_queen;
	case PieceType::KING: return white ? white_king : black_king;
	default: throw std::invalid_argument("Invalid piece type");
	}
}

MoveType Bitboard::getMoveType(int source_square, int target_square, PieceType piece, PieceType target_piece, bool white) const {
	// Determine move type
	if (piece == PieceType::PAWN) {
		if (target_square == en_passant_target) return MoveType::EN_PASSANT;
		if ((white && target_square >= 56) || (!white && target_square <= 7)) {
			return (target_piece == PieceType::EMPTY) ? MoveType::PROMOTION : MoveType::PROMOTION_CAPTURE;
		}
	}
	if (piece == PieceType::KING && abs(source_square - target_square) == 2) return MoveType::CASTLING;
	if (target_piece != PieceType::EMPTY) return MoveType::CAPTURE;
	return MoveType::NORMAL;
}

void Bitboard::undoCastling(bool white, bool kingside) {
	uint64_t rook;
	if (white) {
		if (kingside) {
			rook = (1ULL << 5); // White rook on f1
			white_rooks &= ~rook; // Remove rook from f1
			white_rooks |= (1ULL << 7); // Move rook to h1
		}
		else {
			rook = (1ULL << 3); // White rook on d1
			white_rooks &= ~rook; // Remove rook from d1
			white_rooks |= (1ULL << 0); // Move rook to a1
		}
	}
	else {
		if (kingside) {
			rook = (1ULL << 61); // Black rook on f8
			black_rooks &= ~rook; // Remove rook from f8
			black_rooks |= (1ULL << 63); // Move rook to h8
		}
		else {
			rook = (1ULL << 59); // Black rook on d8
			black_rooks &= ~rook; // Remove rook from d8
			black_rooks |= (1ULL << 56); // Move rook to a8
		}
	}
}

int Bitboard::calculateMaterialScore(bool white) {
	int white_score = 0;
	int black_score = 0;

	// Get material score for all pieces in the board
	// Done by counting the number of set bits in the bitboards (amount of pieces) and multiplying by the piece value
	white_score += count_set_bits(white_pawns) * PAWN_VALUE;
	white_score += count_set_bits(white_knights) * KNIGHT_VALUE;
	white_score += count_set_bits(white_bishops) * BISHOP_VALUE;
	white_score += count_set_bits(white_rooks) * ROOK_VALUE;
	white_score += count_set_bits(white_queen) * QUEEN_VALUE;
	white_score += count_set_bits(white_king) * KING_VALUE;

	black_score += count_set_bits(black_pawns) * PAWN_VALUE;
	black_score += count_set_bits(black_knights) * KNIGHT_VALUE;
	black_score += count_set_bits(black_bishops) * BISHOP_VALUE;
	black_score += count_set_bits(black_rooks) * ROOK_VALUE;
	black_score += count_set_bits(black_queen) * QUEEN_VALUE;
	black_score += count_set_bits(black_king) * KING_VALUE;

	// Calculate the difference: white's score minus black's score
	int score = white_score - black_score;

	// Return the score depending on the turn
	// For white, the score is positive, for black the score is negative
	return white ? score : -score;
}

int Bitboard::calculatePositionalScore(bool white) {
	int white_score = 0;
	int black_score = 0;

	int game_phase = calculateGamePhase(); // Get the game phase score

	// Calculate positional scoring for both sides
	// Done by looping over all pieces and getting the positional score at the current square
	uint64_t white_pieces = whitePieces();
	while (white_pieces != 0) {
		int square = findLastSetBit(white_pieces); // Extract LSB
		PieceType piece = getPieceType(square); // Get piece type at square
		int row = 7 - (square / 8); // Convert square to row (0-7) for white
		int col = square % 8; // Convert square to column (0-7)

		// Add positional score for white
		if (piece == PieceType::PAWN) white_score += (game_phase * PawnTableMid[row][col] + (1 - game_phase) * PawnTableEnd[row][col]);
		else if (piece == PieceType::KNIGHT) white_score += (game_phase * KnightTableMid[row][col] + (1 - game_phase) * KnightTableEnd[row][col]);
		else if (piece == PieceType::BISHOP) white_score += (game_phase * BishopTableMid[row][col] + (1 - game_phase) * BishopTableEnd[row][col]);
		else if (piece == PieceType::ROOK) white_score += (game_phase * RookTableMid[row][col] + (1 - game_phase) * RookTableEnd[row][col]);
		else if (piece == PieceType::QUEEN) white_score += (game_phase * QueenTableMid[row][col] + (1 - game_phase) * QueenTableEnd[row][col]);
		else if (piece == PieceType::KING) white_score += (game_phase * KingTableMid[row][col] + (1 - game_phase) * KingTableEnd[row][col]);

		white_pieces ^= 1ULL << square; // Remove the processed square
	}

	uint64_t black_pieces = blackPieces();
	while (black_pieces != 0) {
		int square = findLastSetBit(black_pieces); // Extract LSB
		PieceType piece = getPieceType(square); // Get piece type at square
		int row = square / 8; // Convert square to row (0-7) for black
		int col = 7 - (square % 8); // Convert square to column (0-7)

		// Add positional score for black
		if (piece == PieceType::PAWN) black_score += (game_phase * PawnTableMid[row][col] + (1 - game_phase) * PawnTableEnd[row][col]);
		else if (piece == PieceType::KNIGHT) black_score += (game_phase * KnightTableMid[row][col] + (1 - game_phase) * KnightTableEnd[row][col]);
		else if (piece == PieceType::BISHOP) black_score += (game_phase * BishopTableMid[row][col] + (1 - game_phase) * BishopTableEnd[row][col]);
		else if (piece == PieceType::ROOK) black_score += (game_phase * RookTableMid[row][col] + (1 - game_phase) * RookTableEnd[row][col]);
		else if (piece == PieceType::QUEEN) black_score += (game_phase * QueenTableMid[row][col] + (1 - game_phase) * QueenTableEnd[row][col]);
		else if (piece == PieceType::KING) black_score += (game_phase * KingTableMid[row][col] + (1 - game_phase) * KingTableEnd[row][col]);

		black_pieces ^= 1ULL << square; // Remove the processed square
	}

	// Calculate the difference: white's score minus black
	int score = white_score - black_score;

	// Return the score depending on the turn
	return white ? score : -score;
}

int Bitboard::calculateGamePhase() {
	int game_phase = 0;

	// We use weighted sum method for game phase calculation
	// Weights are assigned to remaining pieces and summed up
	// Weights:
	// Queen = 4
	// Rook = 2
	// Knight & Bishop = 1

	game_phase += 4 * (count_set_bits(white_queen) + count_set_bits(black_queen));
	game_phase += 2 * (count_set_bits(white_rooks) + count_set_bits(black_rooks));
	game_phase += count_set_bits(white_knights) + count_set_bits(black_knights);

	// Normalize the game_phase to the range [0, 1]
	float normalized_game_phase = static_cast<float>(game_phase) / MAX_GAME_PHASE;

	// Ensure the value is clamped between 0 and 1 (in case of edge cases)
	normalized_game_phase = max(0.0f, min(1.0f, normalized_game_phase));

	return normalized_game_phase;
}
