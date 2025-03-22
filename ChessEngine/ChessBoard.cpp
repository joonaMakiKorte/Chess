#include "pch.h"
#include "ChessBoard.h"
#include "ChessAI.h"

ChessBoard::ChessBoard() : 
    board(Bitboard()),
    white(true), // White starts
    debugMessage("Initial debug message")
{}

uint64_t ChessBoard::LegalMoves(int square) {
    // Validate move notation
    if (square < 0 || square > 63) return 0ULL;

    // Get all legal moves from the source square
    return board.getLegalMoves(square, white);
}

void ChessBoard::MovePiece(int source, int target) {
    // Validate move notations
    if (source < 0 || source > 63 || target < 0 || target > 63) return;

    // Apply move in bitboard
    board.applyMove(source, target, white);
    white = !white; // Switch turn
}

void ChessBoard::MakeMoveAI(int depth) {
    // Get best move in encoded form
    uint32_t best_move = ChessAI::getBestMove(board, depth);

	if (best_move == 0) {
		UpdateDebugMessage("No legal moves available");
		return;
	}

    // Apply move
	board.applyMoveAI(best_move, white);
	white = !white; // Switch turn

}

void ChessBoard::MakePromotion(int target, char promotion) {
	// Validate promotion piece
	if (promotion != 'q' && promotion != 'r' && promotion != 'b' && promotion != 'n') {
		UpdateDebugMessage("Invalid promotion piece");
		return;
	}
	// Apply promotion
	// Piece has already been moved and turn has changed,
	// so we negate the turn to apply the promotion to correct side
	board.applyPromotion(target, promotion, !white);
}

std::string ChessBoard::GetBoardState() {
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
    fen += " " + std::to_string(board.getFullMoveNumber());

    // 7. Game state (Check/Checkmate/Stalemate/No threat)
    fen += " " + board.getGameState(white);

    return fen;
}



void ChessBoard::UpdateDebugMessage(const std::string& message) {
    debugMessage = message;
}

std::string ChessBoard::printBitboardAsSquares(uint64_t bitboard) {
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

std::string ChessBoard::GetDebugMessage() const {
    return debugMessage;
}
