#include "pch.h"
#include "Bitboard.hpp"
#include "MoveTables.hpp"
#include "ChessAI.hpp"
#include "Tables.hpp"
#include "Moves.hpp"
#include "Utils.hpp"
#include "Scoring.hpp"


Bitboard::Bitboard():
	castling_rights(0x0F),                 // All castling rights (0b00001111)
	en_passant_target(UNASSIGNED),         // None
	half_moves(0),                         // Initially 0
	hash_key(0)                           
{
	initBoard();
}

void Bitboard::initBoard() {
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

	// Initialize the piece_at_square lookup table
	std::fill(std::begin(piece_at_square), std::end(piece_at_square), EMPTY);
	for (int color = BLACK; color <= WHITE; ++color) { // 0 = BLACK, 1 = WHITE
		for (int piece = PAWN; piece <= KING; ++piece) {
			uint64_t bitboard = piece_bitboards[color][piece];
			while (bitboard) {
				int square = Utils::findFirstSetBit(bitboard);  // Get least significant set bit
				piece_at_square[square] = static_cast<PieceType>(piece);
				bitboard &= bitboard - 1;  // Clear LSB
			}
		}
	}

	// Initialize pin-data (initially none)
	for (int i = 0; i < 64; ++i) {
		pin_data.pin_rays[i] = 0xFFFFFFFFFFFFFFFFULL;
	}

	// Initialize attack-data
	attack_data.attack_ray = 0xFFFFFFFFFFFFFFFFULL;
	attack_data.attack_squares = 0ULL;

	// Material and positional scores are initially 0 since equal amount of pieces
	material_score = 0;
	positional_score = 0;
	game_phase_score = MAX_GAME_PHASE; // Max game phase == beginning

	state.flags = 0; // Empty game state at beginning (no check, no checkmate, no stalemate)

	// Reserve space for undo-stacks
	undo_stack.reserve(MAX_SEARCH_DEPTH);
	search_history.reserve(MAX_SEARCH_DEPTH);

	// Compute initial Zobrist key which we update incrementally onwards
	hash_key = computeZobristHash();
	position_history[hash_key]++; // Save initial state
}

uint64_t Bitboard::computeZobristHash() {
	uint64_t hash = 0;

	// XOR piece keys
	for (int color = BLACK; color <= WHITE; ++color) {
		for (int piece = PAWN; piece <= KING; ++piece) {
			uint64_t bitboard = piece_bitboards[color][piece];
			while (bitboard) {
				int square = Utils::findFirstSetBit(bitboard);  // Get least significant set bit
				hash ^= Tables::PIECE_KEYS[color][piece][square];
				bitboard &= bitboard - 1;  // Clear LSB
			}
		}
	}

	// XOR castling rights
	hash ^= Tables::CASTLING_KEYS[castling_rights];

	// No need to XOR EN_PASSANT_KEY since no en passant target initially
	// No need to XOR SIDE_TO_MOVE_KEY since White starts

	return hash;
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
	else if (state.isDraw()) {
		game_state = "D";
	}
	else {
		game_state = "-";
	}
	return game_state;
}

std::string Bitboard::squareToString(int square) const {
	char file = 'a' + (square % 8);
	char rank = '1' + (square / 8);
	return std::string() + file + rank;
}

bool Bitboard::isCheckmate(bool white) {
	if (!(white ? state.isCheckWhite() : state.isCheckBlack())) {
		return false; // Not in check, so not checkmate

	}
	uint64_t king_bitboard = white ? piece_bitboards[WHITE][KING] : piece_bitboards[BLACK][KING];
	int king_square = Utils::findFirstSetBit(king_bitboard);

	// Get currently possible king moves
	uint64_t king_moves = getLegalMoves(king_square, white);

	// Check if the king has moves left -> can escape
	if (king_moves) return false;

	// If cannot be blocked -> checkmate
	return !canBlock(white);
}

bool Bitboard::isStalemate(bool white) {
	// Loop over each friendly piece and check if there are legal moves
	// If none of the pieces have legal moves, results in stalemate
	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();
	uint64_t friendly = white ? white_pieces : black_pieces;
	uint64_t possible_moves = 0ULL;
	while (friendly) {
		int current_square = Utils::findFirstSetBit(friendly); // Isolate LSB and get as index
		Utils::popBit(friendly, current_square); // Remove the processed square
		// Get moves 
		possible_moves = getLegalMoves(current_square, white);

		// Check if any moves
		if (possible_moves) return false;
	}
	return true;
}

int Bitboard::getHalfMoveClock() const {
	return half_moves;
}

uint64_t Bitboard::getLegalMoves(int from, bool white) {
	PieceType piece = piece_at_square[from]; // Get piece type at square

	// Get both pieces as bitboards
	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();

	uint64_t legal_moves = 0ULL;
	// Pawns are handled separately
	if (piece == PAWN) {
		legal_moves = Moves::getPawnMoves(from, white_pieces, black_pieces, white, en_passant_target);
	}
	else {
		legal_moves = Moves::getPseudoLegalMoves(from, piece, white_pieces | black_pieces);
		// Filter out own pieces
		legal_moves &= ~(white ? white_pieces : black_pieces);
	}

	uint64_t enemy_king = white ? piece_bitboards[BLACK][KING] : piece_bitboards[WHITE][KING]; // Get enemy king

	// Filter king moves
	if (piece == KING) {
		// First check ability to castle
		bool castling_available = (castling_rights & (white ? 0x03 : 0x0C)) != 0 &&
			(white ? (from == 4) : (from == 60));
		// Add if possible
		if (castling_available) {
			legal_moves |= getCastlingMoves(white);
		}

		// King cannot move to any of the enemy control squares
		// Includes squares attacked by enemy and enemy kings control squares
		int enemy_king_sq = Utils::findFirstSetBit(enemy_king);
		uint64_t enemy_king_control = Moves::getKingMoves(enemy_king_sq); // Get enemy king control
		uint64_t enemy_control = attack_data.attack_squares | enemy_king_control; // Combine with attack squares

		// King cannot move onto any of the squares attacked by enemy
		legal_moves &= ~attack_data.attack_squares;
	}
	else {
		uint64_t piece_bb = 1ULL << from;
		// If a piece is pinned it can only move along its pin ray
		if (pin_data.pinned & piece_bb) {
			legal_moves &= pin_data.pin_rays[from];
		}
		// If there is a current attacker to king, the ray must be blocked, 
		// meaning only allowed to move to squares along it
		legal_moves &= attack_data.attack_ray; // Attack ray is only ones if none, so no need for checking
	}
	// Exclude enemy king from moves
	legal_moves &= ~enemy_king;

	return legal_moves;
}

