#include "pch.h"
#include "Bitboard.h"
#include "MoveTables.h"
#include "Moves.h"
#include "Utils.h"


Bitboard::Bitboard():
	undo_stack_top(0),
	castling_rights(0x0F),                 // All castling rights (0b00001111)
	en_passant_target(UNASSIGNED),         // None
	half_moves(0),                         // Initially 0
	full_moves(0)                          // Initially 0
{
	// Initialize undo_stack
	for (int i = 0; i < MAX_SEARCH_DEPTH; ++i) {
		undo_stack[i] = { 0, UNASSIGNED, 0 };
	}

	// Initialize pin-data (initially none)
	for (int i = 0; i < 64; ++i) {
		pin_data.pin_rays[i] = 0xFFFFFFFFFFFFFFFFULL;
	}

	// Standard little-endian rank-file mapping (LSB = a1, MSB = h8)
	piece_bitboards[WHITE][PAWN] = 0x000000000000FF00;  // a2-h2
	piece_bitboards[BLACK][PAWN] = 0x00FF000000000000;  // a7-h7
	piece_bitboards[WHITE][ROOK] = 0x0000000000000081;  // a1, h1
	piece_bitboards[BLACK][ROOK] = 0x8100000000000000;  // a8, h8
	piece_bitboards[WHITE][KNIGHT] = 0x0000000000000042;  // b1, g1
	piece_bitboards[BLACK][KNIGHT] = 0x4200000000000000;  // b8, g8
	piece_bitboards[WHITE][BISHOP] = 0x0000000000000024;  // c1, f1
	piece_bitboards[BLACK][BISHOP] = 0x2400000000000000;  // c8, f8
	piece_bitboards[WHITE][QUEEN] = 0x0000000000000008;  // d1
	piece_bitboards[BLACK][QUEEN] = 0x0800000000000000;  // d8
	piece_bitboards[WHITE][KING] = 0x0000000000000010;  // e1
	piece_bitboards[BLACK][KING] = 0x1000000000000000;  // e8

	state.flags = 0; // Empty game state at beginning (no check, no checkmate, no stalemate)
}

char Bitboard::getPieceTypeChar(int square_int) const {
	uint64_t square = 1ULL << square_int; // Cast to bitboard

	if (piece_bitboards[WHITE][PAWN] & square) return 'P';
	if (piece_bitboards[BLACK][PAWN] & square) return 'p';
	if (piece_bitboards[WHITE][KNIGHT] & square) return 'N';
	if (piece_bitboards[BLACK][KNIGHT] & square) return 'n';
	if (piece_bitboards[WHITE][BISHOP] & square) return 'B';
	if (piece_bitboards[BLACK][BISHOP] & square) return 'b';
	if (piece_bitboards[WHITE][ROOK] & square) return 'R';
	if (piece_bitboards[BLACK][ROOK] & square) return 'r';
	if (piece_bitboards[WHITE][QUEEN] & square) return 'Q';
	if (piece_bitboards[BLACK][QUEEN] & square) return 'q';
	if (piece_bitboards[WHITE][KING] & square) return 'K';
	if (piece_bitboards[BLACK][KING] & square) return 'k';
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
	if (white ? state.isCheckmateWhite() : state.isCheckmateBlack()) {
		game_state = "M";
	}
	else if (white ? state.isCheckWhite() : state.isCheckBlack()) {
		game_state = "C";
	}
	else if (state.isStalemate()) {
		game_state = "S";
	}
	else {
		game_state = "-";
	}
	return game_state;
}

bool Bitboard::isInCheck(bool white) {
	uint64_t king_bitboard = white ? piece_bitboards[WHITE][KING] : piece_bitboards[BLACK][KING];
	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();

	return (king_bitboard & getAttackSquares(white_pieces, black_pieces, white));
}


