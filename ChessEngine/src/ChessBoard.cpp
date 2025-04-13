#include "pch.h"
#include "ChessBoard.hpp"
#include "ChessAI.hpp"

ChessBoard::ChessBoard() :
    board(Bitboard()),
    white(true), // White starts
    full_moves(1), // Starts at 1
    isEndgame(false),
    previous_move("")
{}

uint64_t ChessBoard::LegalMoves(int square) {
    // Validate move notation
    if (square < 0 || square > 63) return 0ULL;

    // Get all legal moves from the source square
    return board.getLegalMoves(square, white);
}

void ChessBoard::MovePiece(int source, int target, char promotion_char) {
    // Validate move notations
    if (source < 0 || source > 63 || target < 0 || target > 63) return;

    // Check for promotion
    PieceType promotion;;
    switch (promotion_char) {
    case 'q': promotion = QUEEN; break;
    case 'r': promotion = ROOK; break;
    case 'b': promotion = BISHOP; break;
    case 'n': promotion = KNIGHT; break;
    default: promotion = EMPTY; break;
    }

    // Apply move in bitboard and get the applied move in encoded form
    uint32_t move = board.applyMove(source, target, promotion, white);

    // If black moved increment full moves
    if (!white) full_moves++;

    // Transform move to algebraic notation
    std::string move_notation = getMoveNotation(move);
    UpdatePrevMove(move_notation);

    white = !white; // Switch turn

    // Check if triggered endgame
    if (isEndgame) return; // Skip if already endgame
    isEndgame = board.isEndgame();
}

void ChessBoard::MakeMoveAI(int depth, bool maximizing) {
    uint32_t best_move;
    std::string message = "";
	if (isEndgame) {
		best_move = ChessAI::getBestEndgameMove(board, depth, message, maximizing);
	}
	else {
		best_move = ChessAI::getBestMove(board, depth, message, maximizing);
	}

	if (best_move == 0) {
		UpdatePrevMove("");
		return;
	}

    // Apply move
	board.applyMoveAI(best_move, maximizing);
    full_moves++; // Increment full moves

    board.updateDrawByRepetition(); // Check if resulted in draw by repetition

    // Transform move to algebraic notation
    std::string move_notation = getMoveNotation(best_move);
    UpdatePrevMove(move_notation);

    white = !white; // Switch turn

    // Check if triggered endgame
    if (isEndgame) return; // Skip if already endgame
    isEndgame = board.isEndgame();
}

std::string ChessBoard::GetFEN() {
    // Generate the FEN string based on the current board state
    std::string fen;

    // 1. Piece Placement
    for (int rank = 7; rank >= 0; rank--) {
        int empty_squares = 0;
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file; // Get current square as bitboard

            // Get piece type at square
            char piece = board.getPieceTypeChar(square);

            // If empty continue
            if (piece == '\0') {
                empty_squares++;
                continue;
            }

            // Add empty squares count if any
            if (empty_squares > 0) {
                fen += std::to_string(empty_squares);
                empty_squares = 0;
            }

            // Add the piece
            fen += piece;
        }

        // Add empty squares count at the end of the rank
        if (empty_squares > 0) {
            fen += std::to_string(empty_squares);
        }

        // Add a slash between ranks (except the last one)
        if (rank > 0) {
            fen += '/';
        }
    }

    // 2. Active Color
    std::string active_color = white ? " w " : " b ";
    fen += active_color;

    // 3. Castling Rights (assume all castling rights available for simplicity)
    fen += board.getCastlingRightsString();

    // 4. En Passant Target Square
    fen += " " + board.getEnPassantString();

    // 5. Half-Move Clock
    fen += " " + std::to_string(board.getHalfMoveClock());

    // 6. Full-Move Number
    fen += " " + std::to_string(full_moves);

    return fen;
}



void ChessBoard::UpdatePrevMove(const std::string& message) {
    previous_move = message;
}

std::string ChessBoard::printBitboardAsSquares(uint64_t bitboard) const {
    std::string squares;
    for (int square = 0; square < 64; square++) {
        if (bitboard & (1ULL << square)) { // Check if the bit at `square` is set
            char file = 'a' + (square % 8);   // Convert column (file)
            char rank = '1' + (square / 8);   // Convert row (rank)
            squares += std::string(1, file) + rank + " ";
        }
    }
    return squares;
}

char ChessBoard::getPieceLetter(PieceType piece) const {
    switch (piece)
    {
    case KNIGHT: return 'N';
    case BISHOP: return 'B';
    case ROOK: return 'R';
    case QUEEN: return 'Q';
    case KING: return 'K';
    default: return '-'; // Should never reach here
    }
}

char ChessBoard::getFileLetter(int square) const {
    return 'a' + (square % 8);
}

std::string ChessBoard::getMoveNotation(uint32_t move) const {
    // Define move type
    MoveType move_type = ChessAI::moveType(move);
    int from = ChessAI::from(move);
    int to = ChessAI::to(move);

    std::string algebraic_move = "";

    // Start with castling since differs from usual notation
    if (move_type == CASTLING) {
        // Determine castling side
        if (to - from == 2) algebraic_move += "O-O"; // kingside
        else if (to - from == -2) algebraic_move += "O-O-O"; // queenside
    }
    else {
        PieceType piece = ChessAI::piece(move);
        // If not pawn, add piece letter
        if (piece != PAWN) algebraic_move += getPieceLetter(piece);
        
        // Capture
        if (move_type == CAPTURE || move_type == EN_PASSANT) {
            // When a pawn captures we add its origin file
            if (piece == PAWN) algebraic_move += getFileLetter(from);
            algebraic_move += 'x'; // Captures are marked with x
        }

        // Add target square
        algebraic_move += board.squareToString(to);

        // Promotion
        if (move_type == PROMOTION) algebraic_move += getPieceLetter(ChessAI::promotion(move));

        // En passant
        if (move_type == EN_PASSANT) algebraic_move += " e.p.";
    }

    // Handle check and mate marking
    if (board.state.isCheckmateWhite() || board.state.isCheckmateBlack()) algebraic_move += "#";
    else if (board.state.isCheckWhite() || board.state.isCheckBlack()) algebraic_move += "+";

    return algebraic_move;
}

std::string ChessBoard::GetGameState() {
    if (board.state.isCheckmateWhite() || board.state.isCheckmateBlack()) return "mate";
    else if (board.state.isCheckWhite() || board.state.isCheckBlack()) return "check";
    else if (board.state.isStalemate()) return "stalemate";
    else if (board.state.isDrawRepetition()) return "draw_repetition";
    else if (board.state.isDraw50()) return "draw_50";
    // else if (board.state.isDrawInsufficient()) return "draw_insufficient";
    return "ongoing"; // Normal game state
}

std::string ChessBoard::GetPrevMove() const {
    return previous_move;
}