uint32_t Bitboard::applyMove(int source, int target, PieceType promotion, bool white) {
	// Get piece types at squares
	PieceType source_piece = piece_at_square[source];
	PieceType target_piece = piece_at_square[target];
	MoveType move_type = getMoveType(source, target, source_piece, target_piece, white);

	// Clear the source square
	piece_bitboards[white][source_piece] &= ~(1ULL << source);
	piece_at_square[source] = EMPTY;
	hash_key ^= Tables::PIECE_KEYS[white][source_piece][source]; 

	// Precompute whether castling is affected
	bool castling_affected = (castling_rights & (white ? 0x03 : 0x0C)) != 0;
	hash_key ^= Tables::CASTLING_KEYS[castling_rights]; // Clear old castling rights from hash

	// If a rook or knight moved, update castling rights
	if (castling_affected && (source_piece == ROOK || source_piece == KING)) {
		if (source_piece == ROOK) updateRookCastling(white, source);
		else if (source_piece == KING) castling_rights &= ~(white ? 0x03 : 0x0C);  // Disable castling rights
	}

	// If capture, clear target square and update scores
	if (move_type == CAPTURE || move_type == PROMOTION_CAPTURE) {
		piece_bitboards[!white][target_piece] &= ~(1ULL << target);
		hash_key ^= Tables::PIECE_KEYS[!white][target_piece][target];

		// Update game phase
		if (target_piece == QUEEN) game_phase_score -= 4;
		else if (target_piece == ROOK) {
			if ((castling_rights & (white ? 0x0C : 0x03)) != 0) updateRookCastling(!white, target); // Update castling
			game_phase_score -= 2;
		}
		else if (target_piece == KNIGHT || target_piece == BISHOP) game_phase_score -= 1;

		// Update material score
		material_score += white ? PIECE_VALUES[target_piece] : -PIECE_VALUES[target_piece];
	}

	// En passant
	if (move_type == EN_PASSANT) {
		// Compute the pawn captured by en passant
		int en_passant_square = white ? (target - 8) : (target + 8);
		piece_bitboards[!white][PAWN] &= ~(1ULL << en_passant_square); // Capture pawn
		piece_at_square[en_passant_square] = EMPTY;
		hash_key ^= Tables::PIECE_KEYS[!white][PAWN][en_passant_square];

		material_score += white ? PIECE_VALUES[PAWN] : -PIECE_VALUES[PAWN];
	}

	// Castling
	if (move_type == CASTLING) {
		handleCastling(white, target);

		// Determine if kingside or queenside castling
		bool kingside = target == 6 || target == 62;

		// Get rook origin and target
		int rook_origin = white ? (kingside ? 7 : 0) : (kingside ? 63 : 56);
		int rook_target = white ? (kingside ? 5 : 3) : (kingside ? 61 : 59);

		// Update hash
		hash_key ^= Tables::PIECE_KEYS[white][ROOK][rook_origin]; // Clear origin
		hash_key ^= Tables::PIECE_KEYS[white][ROOK][rook_target]; // Set target
	}

	// Promotion
	if (move_type == PROMOTION || move_type == PROMOTION_CAPTURE) {
		// Update promoted pieces bitboard
		piece_bitboards[white][promotion] |= (1ULL << target);
		piece_at_square[target] = promotion;
		hash_key ^= Tables::PIECE_KEYS[white][promotion][target];

		// Update board state
		material_score += white ? PIECE_VALUES[promotion] : -PIECE_VALUES[promotion];
		material_score += white ? -PIECE_VALUES[PAWN] : PIECE_VALUES[PAWN];
	}
	else { // For the non promotion moves move source piece to target
		piece_bitboards[white][source_piece] |= (1ULL << target);
		piece_at_square[target] = source_piece;
		hash_key ^= Tables::PIECE_KEYS[white][source_piece][target];
	}

	// Clear previous en passant if available
	if (en_passant_target != UNASSIGNED) {
		hash_key ^= Tables::EN_PASSANT_KEYS[en_passant_target % 8];
	}

	// Set new castling rights
	hash_key ^= Tables::CASTLING_KEYS[castling_rights];

	// Set en passant target if a pawn double pushes
	if (move_type == PAWN_DOUBLE_PUSH) {
		en_passant_target = white ? (source + 8) : (target + 8);
		hash_key ^= Tables::EN_PASSANT_KEYS[target % 8]; // Set new en passant file
	}
	else {
		en_passant_target = UNASSIGNED; // Reset en passant
	}

	// Toggle side-to-move key
	hash_key ^= Tables::SIDE_TO_MOVE_KEY;

	// Get new board state
	updateBoardState(white);
	updatePositionalScore();

	// For reversible moves increment half-moves
	// Position history saved for also irreversible moves, but cleared before applying
	// If reversable, check for draw by repetition
	if (!(source_piece == PAWN || move_type == CAPTURE || move_type == CASTLING)) {
		half_moves++;
		position_history[hash_key]++; // Save new state
		updateDrawByRepetition();
	}
	else {
		half_moves = 0; // Reset half-moves if irreversible
		position_history.clear(); // Reset threefold tracking
		position_history[hash_key]++; // Save new state
	}

	// Increase ply count
	ply_count++;

	// Finally return the encoded move
	return ChessAI::encodeMove(source, target, source_piece, target_piece, move_type, promotion, false); // Don't add check flag
}

bool Bitboard::isEndgame() {
	// Condition 1: No queens or only one side has a queen
	int queens = Utils::countSetBits(piece_bitboards[WHITE][QUEEN]) + Utils::countSetBits(piece_bitboards[BLACK][QUEEN]);
	if (queens <= 1) return true;

	// Condition 2: Few total non-pawn pieces (e.g., 4 or fewer)
	int total = Utils::countSetBits(whitePieces() | blackPieces());
	int pawns = Utils::countSetBits(piece_bitboards[WHITE][PAWN]) + Utils::countSetBits(piece_bitboards[BLACK][PAWN]);
	if (total - pawns <= 4) return true;

	// Condition 3: Only pawns + kings remain
	int kings = Utils::countSetBits(piece_bitboards[WHITE][KING]) + Utils::countSetBits(piece_bitboards[BLACK][KING]);
	if (total == kings + pawns) return true;

	return false; // No conditions filled
}

void Bitboard::updateDrawByRepetition() {
	if (position_history[hash_key] >= 3) {
		state.flags |= BoardState::DRAW_REPETITION;
	}
	else if (half_moves >= 50) {
		state.flags |= BoardState::DRAW_50;
	}
}

int Bitboard::getPlyCount() const {
	return ply_count;
}

uint64_t Bitboard::whitePieces() {
	return piece_bitboards[WHITE][PAWN] | piece_bitboards[WHITE][ROOK] | piece_bitboards[WHITE][KNIGHT] |
		piece_bitboards[WHITE][BISHOP] | piece_bitboards[WHITE][QUEEN] | piece_bitboards[WHITE][KING];
}

uint64_t Bitboard::blackPieces() {
	return piece_bitboards[BLACK][PAWN] | piece_bitboards[BLACK][ROOK] | piece_bitboards[BLACK][KNIGHT] |
		piece_bitboards[BLACK][BISHOP] | piece_bitboards[BLACK][QUEEN] | piece_bitboards[BLACK][KING];
}