bool Bitboard::isCheckmate(bool white) {
	if (!(white ? state.isCheckWhite() : state.isCheckBlack())) {
		return false; // Not in check, so not checkmate

	}
	uint64_t king_bitboard = white ? piece_bitboards[WHITE][KING] : piece_bitboards[BLACK][KING];
	int king_square = Utils::findFirstSetBit(king_bitboard);

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
	int attacker_square = Utils::findLastSetBit(attacker);

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
		int current_square = Utils::findLastSetBit(friendly); // Isolate LSB and get as index
		PieceType piece_type = getPieceType(current_square);
		// Get moves depending on the piece type
		switch (piece_type)
		{
		case PAWN: possible_moves = Moves::getPawnMoves(current_square, white_pieces, black_pieces, white, en_passant_target); break;
		case KNIGHT: possible_moves = Moves::getKnightMoves(current_square, white_pieces, black_pieces, en_passant_target); break;
		case BISHOP: possible_moves = Moves::getBishopMoves(current_square, white_pieces, black_pieces, white); break;
		case ROOK: possible_moves = Moves::getRookMoves(current_square, white_pieces, black_pieces, white); break;
		case QUEEN: possible_moves = Moves::getQueenMoves(current_square, white_pieces, black_pieces, white); break;
		case KING: possible_moves = getKingMoves(current_square, white_pieces, black_pieces, white); break;
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

	// Get both pieces as bitboards
	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();

	uint64_t legal_moves = Moves::getPseudoLegalMoves(from, piece, white_pieces, black_pieces, white, en_passant_target);

	// King cannot be captured
	// Remove from moves if in
	if (legal_moves & piece_bitboards[WHITE][KING]) {
		legal_moves &= ~piece_bitboards[WHITE][KING];
	}
	else if (legal_moves & piece_bitboards[BLACK][KING]) {
		legal_moves &= ~piece_bitboards[BLACK][KING];
	}

	// When we move a piece that is not the king, the move must not get the king in check
	if (piece != KING) {
		uint64_t king_bitboard = white ? piece_bitboards[WHITE][KING] : piece_bitboards[BLACK][KING];
		int king_square = Utils::findLastSetBit(king_bitboard);
		// If king is in check, move must get the king out of check
		// We have already made sure at this point that the king is not in checkmate,
		// So there must be moves that get the king out of checkmate
		// Exclude king from this check, since we have already calculated the squares where it could move

		// Exclude the piece from its sides bitboard to get enemy attacks after moving
		(white ? white_pieces : black_pieces) &= ~(1ULL << from);
		uint64_t attackers = getAttackers(king_bitboard, white_pieces, black_pieces, white);

		// If more than one attacker after moving, results in checkmate -> no legal moves
		if (Utils::countSetBits(attackers) > 1) return 0ULL;
		if (attackers == 0) return legal_moves; // No attackers after removal -> no modifications

		// Get the ray of the attacker
		int attacker_square = Utils::findLastSetBit(attackers);
		uint64_t attacking_ray = getAttackingRay(attacker_square, king_square);

		// Legal moves are the ones that overlap with attacking ray
		// Includes capturing the attacker
		legal_moves &= attacking_ray;
	}
	return legal_moves;
}

void Bitboard::applyMove(int source, int target, bool white) {
	uint64_t source_square = 1ULL << source; // Convert source to bitboard
	uint64_t target_square = 1ULL << target; // Convert target

	// Get piece types at squares
	PieceType source_piece = getPieceType(source);
	PieceType target_piece = getPieceType(target);

	// Determine if target square was originally empty
	bool empty = (target_piece == EMPTY);

	// Select bitboard to update
	uint64_t* moving_bitboard = nullptr;
	uint64_t* opponent_bitboard = nullptr;

	// Precompute whether castling is affected
	bool castling_affected = (castling_rights & (white ? 0x03 : 0x0C)) != 0;

	// Update half- and full-move counters
	++half_moves;
	++full_moves;

	// Move the piece from source to target using a switch on source_piece
	switch (source_piece) {
	case PAWN:
		moving_bitboard = white ? &piece_bitboards[WHITE][PAWN] : &piece_bitboards[BLACK][PAWN];
		half_moves = 0;  // Resets half moves
		if (target == en_passant_target) {
			target_piece = PAWN;  // Mark as pawn capture
			empty = false;
		}
		break;
	case KNIGHT: moving_bitboard = white ? &piece_bitboards[WHITE][KNIGHT] : &piece_bitboards[BLACK][KNIGHT]; break;
	case BISHOP: moving_bitboard = white ? &piece_bitboards[WHITE][BISHOP] : &piece_bitboards[BLACK][BISHOP]; break;
	case QUEEN:  moving_bitboard = white ? &piece_bitboards[WHITE][QUEEN] : &piece_bitboards[BLACK][QUEEN]; break;
	case ROOK:
		moving_bitboard = white ? &piece_bitboards[WHITE][ROOK] : &piece_bitboards[BLACK][ROOK];
		if (castling_affected) updateRookCastling(white, source);
		break;
	case KING:
		moving_bitboard = white ? &piece_bitboards[WHITE][KING] : &piece_bitboards[BLACK][KING];
		if (castling_affected) {
			castling_rights &= ~(white ? 0x03 : 0x0C);  // Disable castling rights
			if (abs(source - target) == 2) handleCastling(white, target);
		}
		break;
	default:
		throw std::invalid_argument("Invalid piece type");
	}

	// Move the piece
	*moving_bitboard = (*moving_bitboard & ~source_square) | target_square;

	// Update target square if was en passant capture so that we capture the correct pawn
	if (target == en_passant_target) {
		target_square = 1ULL << (white ? (target - 8) : (target + 8));
	}

	// Handle normal and en passant captures
	if (!empty) {
		switch (target_piece) {
		case PieceType::PAWN:   opponent_bitboard = white ? &piece_bitboards[BLACK][PAWN] : &piece_bitboards[WHITE][PAWN]; break;
		case PieceType::KNIGHT: opponent_bitboard = white ? &piece_bitboards[BLACK][KNIGHT] : &piece_bitboards[WHITE][KNIGHT]; break;
		case PieceType::BISHOP: opponent_bitboard = white ? &piece_bitboards[BLACK][BISHOP] : &piece_bitboards[WHITE][BISHOP]; break;
		case PieceType::ROOK:
			opponent_bitboard = white ? &piece_bitboards[BLACK][ROOK] : &piece_bitboards[WHITE][ROOK];
			if ((castling_rights & (white ? 0x0C : 0x03)) != 0) updateRookCastling(!white, target);
			break;
		case PieceType::QUEEN:  opponent_bitboard = white ? &piece_bitboards[BLACK][QUEEN] : &piece_bitboards[WHITE][QUEEN]; break;
		default: throw std::invalid_argument("Invalid piece type");
		}
		*opponent_bitboard &= ~target_square;  // Capture the piece
		half_moves = 0;  // Captures reset halfmove-clock
	}

	// Set en passant target if a pawn double moves
	if (source_piece == PieceType::PAWN && abs(source - target) == 16) {
		en_passant_target = white ? (source + 8) : (target + 8);
	}
	else {
		en_passant_target = UNASSIGNED;
	}

	updateBoardState(white); // Must always be called
}

void Bitboard::applyPromotion(int target, char promotion, bool white) {
	uint64_t target_square = 1ULL << target; // Convert target to bitboard
	// Update the bitboard of the piece moved
	// Clears target square using bitwise AND with the negation target_square
	// Sets the target using bitwise OR with target_square
	// Create lambda for the moving operations
	auto movePiece = [target_square](uint64_t& piece_bitboards) {
		piece_bitboards |= target_square; // Set target square
		};
	// Call movePiece depending which turn ongoing
	switch (promotion)
	{
	case 'n': white ? movePiece(piece_bitboards[WHITE][KNIGHT]) : movePiece(piece_bitboards[BLACK][KNIGHT]); break;
	case 'b': white ? movePiece(piece_bitboards[WHITE][BISHOP]) : movePiece(piece_bitboards[BLACK][BISHOP]); break;
	case 'r': white ? movePiece(piece_bitboards[WHITE][ROOK]) : movePiece(piece_bitboards[BLACK][ROOK]); break;
	case 'q': white ? movePiece(piece_bitboards[WHITE][QUEEN]) : movePiece(piece_bitboards[BLACK][QUEEN]); break;
	default: throw std::invalid_argument("Invalid promotion type");
	}
	// Clear pawn from target square
	(white ? piece_bitboards[WHITE][PAWN] : piece_bitboards[BLACK][PAWN]) &= ~target_square;
}

uint64_t Bitboard::whitePieces() {
	return piece_bitboards[WHITE][PAWN] | piece_bitboards[WHITE][ROOK] | piece_bitboards[WHITE][KNIGHT] |
		piece_bitboards[WHITE][BISHOP] | piece_bitboards[WHITE][QUEEN] | piece_bitboards[WHITE][KING];
}

uint64_t Bitboard::blackPieces() {
	return piece_bitboards[BLACK][PAWN] | piece_bitboards[BLACK][ROOK] | piece_bitboards[BLACK][KNIGHT] |
		piece_bitboards[BLACK][BISHOP] | piece_bitboards[BLACK][QUEEN] | piece_bitboards[BLACK][KING];
}

std::string Bitboard::squareToString(int square) const {
	char file = 'a' + (square % 8);
	char rank = '1' + (square / 8);
	return std::string() + file + rank;
}

uint64_t Bitboard::getKingMoves(int square, uint64_t white_pieces, uint64_t black_pieces, bool white) {
	uint64_t king_bitboard = 1ULL << square; // Convert index to bitboard
	if ((piece_bitboards[WHITE][KING] & king_bitboard) == 0 && (piece_bitboards[BLACK][KING] & king_bitboard) == 0) {
		return 0ULL; // No king exists at this square
	}

	// Initialize moves
	uint64_t moves = KING_MOVES[square].moves;

	// Block moving onto friendly pieces and add castling moves if king is on its starting square
	if (piece_bitboards[WHITE][KING] & king_bitboard) {
		moves &= ~white_pieces; // White king can't move onto white pieces
		if (square == 4) moves |= getCastlingMoves(white);
	}
	else if (piece_bitboards[BLACK][KING] & king_bitboard) {
		moves &= ~black_pieces; // Black king can't move onto black pieces
		if (square == 60) moves |= getCastlingMoves(white);
	}

	// Ensure enemy king's control squares are included in enemy attack calculation
	uint64_t enemy_king_bitboard = white ? piece_bitboards[BLACK][KING] : piece_bitboards[WHITE][KING];
	uint64_t enemy_king_attacks = KING_MOVES[Utils::findFirstSetBit(enemy_king_bitboard)].moves;

	// Compute enemy attacks, including the king's control squares
	uint64_t temp_white_pieces = white_pieces;
	uint64_t temp_black_pieces = black_pieces;

	(white ? temp_white_pieces : temp_black_pieces) &= ~(1ULL << square); // Temporarily remove king
	uint64_t enemy_attacks = getAttackSquares(temp_white_pieces, temp_black_pieces, white);
	enemy_attacks |= enemy_king_attacks; // Add enemy king's control squares

	// Remove squares that are under enemy attack
	moves &= ~enemy_attacks;

	// Go through each capture move and remove if it results in king capture
	uint64_t king_captures = (white ? black_pieces : white_pieces) & moves;
	while (king_captures != 0) {
		int target = Utils::findFirstSetBit(king_captures); // Isolate LSB
		uint64_t target_square = 1ULL << target; // Convert to bitboard

		// Temporarily remove the piece that is captured and check if king is still under attack
		(white ? temp_black_pieces : temp_white_pieces) &= ~target_square; // Remove captured piece
		uint64_t new_enemy_attacks = getAttackSquares(temp_white_pieces, temp_black_pieces, white); // Recalculate enemy attacks
		if (new_enemy_attacks & target_square) {
			moves &= ~target_square; // Add to enemy attacks if king would be captured there
		}
		king_captures ^= target_square; // Remove the processed square
	}

	return moves;
}



void Bitboard::filterKingMoves(uint64_t& legal_moves, int square, bool white) {
	// First check ability to castle
	bool castling_available = (castling_rights & (white ? 0x03 : 0x0C)) != 0 &&
		(white ? (square == 4) : (square == 60));
	// Add if possible
	if (castling_available) {
		legal_moves |= getCastlingMoves(white);
	}


}

uint64_t Bitboard::getCastlingMoves(bool white) {
	// Initialize castling moves
	uint64_t castling_moves = 0ULL;
	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	uint64_t occupied = white_pieces | black_pieces;
	uint64_t attack_squares = getAttackSquares(white_pieces, black_pieces, white);

	// If is in check, cannot castle
	if (white ? state.isCheckWhite() : state.isCheckBlack()) return 0ULL;

	uint64_t critical_squares;
	// Depending on player turn and castling availability add available castling moves
	if (white) {
		if (castling_rights & 0x01) { // White Kingside
			if ((occupied & WHITE_KINGSIDE_CASTLE_SQUARES) == 0) { // f1 and g1 must be free
				// King cannot castle out of, throught, or into check
				// Get squares that can't be under attack
				critical_squares = WHITE_KINGSIDE_CASTLE_SQUARES;

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
				critical_squares = WHITE_QUEENSIDE_CASTLE_SQUARES;
				if (!(critical_squares & attack_squares)) {
					castling_moves |= 1ULL << 2; // King moves to c1
				}
			}
		}
	}
	else {
		if (castling_rights & 0x04) { // Black Kingside
			if ((occupied & BLACK_KINGSIDE_CASTLE_SQUARES) == 0) { // f8 and g8 must be free
				critical_squares = BLACK_KINGSIDE_CASTLE_SQUARES; // e8, f8, g8
				if (!(critical_squares & attack_squares)) {
					castling_moves |= 1ULL << 62; // King moves to g8
				}
			}
		}
		if (castling_rights & 0x08) { // Black Queenside
			if ((occupied & BLACK_KINGSIDE_CASTLE_SQUARES) == 0) { // c8 and d8 must be free
				critical_squares = BLACK_KINGSIDE_CASTLE_SQUARES; // e8, d8, c8
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
			piece_bitboards[WHITE][ROOK] &= ~rook; // Remove rook from h1
			piece_bitboards[WHITE][ROOK] |= (rook >> 2); // Move rook to f1
		}
		else if (target == 2) { // Queenside castling (c1)
			rook = (1ULL << 0); // White rook on a1
			piece_bitboards[WHITE][ROOK] &= ~rook; // Remove rook from a1
			piece_bitboards[WHITE][ROOK] |= (rook << 3); // Move rook to d1
		}
	}
	else {
		// Black castling: Kingside (h8 -> f8), Queenside (a8 -> d8)
		if (target == 62) { // Kingside castling (g8)
			rook = (1ULL << 63); // Black rook on h8
			piece_bitboards[BLACK][ROOK] &= ~rook; // Remove rook from h8
			piece_bitboards[BLACK][ROOK] |= (rook >> 2); // Move rook to f8
		}
		else if (target == 58) { // Queenside castling (c8)
			rook = (1ULL << 56); // Black rook on a8
			piece_bitboards[BLACK][ROOK] &= ~rook; // Remove rook from a8
			piece_bitboards[BLACK][ROOK] |= (rook << 3); // Move rook to d8
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
		int current_square = Utils::findFirstSetBit(opponent); // Isolate FSB and get as index
		PieceType piece_type = getPieceType(current_square); // Get piece type
		// Get moves depending on the piece type
		switch (piece_type)
		{
		case PAWN: attack_squares |= Moves::getPawnCaptures(current_square, white_pieces, black_pieces, !white, en_passant_target); break;
		case KNIGHT: attack_squares |= Moves::getKnightMoves(current_square, white_pieces, black_pieces, white); break;
		case BISHOP: attack_squares |= Moves::getBishopMoves(current_square, white_pieces, black_pieces, !white); break;
		case ROOK: attack_squares |= Moves::getRookMoves(current_square, white_pieces, black_pieces, !white); break;
		case QUEEN: attack_squares |= Moves::getQueenMoves(current_square, white_pieces, black_pieces, !white); break;
		case KING: break;
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
		int current_square = Utils::findFirstSetBit(opponent); // Isolate FSB and get as index
		PieceType piece_type = getPieceType(current_square);
		// Get moves of the piece and check if any of the moves land on the king
		// If true, add current square to attackers as a bitboard
		switch (piece_type)
		{
		case PAWN: if (Moves::getPawnCaptures(current_square, white_pieces, black_pieces, !white, en_passant_target) & king) attackers |= 1ULL << current_square; break;
		case KNIGHT: if (Moves::getKnightMoves(current_square, white_pieces, black_pieces, !white) & king) attackers |= 1ULL << current_square; break;
		case BISHOP: if (Moves::getBishopMoves(current_square, white_pieces, black_pieces, !white) & king) attackers |= 1ULL << current_square; break;
		case ROOK: if (Moves::getRookMoves(current_square, white_pieces, black_pieces, !white) & king) attackers |= 1ULL << current_square; break;
		case QUEEN: if (Moves::getQueenMoves(current_square, white_pieces, black_pieces, !white) & king) attackers |= 1ULL << current_square; break;
		case KING: break;
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
	case PAWN: break; // Must be captured
	case KNIGHT: break; // Must be captured
	case BISHOP: attack_ray |= formAttackingRay(attacker, king); break;
	case ROOK: attack_ray |= formAttackingRay(attacker, king); break;
	case QUEEN: attack_ray |= formAttackingRay(attacker, king); break;
	case KING: break; // King can't capture another king
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
	int direction = Utils::get_direction(diff);
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
	friendly &= ~(white ? piece_bitboards[WHITE][KING] : piece_bitboards[BLACK][KING]); // Exclude own king

	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();;

	// Loop over own pieces and get their possible attacks at the current square
	// If the move is able to block the attack ray returns true
	uint64_t possible_moves = 0ULL;
	while (friendly != 0) {
		int current_square = Utils::findLastSetBit(friendly); // Isolate LSB and get as index
		PieceType piece_type = getPieceType(current_square);
		// Get moves depending on the piece type
		switch (piece_type)
		{
		case PAWN: possible_moves = Moves::getPawnMoves(current_square, white_pieces, black_pieces, white, en_passant_target); break;
		case KNIGHT: possible_moves = Moves::getKnightMoves(current_square, white_pieces, black_pieces, white); break;
		case BISHOP: possible_moves = Moves::getBishopMoves(current_square, white_pieces, black_pieces, white); break;
		case ROOK: possible_moves = Moves::getRookMoves(current_square, white_pieces, black_pieces, white); break;
		case QUEEN: possible_moves = Moves::getQueenMoves(current_square, white_pieces, black_pieces, white); break;
		default: throw std::invalid_argument("Invalid piece type");
		}
		friendly ^= 1ULL << current_square; // Remove the processed square

		// Check for ability to block
		if (possible_moves & attack_ray) return true;
	}
	return false; // No blocks were found
}

void Bitboard::updateBoardState(bool white) {
	state.flags = 0; // Reset state before updating
	
	// TODO
	// Update attack tables here

	// Check/checkmate/stalemate check
	if (isInCheck(!white)) {
		state.flags |= white ? BoardState::CHECK_BLACK : BoardState::CHECK_WHITE;
		if (isCheckmate(!white)) { // Only if in check continue to checkmate 
			state.flags |=  white ? BoardState::CHECKMATE_BLACK : BoardState::CHECKMATE_WHITE;
		}
	}
	else if (isStalemate(!white)) {
		state.flags |= BoardState::STALEMATE;
	}

	// TODO
	// Additional state updates
	// Like Zobrist
}

void Bitboard::resetUndoStack() {
	undo_stack_top = 0;
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
	move_count = 0;
	std::array<std::pair<uint32_t, int>, MAX_MOVES> move_scores;  // Stack-allocated array

	// Generate all moves directly into move_scores with scoring
	uint64_t friendly_pieces = white ? whitePieces() : blackPieces();
	while (friendly_pieces != 0) {
		int from = Utils::findFirstSetBit(friendly_pieces);
		PieceType piece = getPieceType(from);
		uint64_t legal_moves = getLegalMoves(from, white);

		while (legal_moves != 0) {
			int to = Utils::findFirstSetBit(legal_moves);
			PieceType target_piece = getPieceType(to);
			MoveType move_type = getMoveType(from, to, piece, target_piece, white);

			// Score moves immediately (MVV-LVA for captures, 0 for quiet)
			int score = (move_type == CAPTURE || move_type == PROMOTION_CAPTURE || move_type == EN_PASSANT)
				? Utils::get_mvv_lva_score(piece, (move_type == EN_PASSANT ? PAWN : target_piece))
				: 0;

			if (move_type == PROMOTION || move_type == PROMOTION_CAPTURE) {
				// Add all promotion options with same score
				move_scores[move_count++] = { ChessAI::encodeMove(from, to, piece, target_piece, move_type, QUEEN, false), score };
				move_scores[move_count++] = { ChessAI::encodeMove(from, to, piece, target_piece, move_type, ROOK, false), score };
				move_scores[move_count++] = { ChessAI::encodeMove(from, to, piece, target_piece, move_type, BISHOP, false), score };
				move_scores[move_count++] = { ChessAI::encodeMove(from, to, piece, target_piece, move_type, KNIGHT, false), score };
			}
			else {
				move_scores[move_count++] = { ChessAI::encodeMove(from, to, piece, target_piece, move_type, EMPTY, move_type == EN_PASSANT), score };
			}

			legal_moves ^= 1ULL << to;
		}
		friendly_pieces ^= 1ULL << from;
	}

	// Sort only the portion containing actual moves
	std::sort(move_scores.begin(), move_scores.begin() + move_count,
		[](const auto& a, const auto& b) { return a.second > b.second; });

	// Extract just the moves
	for (int i = 0; i < move_count; ++i) {
		move_list[i] = move_scores[i].first;
	}
}

void Bitboard::generateNoisyMoves(std::array<uint32_t, MAX_MOVES>& move_list, int& move_count, bool white) {
	move_count = 0;
	uint64_t friendly_pieces = white ? whitePieces() : blackPieces();
	uint64_t opponent_pieces = white ? blackPieces() : whitePieces();

	while (friendly_pieces != 0) {
		int from = Utils::findFirstSetBit(friendly_pieces);
		PieceType piece = getPieceType(from);
		uint64_t legal_moves = getLegalMoves(from, white);
		uint64_t captures = legal_moves & opponent_pieces;

		// Process captures first
		while (captures != 0) {
			int to = Utils::findFirstSetBit(captures);
			PieceType target_piece = getPieceType(to);
			MoveType move_type = getMoveType(from, to, piece, target_piece, white);

			if (move_type == PROMOTION_CAPTURE) {
				move_list[move_count++] = ChessAI::encodeMove(from, to, piece, target_piece, move_type, QUEEN, false);
				move_list[move_count++] = ChessAI::encodeMove(from, to, piece, target_piece, move_type, ROOK, false);
				move_list[move_count++] = ChessAI::encodeMove(from, to, piece, target_piece, move_type, BISHOP, false);
				move_list[move_count++] = ChessAI::encodeMove(from, to, piece, target_piece, move_type, KNIGHT, false);
			}
			else {
				move_list[move_count++] = ChessAI::encodeMove(from, to, piece, target_piece, move_type, EMPTY, false);
			}
			captures ^= 1ULL << to;
		}

		// Process en passant if applicable
		if (piece == PAWN && en_passant_target != UNASSIGNED) {
			uint64_t ep_mask = 1ULL << en_passant_target;
			if (legal_moves & ep_mask) {
				if (move_count < MAX_MOVES) { // Safety check
					move_list[move_count++] = ChessAI::encodeMove(from, en_passant_target, PAWN, PAWN, EN_PASSANT, EMPTY, true);
				}
			}
		}

		// Process promotions (non-captures)
		if (piece == PAWN) {
			uint64_t promotions = legal_moves & (white ? RANK_8 : RANK_1) & ~opponent_pieces;
			while (promotions != 0) {
				int to = Utils::findFirstSetBit(promotions);
				if (move_count < MAX_MOVES - 4) { // Safety check
					move_list[move_count++] = ChessAI::encodeMove(from, to, PAWN, EMPTY, PROMOTION, QUEEN, false);
					move_list[move_count++] = ChessAI::encodeMove(from, to, PAWN, EMPTY, PROMOTION, ROOK, false);
					move_list[move_count++] = ChessAI::encodeMove(from, to, PAWN, EMPTY, PROMOTION, BISHOP, false);
					move_list[move_count++] = ChessAI::encodeMove(from, to, PAWN, EMPTY, PROMOTION, KNIGHT, false);
				}
				promotions ^= 1ULL << to;
			}
		}

		friendly_pieces ^= 1ULL << from;
	}

	// Sort using MVV-LVA
	std::sort(move_list.begin(), move_list.begin() + move_count,
		[this](uint32_t a, uint32_t b) {
			return Utils::get_mvv_lva_score(ChessAI::piece(a), ChessAI::capturedPiece(a)) > Utils::get_mvv_lva_score(ChessAI::piece(b), ChessAI::capturedPiece(b));
		});
}

void Bitboard::applyMoveAI(uint32_t move, bool white) {
	// Decode source and target squares
	int source = ChessAI::from(move);
	int target = ChessAI::to(move);

	// Save state to undo stack (hot path - optimized)
	assert(undo_stack_top < MAX_SEARCH_DEPTH);
	UndoInfo& current = undo_stack[undo_stack_top++];
	current.castling_rights = castling_rights;
	current.en_passant_target = en_passant_target;
	current.flags = state.flags;

	// Call applyMove depending which turn ongoing
	// applyMove handles all the move logic
	applyMove(source, target, white);

	// Handle promotion
	if (ChessAI::moveType(move) == PROMOTION || ChessAI::moveType(move) == PROMOTION_CAPTURE) {
		// Get promotion piece and promote pawn
		PieceType promotion = ChessAI::promotion(move);
		switch (promotion) {
		case QUEEN: applyPromotion(target, 'q', white); break;
		case ROOK: applyPromotion(target, 'r', white); break;
		case BISHOP: applyPromotion(target, 'b', white); break;
		case KNIGHT: applyPromotion(target, 'n', white); break;
		default: throw std::invalid_argument("Invalid promotion piece");
		}
	}
}

void Bitboard::undoMoveAI(uint32_t move, bool white) {
	// Decode the move
	int source = ChessAI::from(move);
	int target = ChessAI::to(move);
	PieceType source_piece = ChessAI::piece(move);
	PieceType target_piece = ChessAI::capturedPiece(move);
	MoveType move_type = ChessAI::moveType(move);
	PieceType promotion = ChessAI::promotion(move);

	// Restore board state
	assert(undo_stack_top > 0);
	const UndoInfo& prev = undo_stack[--undo_stack_top];
	castling_rights = prev.castling_rights;
	en_passant_target = prev.en_passant_target;
	state.flags = prev.flags;

	// Move source piece back to source square
	// Doesn't differ for any move type
	uint64_t& source_bitboard = getPieceBitboard(source_piece, white);
	source_bitboard |= 1ULL << source; // Move to original position

	// Handle special cases

	// Restore captured piece if move was a capture
	if (move_type == CAPTURE || move_type == PROMOTION_CAPTURE) {
		uint64_t& target_bitboard = getPieceBitboard(target_piece, !white);
		target_bitboard |= 1ULL << target; // Restore captured piece
	}

	// Restore en passant pawn if move was en passant
	if (move_type == EN_PASSANT) {
		// Determine en passant square
		int en_passant_square = white ? (target - 8) : (target + 8);
		uint64_t& en_passant_pawn = getPieceBitboard(PAWN, !white);
		en_passant_pawn |= 1ULL << en_passant_square; // Restore captured pawn
	}

	// Restore rook to original position if move was castling
	if (move_type == CASTLING) {
		// Determine if kingside or queenside castling
		bool kingside = target == 6 || target == 62;
		undoCastling(white, kingside);
	}

	// Restore promotion piece if move was promotion
	if (move_type == PROMOTION || move_type == PROMOTION_CAPTURE) {
		uint64_t& promoted_piece = getPieceBitboard(promotion, white);
		promoted_piece &= ~(1ULL << target); // Clear promotion square
	}

	// Remove the piece from the target square (applies to all non-promotion moves)
	if (move_type != PROMOTION && move_type != PROMOTION_CAPTURE) {
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

bool Bitboard::isGameOver() {
	return state.isCheckmateWhite() || state.isCheckmateBlack() || state.isStalemate();
}

int Bitboard::calculateKingMobility(bool white) {
	// Determine king square and get king moves
	// Return the set bits of the moves bitboard (amount of legal moves)
	if (white) {
		return Utils::countSetBits(getKingMoves(Utils::findFirstSetBit(piece_bitboards[WHITE][KING]), whitePieces(), blackPieces(), true));
	}
	else {
		return Utils::countSetBits(getKingMoves(Utils::findFirstSetBit(piece_bitboards[BLACK][KING]), whitePieces(), blackPieces(), false));
	}
}

PieceType Bitboard::getPieceType(int square) const {
	uint64_t bitboard = 1ULL << square;
	// Determine piece type at square
	if (bitboard & (piece_bitboards[WHITE][PAWN] | piece_bitboards[BLACK][PAWN])) return PAWN;
	if (bitboard & (piece_bitboards[WHITE][KNIGHT] | piece_bitboards[BLACK][KNIGHT])) return KNIGHT;
	if (bitboard & (piece_bitboards[WHITE][BISHOP] | piece_bitboards[BLACK][BISHOP])) return BISHOP;
	if (bitboard & (piece_bitboards[WHITE][ROOK] | piece_bitboards[BLACK][ROOK])) return ROOK;
	if (bitboard & (piece_bitboards[WHITE][QUEEN] | piece_bitboards[BLACK][QUEEN])) return QUEEN;
	if (bitboard & (piece_bitboards[WHITE][KING] | piece_bitboards[BLACK][KING])) return KING;
	return EMPTY;
}

uint64_t& Bitboard::getPieceBitboard(PieceType piece, bool white) {
	// Return the correct piece bitboard depending on the piece and color
	switch (piece)
	{
	case PAWN: return white ? piece_bitboards[WHITE][PAWN] : piece_bitboards[BLACK][PAWN];
	case KNIGHT: return white ? piece_bitboards[WHITE][KNIGHT] : piece_bitboards[BLACK][KNIGHT];
	case BISHOP: return white ? piece_bitboards[WHITE][BISHOP] : piece_bitboards[BLACK][BISHOP];
	case ROOK: return white ? piece_bitboards[WHITE][ROOK] : piece_bitboards[BLACK][ROOK];
	case QUEEN: return white ? piece_bitboards[WHITE][QUEEN] : piece_bitboards[BLACK][QUEEN];
	case KING: return white ? piece_bitboards[WHITE][KING] : piece_bitboards[BLACK][KING];
	default: throw std::invalid_argument("Invalid piece type");
	}
}

MoveType Bitboard::getMoveType(int source_square, int target_square, PieceType piece, PieceType target_piece, bool white) const {
	// Determine move type
	if (piece == PAWN) {
		if (target_square == en_passant_target) return EN_PASSANT;
		if ((white && target_square >= 56) || (!white && target_square <= 7)) {
			return (target_piece == EMPTY) ? PROMOTION : PROMOTION_CAPTURE;
		}
	}
	if (piece == KING && abs(source_square - target_square) == 2) return CASTLING;
	if (target_piece != EMPTY) return CAPTURE;
	return NORMAL;
}

void Bitboard::undoCastling(bool white, bool kingside) {
	uint64_t rook;
	if (white) {
		if (kingside) {
			rook = (1ULL << 5); // White rook on f1
			piece_bitboards[WHITE][ROOK] &= ~rook; // Remove rook from f1
			piece_bitboards[WHITE][ROOK] |= (1ULL << 7); // Move rook to h1
		}
		else {
			rook = (1ULL << 3); // White rook on d1
			piece_bitboards[WHITE][ROOK] &= ~rook; // Remove rook from d1
			piece_bitboards[WHITE][ROOK] |= (1ULL << 0); // Move rook to a1
		}
	}
	else {
		if (kingside) {
			rook = (1ULL << 61); // Black rook on f8
			piece_bitboards[BLACK][ROOK] &= ~rook; // Remove rook from f8
			piece_bitboards[BLACK][ROOK] |= (1ULL << 63); // Move rook to h8
		}
		else {
			rook = (1ULL << 59); // Black rook on d8
			piece_bitboards[BLACK][ROOK] &= ~rook; // Remove rook from d8
			piece_bitboards[BLACK][ROOK] |= (1ULL << 56); // Move rook to a8
		}
	}
}

int Bitboard::calculateMaterialScore(bool white) {
	int white_score = 0;
	int black_score = 0;

	// Get material score for all pieces in the board
	// Done by counting the number of set bits in the bitboards (amount of pieces) and multiplying by the piece value
	white_score += Utils::countSetBits(piece_bitboards[WHITE][PAWN]) * PIECE_VALUES[PAWN];
	white_score += Utils::countSetBits(piece_bitboards[WHITE][KNIGHT]) * PIECE_VALUES[KNIGHT];
	white_score += Utils::countSetBits(piece_bitboards[WHITE][BISHOP]) * PIECE_VALUES[BISHOP];
	white_score += Utils::countSetBits(piece_bitboards[WHITE][ROOK]) * PIECE_VALUES[ROOK];
	white_score += Utils::countSetBits(piece_bitboards[WHITE][QUEEN]) * PIECE_VALUES[QUEEN];
	white_score += Utils::countSetBits(piece_bitboards[WHITE][KING]) * PIECE_VALUES[KING];

	black_score += Utils::countSetBits(piece_bitboards[BLACK][PAWN]) * PIECE_VALUES[PAWN];
	black_score += Utils::countSetBits(piece_bitboards[BLACK][KNIGHT]) * PIECE_VALUES[KNIGHT];
	black_score += Utils::countSetBits(piece_bitboards[BLACK][BISHOP]) * PIECE_VALUES[BISHOP];
	black_score += Utils::countSetBits(piece_bitboards[BLACK][ROOK]) * PIECE_VALUES[ROOK];
	black_score += Utils::countSetBits(piece_bitboards[BLACK][QUEEN]) * PIECE_VALUES[QUEEN];
	black_score += Utils::countSetBits(piece_bitboards[BLACK][KING]) * PIECE_VALUES[KING];

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
		int square = Utils::findLastSetBit(white_pieces); // Extract LSB
		PieceType piece = getPieceType(square); // Get piece type at square
		int row = 7 - (square / 8); // Convert square to row (0-7) for white
		int col = square % 8; // Convert square to column (0-7)

		// Add positional score for white
		if (piece == PAWN) white_score += (game_phase * PawnTableMid[row][col] + (1 - game_phase) * PawnTableEnd[row][col]);
		else if (piece == KNIGHT) white_score += (game_phase * KnightTableMid[row][col] + (1 - game_phase) * KnightTableEnd[row][col]);
		else if (piece == BISHOP) white_score += (game_phase * BishopTableMid[row][col] + (1 - game_phase) * BishopTableEnd[row][col]);
		else if (piece == ROOK) white_score += (game_phase * RookTableMid[row][col] + (1 - game_phase) * RookTableEnd[row][col]);
		else if (piece == QUEEN) white_score += (game_phase * QueenTableMid[row][col] + (1 - game_phase) * QueenTableEnd[row][col]);
		else if (piece == KING) white_score += (game_phase * KingTableMid[row][col] + (1 - game_phase) * KingTableEnd[row][col]);

		white_pieces ^= 1ULL << square; // Remove the processed square
	}

	uint64_t black_pieces = blackPieces();
	while (black_pieces != 0) {
		int square = Utils::findLastSetBit(black_pieces); // Extract LSB
		PieceType piece = getPieceType(square); // Get piece type at square
		int row = square / 8; // Convert square to row (0-7) for black
		int col = 7 - (square % 8); // Convert square to column (0-7)

		// Add positional score for black
		if (piece == PAWN) black_score += (game_phase * PawnTableMid[row][col] + (1 - game_phase) * PawnTableEnd[row][col]);
		else if (piece == KNIGHT) black_score += (game_phase * KnightTableMid[row][col] + (1 - game_phase) * KnightTableEnd[row][col]);
		else if (piece == BISHOP) black_score += (game_phase * BishopTableMid[row][col] + (1 - game_phase) * BishopTableEnd[row][col]);
		else if (piece == ROOK) black_score += (game_phase * RookTableMid[row][col] + (1 - game_phase) * RookTableEnd[row][col]);
		else if (piece == QUEEN) black_score += (game_phase * QueenTableMid[row][col] + (1 - game_phase) * QueenTableEnd[row][col]);
		else if (piece == KING) black_score += (game_phase * KingTableMid[row][col] + (1 - game_phase) * KingTableEnd[row][col]);

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

	game_phase += 4 * (Utils::countSetBits(piece_bitboards[WHITE][QUEEN]) + Utils::countSetBits(piece_bitboards[BLACK][QUEEN]));
	game_phase += 2 * (Utils::countSetBits(piece_bitboards[WHITE][ROOK]) + Utils::countSetBits(piece_bitboards[BLACK][ROOK]));
	game_phase += Utils::countSetBits(piece_bitboards[WHITE][KNIGHT]) + Utils::countSetBits(piece_bitboards[BLACK][KNIGHT]);

	// Normalize the game_phase to the range [0, 1]
	float normalized_game_phase = static_cast<float>(game_phase) / MAX_GAME_PHASE;

	// Ensure the value is clamped between 0 and 1 (in case of edge cases)
	normalized_game_phase = max(0.0f, min(1.0f, normalized_game_phase));

	return normalized_game_phase;
}

int Bitboard::estimateCaptureValue(uint32_t move) {
	// Extract move information
	int from_sq = ChessAI::from(move);
	int to_sq = ChessAI::to(move);
	MoveType move_type = ChessAI::moveType(move);

	// Get pieces involved
	PieceType captured_piece = ChessAI::capturedPiece(move);
	PieceType attacking_piece = ChessAI::piece(move);

	// Value of captured piece
	int capture_value = PIECE_VALUES[captured_piece];

	// Handle promotions
	if (move_type == PROMOTION || move_type == PROMOTION_CAPTURE) {
		PieceType promo_piece = ChessAI::promotion(move);
		capture_value += PIECE_VALUES[promo_piece] - PIECE_VALUES[PAWN];
	}

	// If we're losing the more valuable piece (bad trade)
	int trade_delta = PIECE_VALUES[attacking_piece] - PIECE_VALUES[captured_piece];

	// Return net gain (could be negative for bad trades)
	return capture_value - (trade_delta > 0 ? PIECE_VALUES[attacking_piece] : 0);
}