uint64_t Bitboard::getCastlingMoves(bool white) {
	// Initialize castling moves
	uint64_t castling_moves = 0ULL;
	uint64_t occupied = whitePieces() | blackPieces();

	// If is in check, cannot castle
	if (white ? state.isCheckWhite() : state.isCheckBlack()) return 0ULL;

	uint64_t critical_squares;
	// Depending on player turn and castling availability add available castling moves
	if (white) {
		if (castling_rights & 0x01) { // White Kingside
			if ((occupied & WHITE_KINGSIDE_CASTLE_SQUARES) == 0) { // f1 and g1 must be free
				// King cannot castle out of, through, or into check
				// Get squares that can't be under attack
				critical_squares = WHITE_KINGSIDE_CASTLE_SQUARES;

				// Now compare with opponents attack squares and make sure no squares align (bitwise AND)
				// // Ensure the king does not move through or into check
				// If castling available, add to moves
				if (!(critical_squares & attack_data.attack_squares)) {
					castling_moves |= 1ULL << 6; // King moves to g1
				}
			}
		}
		if (castling_rights & 0x02) { // White Queenside
			if ((occupied & WHITE_QUEENSIDE_CASTLE_SQUARES) == 0) { // b1, c1 and d1 must be free
				critical_squares = WHITE_QUEENSIDE_CASTLE_SQUARES;
				if (!(critical_squares & attack_data.attack_squares)) {
					castling_moves |= 1ULL << 2; // King moves to c1
				}
			}
		}
	}
	else {
		if (castling_rights & 0x04) { // Black Kingside
			if ((occupied & BLACK_KINGSIDE_CASTLE_SQUARES) == 0) { // f8 and g8 must be free
				critical_squares = BLACK_KINGSIDE_CASTLE_SQUARES; // e8, f8, g8
				if (!(critical_squares & attack_data.attack_squares)) {
					castling_moves |= 1ULL << 62; // King moves to g8
				}
			}
		}
		if (castling_rights & 0x08) { // Black Queenside
			if ((occupied & BLACK_QUEENSIDE_CASTLE_SQUARES) == 0) { // c8 and d8 must be free
				critical_squares = BLACK_QUEENSIDE_CASTLE_SQUARES; // e8, d8, c8
				if (!(critical_squares & attack_data.attack_squares)) {
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
	if (white) {
		// White castling: Kingside (h1 -> f1), Queenside (a1 -> d1)
		if (target == 6) { // Kingside castling (g1)
			piece_bitboards[WHITE][ROOK] &= ~ROOK_H1; // Remove rook from h1
			piece_bitboards[WHITE][ROOK] |= ROOK_F1; // Move rook to f1

			// Also update piece types
			piece_at_square[7] = EMPTY;
			piece_at_square[5] = ROOK;
		}
		else if (target == 2) { // Queenside castling (c1)
			piece_bitboards[WHITE][ROOK] &= ~ROOK_A1; // Remove rook from a1
			piece_bitboards[WHITE][ROOK] |= ROOK_D1; // Move rook to d1

			piece_at_square[0] = EMPTY;
			piece_at_square[3] = ROOK;
		}
	}
	else {
		// Black castling: Kingside (h8 -> f8), Queenside (a8 -> d8)
		if (target == 62) { // Kingside castling (g8)
			piece_bitboards[BLACK][ROOK] &= ~ROOK_H8; // Remove rook from h8
			piece_bitboards[BLACK][ROOK] |= ROOK_F8; // Move rook to f8

			piece_at_square[63] = EMPTY;
			piece_at_square[61] = ROOK;
		}
		else if (target == 58) { // Queenside castling (c8)
			piece_bitboards[BLACK][ROOK] &= ~ROOK_A8; // Remove rook from a8
			piece_bitboards[BLACK][ROOK] |= ROOK_D8; // Move rook to d8

			piece_at_square[56] = EMPTY;
			piece_at_square[59] = ROOK;
		}
	}
}


void Bitboard::getAttackSquares(int enemy_king, const uint64_t& white_pieces, const uint64_t black_pieces, bool white) {
	// Reset previous attack squares and ray
	attack_data.attack_ray = 0xFFFFFFFFFFFFFFFFULL; // Full ray so moves don't get limited
	attack_data.attack_squares = 0ULL; // None

	uint64_t occupied = white_pieces | black_pieces;

	// Get pseudo-legal moves for our pieces
	// If a move lands on enemy king, update check and the attack ray
	uint64_t friendly = white ? white_pieces : black_pieces;
	while (friendly) {
		int current_square = Utils::findFirstSetBit(friendly);
		Utils::popBit(friendly, current_square);
		PieceType piece_type = piece_at_square[current_square];
		// Get pseudo-legal moves
		// If pawn get only capture moves since those are the attack squares
		uint64_t moves;
		if (piece_type == PAWN) {
			moves = Moves::getPawnCaptures(current_square, white);
		}
		else {
			moves = Moves::getPseudoLegalMoves(current_square, piece_type, occupied);
		}
		// If move landed on enemy king get the pre-computed attack ray
		if (moves & (1ULL << enemy_king)) {
			attack_data.attack_ray = Tables::BETWEEN[current_square][enemy_king] | (1ULL << current_square) | (1ULL << enemy_king);
			// Also update that the king is in check
			state.flags |= (white ? BoardState::CHECK_BLACK : BoardState::CHECK_WHITE);
		}
		attack_data.attack_squares |= moves; // Add to attack data
	}
}

bool Bitboard::canBlock(bool white) {
	// Get own pieces depending on the turn
	uint64_t friendly = white ? whitePieces() : blackPieces();
	friendly &= ~piece_bitboards[white][KING]; // Exclude own king

	// Loop over own pieces and get their possible attacks at the current square
	// If the move is able to block the attack ray returns true
	uint64_t possible_moves = 0ULL;
	while (friendly) {
		int current_square = Utils::findFirstSetBit(friendly); // Isolate LSB and get as index
		Utils::popBit(friendly, current_square); // Remove the processed square
		// Get moves
		possible_moves = getLegalMoves(current_square, white);

		// Check for ability to block
		if (possible_moves & attack_data.attack_ray) return true;
	}
	return false; // No blocks were found
}

void Bitboard::updateBoardState(bool white) {
	state.flags = 0; // Reset state before updating
	
	// Calculate pinned enemy pieces
	// Used for legal move generation
	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();

	uint64_t enemy_king = piece_bitboards[!white][KING];
	int king_bb = Utils::findFirstSetBit(enemy_king);

	Moves::computePinnedPieces(pin_data, king_bb, white_pieces | black_pieces, 
		piece_bitboards[white][BISHOP], piece_bitboards[white][ROOK], piece_bitboards[white][QUEEN]);

	// Now we calculate attack squares of the previously moved side
	// Exclude enemy king from calculation so we get the rays that pass through king
	// Also updates if the move got the enemy king in check and calculates the attack ray
	(white ? black_pieces : white_pieces) &= ~enemy_king;
	getAttackSquares(king_bb, white_pieces, black_pieces, white);

	// Check/checkmate/stalemate check
	if (state.isCheckBlack() || state.isCheckWhite()) {
		if (isCheckmate(!white)) { // Only if in check continue to checkmate 
			state.flags |=  white ? BoardState::CHECKMATE_BLACK : BoardState::CHECKMATE_WHITE;
		}
	}
	else if (isStalemate(!white)) {
		state.flags |= BoardState::STALEMATE;
	}
}

void Bitboard::updatePositionalScore() {
	// Reset positional score
	positional_score = 0;

	// Get game phase
	float game_phase = std::max(0.0f, std::min(1.0f, static_cast<float>(game_phase_score) / MAX_GAME_PHASE));

	// Get all pieces of both sides
	uint64_t white_pieces = whitePieces();
	uint64_t black_pieces = blackPieces();

	// Loop through both bitboard and update positional scores accordingly
	while (white_pieces) {
		int sq = Utils::findFirstSetBit(white_pieces);
		Utils::popBit(white_pieces, sq);
		positional_score += getPositionalScore(sq, game_phase, piece_at_square[sq], true);
	}
	while (black_pieces) {
		int sq = Utils::findFirstSetBit(black_pieces);
		Utils::popBit(black_pieces, sq);
		positional_score -= getPositionalScore(sq, game_phase, piece_at_square[sq], false);
	}
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

void Bitboard::startNewSearch() {
	// Clear stacks
	undo_stack.clear();
	search_history.clear();

	// Also reserve space for all the new potential elements to avoid dynamic resizing (causes overhead)
	undo_stack.reserve(MAX_SEARCH_DEPTH);
	search_history.reserve(MAX_SEARCH_DEPTH);
}

uint64_t Bitboard::getHashKey() {
	return hash_key;
}

void Bitboard::generateMoves(std::array<uint32_t, MAX_MOVES>& move_list, int& move_count, int depth, bool white, uint32_t move_hint) {
	move_count = 0;
	std::array<std::pair<uint32_t, int>, MAX_MOVES> move_scores; // Stack allocated array

	// Generate all moves directly into move_scores with scoring
	uint64_t friendly_pieces = white ? whitePieces() : blackPieces();
	while (friendly_pieces) {
		int from = Utils::findFirstSetBit(friendly_pieces);
		PieceType piece = piece_at_square[from];
		uint64_t legal_moves = getLegalMoves(from, white);

		while (legal_moves) {
			int to = Utils::findFirstSetBit(legal_moves);
			PieceType target_piece = piece_at_square[to];
			MoveType move_type = getMoveType(from, to, piece, target_piece, white);

			// Encode move (check moves only handled in endgame)
			// Promote only to queen
			uint32_t move = ChessAI::encodeMove(from, to, piece, target_piece, move_type,
				(move_type == PROMOTION || move_type == PROMOTION_CAPTURE) ? QUEEN : EMPTY, false);

			// --- Scoring Logic ---
			int score = 0; // Default score for quiet moves at depth 0 or unhandled cases

			// Check if this is the TT move hint
			if (move_hint != NULL_MOVE_32 && move == move_hint) {
				score = TT_MOVE_SCORE;
			}
			else {
				// If not TT move, score captures using MVV-LVA
				if (move_type == CAPTURE || move_type == PROMOTION_CAPTURE || move_type == EN_PASSANT) {
					PieceType victim = (move_type == EN_PASSANT) ? PAWN : target_piece;
					score = MVV_LVA[victim][piece];
				}
				else if (depth > 0) { // If non-capture, prioritize killer moves and use history heuristic (not scored for depth 0)
					// Killer move priority
					if (ChessAI::isKillerMove(from, to, piece, depth)) {
						score = KILLER_SCORE;
					}
					else {
						score = ChessAI::getHistoryScore(from, to, piece);
					}

					// Prioritize queen promotions
					if (move_type == PROMOTION || move_type == PROMOTION_CAPTURE) score += QUEEN_PROMOTION;
				}
			}

			move_scores[move_count++] = { move, score };

			Utils::popBit(legal_moves, to);
		}
		Utils::popBit(friendly_pieces, from);
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
	std::array<std::pair<uint32_t, int>, MAX_MOVES> move_scores;  // Stack-allocated array

	uint64_t friendly_pieces = white ? whitePieces() : blackPieces();
	uint64_t opponent_pieces = white ? blackPieces() : whitePieces();

	while (friendly_pieces) {
		int from = Utils::findFirstSetBit(friendly_pieces);
		PieceType piece = piece_at_square[from];
		uint64_t legal_moves = getLegalMoves(from, white);
		uint64_t captures = legal_moves & opponent_pieces;

		// Process captures first
		while (captures) {
			int to = Utils::findFirstSetBit(captures);
			PieceType target_piece = piece_at_square[to];
			MoveType move_type = getMoveType(from, to, piece, target_piece, white);

			// Score moves using MVV-LVA for captures
			int score = MVV_LVA[target_piece][piece];

			move_scores[move_count++] = { ChessAI::encodeMove(from, to, piece, target_piece, move_type,
				(move_type == PROMOTION_CAPTURE) ? QUEEN : EMPTY, false), score + (move_type == PROMOTION_CAPTURE) ? QUEEN_PROMOTION : 0};

			Utils::popBit(captures, to);
		}

		// Process en passant if applicable
		if (piece == PAWN && en_passant_target != UNASSIGNED) {
			uint64_t ep_mask = 1ULL << en_passant_target;
			if (legal_moves & ep_mask) {
				move_scores[move_count++] = { ChessAI::encodeMove(from, en_passant_target, PAWN, EMPTY, EN_PASSANT, EMPTY, false), MVV_LVA[PAWN][PAWN]};
			}
		}

		// Process quiet promotions to queen
		if (piece == PAWN) {
			uint64_t promotion_mask = (white ? RANK_8 : RANK_1) & ~(opponent_pieces);
			if ((legal_moves & promotion_mask) != 0) {
				int promotion_sq = Utils::findFirstSetBit(legal_moves & promotion_mask);
				move_scores[move_count++] = { ChessAI::encodeMove(from, promotion_sq, PAWN, EMPTY, PROMOTION, QUEEN, false), QUEEN_PROMOTION };
			}
		}

		Utils::popBit(friendly_pieces, from);
	}

	// Sort only the portion containing actual moves
	std::sort(move_scores.begin(), move_scores.begin() + move_count,
		[](const auto& a, const auto& b) { return a.second > b.second; });

	// Extract just the moves
	for (int i = 0; i < move_count; ++i) {
		move_list[i] = move_scores[i].first;
	}
}

void Bitboard::generateEndgameMoves(std::array<uint32_t, MAX_MOVES>&move_list, int& move_count, int depth, bool white, uint32_t move_hint) {
	move_count = 0;
	std::array<std::pair<uint32_t, int>, MAX_MOVES> move_scores; // Stack allocated array

	// Generate all moves directly into move_scores with scoring
	uint64_t friendly_pieces = white ? whitePieces() : blackPieces();
	uint64_t opponent_pieces = white ? blackPieces() : whitePieces();

	// Determine if we are in winning position (simplified)
	bool winning_position = white ? (evaluateBoard() >= 0) : (evaluateBoard() < 0);

	// Get squares where we can check the enemy king
	int enemy_king = Utils::findFirstSetBit(piece_bitboards[!white][KING]);
	KingDanger king_danger = Moves::computeKingDanger(enemy_king, friendly_pieces | opponent_pieces, white);

	while (friendly_pieces) {
		int from = Utils::findFirstSetBit(friendly_pieces);
		Utils::popBit(friendly_pieces, from);

		PieceType piece = piece_at_square[from];
		uint64_t legal_moves = getLegalMoves(from, white);

		while (legal_moves) {
			int to = Utils::findFirstSetBit(legal_moves);
			Utils::popBit(legal_moves, to);

			PieceType target_piece = piece_at_square[to];
			MoveType move_type = getMoveType(from, to, piece, target_piece, white);
			bool is_check = isCheckMove(king_danger, to, piece);

			// --- Scoring Logic ---
			int score = 0; // Default score for quiet moves at depth 0 or unhandled cases

			// Checks are absolute priorities, gives highest score
			if (is_check) score += CHECK_MOVE_SCORE;

			// Score captures with endgame specific MVV-LVA
			if (move_type == CAPTURE || move_type == PROMOTION_CAPTURE || move_type == EN_PASSANT) {
				PieceType victim = (move_type == EN_PASSANT) ? PAWN : target_piece;
				score += MVV_LVA_ENDGAME[victim][piece];

				// Penalize losing trades in winning positions
				if (winning_position && PIECE_VALUES[piece] > PIECE_VALUES[victim]) {
					score -= LOSING_TRADE_PENALTY;
				}
			}
			// Killer moves and history heuristics for quiet moves
			else if (depth > 0) {
				if (ChessAI::isKillerMove(from, to, piece, depth)) {
					score += (piece == PAWN) ? PAWN_KILLER_SCORE : (piece == KING) ? KING_KILLER_SCORE : ENDGAME_KILLER_SCORE;
				}
				score += ChessAI::getHistoryScore(from, to, piece) / HISTORY_SCORE_SCALEFACTOR; // History score is scaled down (prevent domination)
			}

			if (piece == PAWN && isPassedPawn(to, white)) {
				score += PASSED_PAWN_SCORE + PASSED_PAWN_RANK_MULTIPLIER * (white ? (to / 8) : (7 - to / 8)); // Passed pawn push
			}

			if (piece == KING) {
				score += 600 * (4 - CENTRALITY_DISTANCE[to]); // King activity
			}

			// --- Encode Move(s) and Apply TT Bonus ---
			if (move_type == PROMOTION || move_type == PROMOTION_CAPTURE) {
				PieceType promotions[] = { QUEEN, ROOK, BISHOP, KNIGHT }; // Order Q>N>R>B? maybe better
				int promotion_base_score = score + PROMOTION_SCORE; // Add base promotion bonus

				for (PieceType pt : promotions) {
					uint32_t move = ChessAI::encodeMove(from, to, piece, target_piece, move_type, pt, is_check);
					// Check if this specific promotion move matches the hint
					int final_score = (move_hint != NULL_MOVE_32 && move == move_hint) ? TT_MOVE_SCORE : promotion_base_score + PROMOTION_SCORES[4 - pt];
					move_scores[move_count++] = { move, final_score };
				}
			}
			else {
				uint32_t move = ChessAI::encodeMove(from, to, piece, target_piece, move_type, EMPTY, is_check);
				// Check if this regular move matches the hint
				int final_score = (move_hint != NULL_MOVE_32 && move == move_hint) ? TT_MOVE_SCORE : score;
				move_scores[move_count++] = { move, final_score };
			}
		}
	}

	// Sort only the portion containing actual moves
	std::sort(move_scores.begin(), move_scores.begin() + move_count,
		[](const auto& a, const auto& b) { return a.second > b.second; });

	// Extract just the moves
	for (int i = 0; i < move_count; ++i) {
		move_list[i] = move_scores[i].first;
	}
}

void Bitboard::generateEndgameNoisyMoves(std::array<uint32_t, MAX_MOVES>& move_list, int& move_count, bool white) {
	move_count = 0;
	std::array<std::pair<uint32_t, int>, MAX_MOVES> move_scores; // Stack allocated array

	// Generate all moves directly into move_scores with scoring
	uint64_t friendly_pieces = white ? whitePieces() : blackPieces();
	uint64_t opponent_pieces = white ? blackPieces() : whitePieces();

	// Determine if we are in winning position (simplified)
	bool winning_position = white ? (evaluateBoard() >= 0) : (evaluateBoard() < 0);

	// Get squares where we can check the enemy king
	int enemy_king = Utils::findFirstSetBit(piece_bitboards[!white][KING]);
	KingDanger king_danger = Moves::computeKingDanger(enemy_king, friendly_pieces | opponent_pieces, white);

	while (friendly_pieces) {
		int from = Utils::findFirstSetBit(friendly_pieces);
		Utils::popBit(friendly_pieces, from);

		PieceType piece = piece_at_square[from];
		uint64_t legal_moves = getLegalMoves(from, white);

		while (legal_moves) {
			int to = Utils::findFirstSetBit(legal_moves);
			Utils::popBit(legal_moves, to);

			PieceType target_piece = piece_at_square[to];
			MoveType move_type = getMoveType(from, to, piece, target_piece, white);

			// Filter: Only include captures, checks, and promotions
			bool is_check = isCheckMove(king_danger, to, piece);
			bool is_quiet = (move_type == NORMAL || move_type == CASTLING);
			if (is_quiet && !is_check) continue;

			int score = 0;

			// Checks get highest priority
			if (is_check) score += CHECK_MOVE_SCORE;

			// Promotions (queen only)
			if (move_type == PROMOTION || move_type == PROMOTION_CAPTURE) {
				score += PROMOTION_SCORE;
				// Bonus for passed pawn promotions
				if (piece == PAWN && isPassedPawn(from, white)) {
					score += PASSED_PAWN_SCORE + PASSED_PAWN_RANK_MULTIPLIER * (white ? (to / 8) : (7 - to / 8));
				}
			}

			// Captures with MVV_LVA (penalize bad trades when winning)
			if (move_type == CAPTURE || move_type == PROMOTION_CAPTURE || move_type == EN_PASSANT) {
				PieceType victim = (move_type == EN_PASSANT) ? PAWN : target_piece;
				score += MVV_LVA_ENDGAME[victim][piece];

				// Penalize losing trades in winning positions
				if (winning_position && PIECE_VALUES[piece] > PIECE_VALUES[victim]) {
					score -= LOSING_TRADE_PENALTY;
				}
			}

			// King activity - only for moves that gain opposition/centralization
			if (piece == KING) {
				// Only score king moves if they improve position
				score += 200 * (4 - CENTRALITY_DISTANCE[to]); // Centralization bonus

				// TODO
			
				//if (isOppositionGainingMove(from, to, white)) {
				//	score += 800; // Big bonus for gaining opposition
				//}
			}

			// Encode move
			if (move_type == PROMOTION || move_type == PROMOTION_CAPTURE) {
				move_scores[move_count++] = { ChessAI::encodeMove(from, to, piece, target_piece, move_type, QUEEN, is_check), score + QUEEN_PROMOTION};
				move_scores[move_count++] = { ChessAI::encodeMove(from, to, piece, target_piece, move_type, ROOK, is_check), score + ROOK_PROMOTION };
				move_scores[move_count++] = { ChessAI::encodeMove(from, to, piece, target_piece, move_type, BISHOP, is_check), score + BN_PROMOTION };
				move_scores[move_count++] = { ChessAI::encodeMove(from, to, piece, target_piece, move_type, KNIGHT, is_check), score + BN_PROMOTION };
			}
			else {
				move_scores[move_count++] = { ChessAI::encodeMove(from, to, piece, target_piece, move_type, EMPTY, is_check), score };
			}
		}
	}

	// Sort only the portion containing actual moves
	std::sort(move_scores.begin(), move_scores.begin() + move_count,
		[](const auto& a, const auto& b) { return a.second > b.second; });

	// Extract just the moves
	for (int i = 0; i < move_count; ++i) {
		move_list[i] = move_scores[i].first;
	}
}

void Bitboard::applyMoveAI(uint32_t move, bool white) {
	// Decode the move
	int source = ChessAI::from(move);
	int target = ChessAI::to(move);
	PieceType source_piece = ChessAI::piece(move);
	PieceType target_piece = ChessAI::capturedPiece(move);
	MoveType move_type = ChessAI::moveType(move);
	PieceType promotion = ChessAI::promotion(move);

	// Save state to undo stack
	UndoInfo current;
	current.castling_rights = castling_rights;
	current.en_passant_target = en_passant_target;
	current.flags = state.flags;
	current.half_moves = half_moves;
	// Board score deltas are stored after move has been applied

	// Save current state hash in history before making the move
	search_history.push_back(hash_key);

	float previous_game_phase = std::max(0.0f, std::min(1.0f, static_cast<float>(game_phase_score) / MAX_GAME_PHASE)); // Store previous phase
	int material_delta = 0; // Count material losses/gains in this move
	int positional_delta = 0; // Change of positional score with move
	int game_phase_delta = 0; // Change of game phase score

	// Clear the source square, doesn't differ for any move
	piece_bitboards[white][source_piece] &= ~(1ULL << source);
	piece_at_square[source] = EMPTY;
	hash_key ^= Tables::PIECE_KEYS[white][source_piece][source];

	// Clear positional score of source square
	positional_delta -= getPositionalScore(source, previous_game_phase, source_piece, white);

	// Precompute whether castling is affected
	bool castling_affected = (castling_rights & (white ? 0x03 : 0x0C)) != 0;
	hash_key ^= Tables::CASTLING_KEYS[castling_rights]; // Remove previous rights

	// If a rook or knight moved, update castling rights
	if (castling_affected && (source_piece == ROOK || source_piece == KING)) {
		if (source_piece == ROOK) updateRookCastling(white, source);
		else if (source_piece == KING) castling_rights &= ~(white ? 0x03 : 0x0C);  // Disable castling rights
	}
	
	// If capture, clear target square and update scores
	if (move_type == CAPTURE || move_type == PROMOTION_CAPTURE) {
		piece_bitboards[!white][target_piece] &= ~(1ULL << target);
		hash_key ^= Tables::PIECE_KEYS[!white][target_piece][target];

		// Update game phase
		if (target_piece == QUEEN) game_phase_delta -= 4;
		else if (target_piece == ROOK) {
			if ((castling_rights & (white ? 0x0C : 0x03)) != 0) updateRookCastling(!white, target); // Update castling
			game_phase_delta -= 2;
		}
		else if (target_piece == KNIGHT || target_piece == BISHOP) game_phase_delta -= 1;

		// Update material score
		material_delta += PIECE_VALUES[target_piece];

		// Clear positional score of target
		positional_delta += getPositionalScore(target, previous_game_phase, target_piece, !white);
	}

	// En passant
	if (move_type == EN_PASSANT) {
		// Compute the pawn captured by en passant
		int en_passant_square = white ? (target - 8) : (target + 8);
		piece_bitboards[!white][PAWN] &= ~(1ULL << en_passant_square); // Capture pawn
		piece_at_square[en_passant_square] = EMPTY;
		hash_key ^= Tables::PIECE_KEYS[!white][PAWN][en_passant_square];

		material_delta += PIECE_VALUES[PAWN];
		positional_delta += getPositionalScore(en_passant_square, previous_game_phase, PAWN, !white);
	}

	// Castling
	if (move_type == CASTLING) {
		handleCastling(white, target);

		// Determine if kingside or queenside castling
		bool kingside = target == 6 || target == 62;

		// Get rook origin and target
		int rook_origin = white ? (kingside ? 7 : 0) : (kingside ? 63 : 56);
		int rook_target = white ? (kingside ? 5 : 3) : (kingside ? 61 : 59);

		hash_key ^= Tables::PIECE_KEYS[white][ROOK][rook_origin];
		hash_key ^= Tables::PIECE_KEYS[white][ROOK][rook_target];

		positional_delta -= getPositionalScore(rook_origin, previous_game_phase, ROOK, white);
		positional_delta += getPositionalScore(rook_target, previous_game_phase, ROOK, white);
	}

	// Promotion
	if (move_type == PROMOTION || move_type == PROMOTION_CAPTURE) {
		// Update promoted pieces bitboard
		piece_bitboards[white][promotion] |= (1ULL << target);
		piece_at_square[target] = promotion;
		hash_key ^= Tables::PIECE_KEYS[white][promotion][target];

		// Update game phase score
		if (promotion == QUEEN) game_phase_delta += 4;
		else if (promotion == ROOK) game_phase_delta += 2;
		else if (promotion == BISHOP || promotion == KNIGHT) game_phase_delta += 1;

		material_delta += PIECE_VALUES[promotion];
		material_delta -= PIECE_VALUES[PAWN];

		positional_delta += getPositionalScore(target, previous_game_phase, promotion, white);
	}
	else { // For the non promotion moves move source piece to target
		piece_bitboards[white][source_piece] |= (1ULL << target);
		piece_at_square[target] = source_piece;
		hash_key ^= Tables::PIECE_KEYS[white][source_piece][target];
	}

	// Clear previous en passant
	if (en_passant_target != UNASSIGNED) {
		hash_key ^= Tables::EN_PASSANT_KEYS[en_passant_target % 8];
	}

	hash_key ^= Tables::CASTLING_KEYS[castling_rights]; // Set new castling rights

	// Set en passant target if a pawn double pushes
	if (move_type == PAWN_DOUBLE_PUSH) {
		en_passant_target = white ? (source + 8) : (target + 8);
		hash_key ^= Tables::EN_PASSANT_KEYS[target % 8];
	}
	else {
		en_passant_target = UNASSIGNED;
	}

	// Toggle side to move
	hash_key ^= Tables::SIDE_TO_MOVE_KEY;

	// For reversible moves increment half-moves
	// Count incremented for both
	if (!(source_piece == PAWN || move_type == CAPTURE || move_type == CASTLING)) {
		half_moves++;
	}
	else {
		half_moves = 0; // Reset half-moves if irreversible
	}

	// Apply score deltas
	if (!white) {
		material_delta = -material_delta;
		positional_delta = -positional_delta;
	}

	material_score += material_delta;
	positional_score += positional_delta;
	game_phase_score += game_phase_delta;

	// Save deltas and undo-info to stack
	current.material_delta = material_delta;
	current.positional_delta = positional_delta;
	current.game_phase_delta = game_phase_delta;
	undo_stack.push_back(current);

	// Compute new game phase (clamped 0-1 range)
	float new_game_phase = std::max(0.0f, std::min(1.0f, static_cast<float>(game_phase_score) / MAX_GAME_PHASE));

	// **Check if the phase change is large enough for a full recalculation**
	float phase_change = abs(new_game_phase - previous_game_phase);
	if (phase_change >= FULL_RECALC_THRESHOLD) {
		updatePositionalScore();
	}

	updateBoardState(white); // Update board state after applied move (+promoted)

	ply_count++; 
}

void Bitboard::undoMoveAI(uint32_t move, bool white) {
	// Decode the move
	int source = ChessAI::from(move);
	int target = ChessAI::to(move);
	PieceType source_piece = ChessAI::piece(move);
	PieceType target_piece = ChessAI::capturedPiece(move);
	MoveType move_type = ChessAI::moveType(move);
	PieceType promotion = ChessAI::promotion(move);

	// --- Undo Board and Hash Modifications (Reverse order of applyMove) ---
	search_history.pop_back(); // Pop the history stack

	hash_key ^= Tables::SIDE_TO_MOVE_KEY; // Toggle side to move

	if (move_type == PAWN_DOUBLE_PUSH) {
		hash_key ^= Tables::EN_PASSANT_KEYS[target % 8];
	}
	hash_key ^= Tables::CASTLING_KEYS[castling_rights];

	// Restore board state
	const UndoInfo& prev = undo_stack.back();
	castling_rights = prev.castling_rights;
	en_passant_target = prev.en_passant_target;
	state.flags = prev.flags;
	material_score -= prev.material_delta;
	positional_score -= prev.positional_delta;
	game_phase_score -= prev.game_phase_delta;
	half_moves = prev.half_moves;
	undo_stack.pop_back(); // Pop the undo stack

	// Apply restored castling rights and en passant
	if (en_passant_target != UNASSIGNED) {
		hash_key ^= Tables::EN_PASSANT_KEYS[en_passant_target % 8];
	}
	hash_key ^= Tables::CASTLING_KEYS[prev.castling_rights];

	// Move source piece back to source square
	// Doesn't differ for any move type
	piece_bitboards[white][source_piece] |= 1ULL << source; // Move to original position
	hash_key ^= Tables::PIECE_KEYS[white][source_piece][source];

	piece_at_square[source] = source_piece; // Restore piece type
	piece_at_square[target] = target_piece; // Restore target

	// Handle special cases

	// Restore captured piece if move was a capture
	if (move_type == CAPTURE || move_type == PROMOTION_CAPTURE) {
		piece_bitboards[!white][target_piece] |= 1ULL << target; // Restore captured piece
		hash_key ^= Tables::PIECE_KEYS[!white][target_piece][target];
	}

	// Restore en passant pawn if move was en passant
	if (move_type == EN_PASSANT) {
		// Determine en passant square
		int en_passant_square = white ? (target - 8) : (target + 8);
		piece_bitboards[!white][PAWN] |= 1ULL << en_passant_square; // Restore captured pawn
		piece_at_square[en_passant_square] = PAWN; // Also restore piece type
		hash_key ^= Tables::PIECE_KEYS[!white][PAWN][en_passant_square];
	}

	// Restore rook to original position if move was castling
	if (move_type == CASTLING) {
		// Determine if kingside or queenside castling
		bool kingside = target == 6 || target == 62;

		undoCastling(white, kingside);

		// Get rook origin and target
		int rook_origin = white ? (kingside ? 7 : 0) : (kingside ? 63 : 56);
		int rook_target = white ? (kingside ? 5 : 3) : (kingside ? 61 : 59);

		hash_key ^= Tables::PIECE_KEYS[white][ROOK][rook_target];
		hash_key ^= Tables::PIECE_KEYS[white][ROOK][rook_origin];
	}

	// Restore promotion piece if move was promotion
	if (move_type == PROMOTION || move_type == PROMOTION_CAPTURE) {
		piece_bitboards[white][promotion] &= ~(1ULL << target); // Clear promotion square
		hash_key ^= Tables::PIECE_KEYS[white][promotion][target];
	}
	else { // Recover source piece, applies to non promotions
		piece_bitboards[white][source_piece] &= ~(1ULL << target);
		hash_key ^= Tables::PIECE_KEYS[white][source_piece][target];
	}

	ply_count--;
}

int Bitboard::evaluateBoard() {
	// Return the total score
	return material_score + positional_score;
}

int Bitboard::evaluateKingSafety() {
	int white_king = Utils::findFirstSetBit(piece_bitboards[WHITE][KING]);
	int black_king = Utils::findFirstSetBit(piece_bitboards[BLACK][KING]);

	int white_penalty = evaluateSingleKingSafety(white_king, true);
	int black_penalty = evaluateSingleKingSafety(black_king, false);

	// Return penalty diff
	return white_penalty - black_penalty;
}

bool Bitboard::isGameOver() {
	return state.isCheckmateWhite() || state.isCheckmateBlack() || state.isDraw();
}

bool Bitboard::isDrawByRepetition() {
	// We use the current hash key 
	// Current half moves are how many reversible plies back we need to check
	int count = 0;

	// Iterate backwards through the history stack, starting from the parent state.
	// Check only as far back as the plies since the last irreversible move allows.
	int current_search_depth = static_cast<int>(search_history.size());
	for (int i = 1; i <= half_moves && (current_search_depth - i >= 0); ++i) {
		int history_index = current_search_depth - i;
		if (search_history[history_index] == hash_key) {
			count++;
			// If we found the same position twice previously in the relevant history,
			// the current position is the 3rd occurrence.
			if (count >= 2) {
				return true;
			}
		}
	}
	return false;
}

MoveType Bitboard::getMoveType(int source_square, int target_square, PieceType piece, PieceType target_piece, bool white) const {
	// Determine move type
	if (piece == PAWN) {
		if (target_square == en_passant_target) return EN_PASSANT;
		if ((white && target_square >= 56) || (!white && target_square <= 7)) {
			return (target_piece == EMPTY) ? PROMOTION : PROMOTION_CAPTURE;
		}
		if (abs(source_square - target_square) == 16) return PAWN_DOUBLE_PUSH;
	}
	if (piece == KING && abs(source_square - target_square) == 2) return CASTLING;
	if (target_piece != EMPTY) return CAPTURE;
	return NORMAL;
}

void Bitboard::undoCastling(bool white, bool kingside) {
	if (white) {
		if (kingside) {
			piece_bitboards[WHITE][ROOK] &= ~ROOK_F1; // Remove rook from f1
			piece_bitboards[WHITE][ROOK] |= ROOK_H1; // Move rook to h1

			piece_at_square[5] = EMPTY;
			piece_at_square[7] = ROOK;
		}
		else {
			piece_bitboards[WHITE][ROOK] &= ~ROOK_D1; // Remove rook from d1
			piece_bitboards[WHITE][ROOK] |= ROOK_A1; // Move rook to a1

			piece_at_square[3] = EMPTY;
			piece_at_square[0] = ROOK;
		}
	}
	else {
		if (kingside) {
			piece_bitboards[BLACK][ROOK] &= ~ROOK_F8; // Remove rook from f8
			piece_bitboards[BLACK][ROOK] |= ROOK_H8; // Move rook to h8

			piece_at_square[61] = EMPTY;
			piece_at_square[63] = ROOK;
		}
		else {
			piece_bitboards[BLACK][ROOK] &= ~ROOK_D8; // Remove rook from d8
			piece_bitboards[BLACK][ROOK] |= ROOK_A8; // Move rook to a8

			piece_at_square[59] = EMPTY;
			piece_at_square[56] = ROOK;
		}
	}
}

inline int Bitboard::getPositionalScore(int square, float game_phase, PieceType piece, bool white) {
	return static_cast<int>(
		game_phase * PIECE_TABLE_MID[piece][Utils::getRow(square, white)][Utils::getCol(square, white)]
		+ (1.0f - game_phase) * PIECE_TABLE_END[piece][Utils::getRow(square, white)][Utils::getCol(square, white)]
	);
}

bool Bitboard::isPassedPawn(int pawn, bool white) {
	// Early exit if pawn is already on promotion rank
	if (white ? (pawn >= 48) : (pawn <= 16)) return true;

	// Get pawn promotion path mask
	// Pawn promotion path includes the pawn file itself and adjacent (left/right) files
	int file = pawn % 8;
	int final_sq = (white ? 48 : 8) + file; // The final square in pawns file where enemy pawn cannot be (e.g., 30 -> 54)

	uint64_t promotion_path = Tables::BETWEEN[pawn][final_sq] | (1ULL << final_sq); // Main file path

	promotion_path |= (file > 0) ? Tables::BETWEEN[white ? (pawn + 7) : (pawn - 9)][final_sq - 1] | (1ULL << (final_sq - 1)) : 0;
	promotion_path |= (file < 7) ? Tables::BETWEEN[white ? (pawn + 9) : (pawn - 7)][final_sq + 1] | (1ULL << (final_sq + 1)) : 0;

	// Get enemy pawns and check if any line up with our mask
	uint64_t enemy_pawns = white ? piece_bitboards[BLACK][PAWN] : piece_bitboards[WHITE][PAWN];
	return (enemy_pawns & promotion_path) == 0; // No enemy pawns in mask
}

bool Bitboard::isCheckMove(const KingDanger& king_danger, int to, PieceType piece) {
	if (piece == KING) return false; // King cannot check
	if (piece == PAWN) return (king_danger.pawn & (1ULL << to)) != 0;
	if (piece == KNIGHT) return (king_danger.knight & (1ULL << to)) != 0;
	if (piece == BISHOP) return (king_danger.diagonal & (1ULL << to)) != 0;
	if (piece == ROOK) return (king_danger.orthogonal & (1ULL << to)) != 0;
	return ((king_danger.orthogonal | king_danger.diagonal) & (1ULL << to)) != 0; // Queen
}

int Bitboard::evaluateSingleKingSafety(int king_sq, bool white) {
	int penalty = 0;

	uint64_t friendly_pawns = piece_bitboards[white][PAWN];
	uint64_t enemy_pawns = piece_bitboards[!white][PAWN];

	// Define kings file and adjacent files
	int king_file = king_sq % 8;
	uint64_t file_mask = Utils::getFile(king_sq);
	if (king_file > 0) file_mask |= Utils::getFile(king_sq - 1);
	if (king_file < 7) file_mask |= Utils::getFile(king_sq + 1);

	// Open file penalty
	if (!(file_mask & friendly_pawns)) { // File is open or semi-open for the enemy
		int file_mask_penalty = OPEN_FILE_PENALTY; // Set penalty base
		// Multiply penalty by heavy piece factor if present
		if ((file_mask & (piece_bitboards[!white][QUEEN] | piece_bitboards[!white][ROOK])) != 0) {
			file_mask_penalty *= HEAVY_PIECE_MULTIPLIER;
		}
		// If fully open, slightly higher penalty
		if (!(file_mask & enemy_pawns)) {
			file_mask_penalty += OPEN_FILE_PENALTY / 2;
		}
		penalty += file_mask_penalty; // Apply file penalty
	}

	int shield_penalty = 0;
	// Penalty for missing pawn shields
	// Check for ranks 4 or below (absolute), else open file penalty for aggressive advancing
	int king_rank = white ? (king_sq / 8) : 7 - (king_sq / 8);
	if (king_rank <= 3) {
		int front_sq = white ? (king_sq + 8) : (king_sq - 8);
		if (!(friendly_pawns & (1ULL << front_sq))) shield_penalty += PAWN_SHIELD_PENALTY; // Front square
		if (king_sq % 8 > 0) {
			if (!(friendly_pawns & (1ULL << (front_sq - 1)))) shield_penalty += PAWN_SHIELD_PENALTY; // Front-left square
		}
		if (king_sq % 8 < 7) {
			if (!(friendly_pawns & (1ULL << (front_sq + 1)))) shield_penalty += PAWN_SHIELD_PENALTY; // Front-right square
		}
	}
	else {
		shield_penalty += OPEN_FILE_PENALTY;
	}

	// Penalize pawn storms (enemy pawns near the king)
	uint64_t storm_zone = MoveTables::KING_MOVES->moves;
	shield_penalty += Utils::countSetBits(enemy_pawns & storm_zone) * PAWN_STORM_PENALTY;

	penalty += shield_penalty; // Apply shield penalty

	return penalty;
}

int Bitboard::estimateCaptureValue(uint32_t move) {
	// Get pieces involved
	PieceType captured_piece = ChessAI::capturedPiece(move);
	PieceType attacking_piece = ChessAI::piece(move);

	// Value of captured piece
	int capture_value = PIECE_VALUES[captured_piece];

	// If we're losing the more valuable piece (bad trade)
	int trade_delta = PIECE_VALUES[attacking_piece] - PIECE_VALUES[captured_piece];

	// Return net gain (could be negative for bad trades)
	return capture_value - (trade_delta > 0 ? PIECE_VALUES[attacking_piece] : 0);
}

int Bitboard::estimateEndgameCaptureValue(uint32_t move, bool white) {
	int to_sq = ChessAI::to(move);

	// Get pieces involved
	PieceType captured_piece = ChessAI::capturedPiece(move);
	PieceType attacking_piece = ChessAI::piece(move);

	// Value of captured piece
	int capture_value = PIECE_VALUES[captured_piece];

	// Bonus for capturing passed pawns
	if (captured_piece == PAWN && isPassedPawn(to_sq, !white)) {
		int rank = white ? (to_sq / 8) : (7 - (to_sq / 8)); // Relative rank (0=start, 7=promotion
		capture_value += (10 + (rank * rank) * 5); // Quadratic scaling
	}

	// If we're losing the more valuable piece (bad trade)
	int trade_delta = PIECE_VALUES[attacking_piece] - PIECE_VALUES[captured_piece];

	// Return net gain (could be negative for bad trades)
	return capture_value - (trade_delta > 0 ? PIECE_VALUES[attacking_piece] : 0);
}

int Bitboard::calculateKingDistance() {
	int white_king_sq = Utils::findFirstSetBit(piece_bitboards[WHITE][KING]);
	int black_king_sq = Utils::findFirstSetBit(piece_bitboards[BLACK][KING]);
	return Utils::calculateDistance(white_king_sq, black_king_sq);
}

int Bitboard::getKingCentralization() {
	int white_king_sq = Utils::findFirstSetBit(piece_bitboards[WHITE][KING]);
	int black_king_sq = Utils::findFirstSetBit(piece_bitboards[BLACK][KING]);

	// Calculate both kings distance from center and get diff
	// If white is closer to center than black -> positive (good for white), and reversed for black
	return CENTRALITY_DISTANCE[black_king_sq] - CENTRALITY_DISTANCE[white_king_sq];
}

int Bitboard::evaluatePassedPawns(bool white) {
	uint64_t pawns = piece_bitboards[white][PAWN]; // Get friendly pawns
	int score = 0;

	while (pawns) {
		int pawn_sq = Utils::findFirstSetBit(pawns);
		Utils::popBit(pawns, pawn_sq);

		if (!isPassedPawn(pawn_sq, white)) continue; // Skip non-passed pawns

		int rank = white ? (pawn_sq / 8) : (7 - (pawn_sq / 8)); // Relative rank (0=start, 7=promotion
		score += (10 + (rank * rank) * 5); // Quadratic scaling

		// Bonus if supported by friendly king
		int king_sq = Utils::findFirstSetBit(piece_bitboards[white][KING]);
		int king_dist = Utils::calculateDistance(pawn_sq, king_sq);
		score += (7 - king_dist) * 10; // Closer king = better

		// Penalty if blocked by enemy king
		int enemy_king_sq = Utils::findFirstSetBit(piece_bitboards[!white][KING]);
		int enemy_king_dist = Utils::calculateDistance(pawn_sq, enemy_king_sq);
		if (enemy_king_dist <= 2) score -= 100; // Enemy king can intercept
		if (king_dist < enemy_king_dist) score += 50; // King is closer than opponent (protecting passed pawn)
	}
	return score;
}