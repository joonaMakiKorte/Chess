#include "pch.h"
#include "ChessBoard.h"

ChessBoard::ChessBoard() 
    : board(std::make_unique<Bitboard>()), debugMessage("Initial debug message")
{}

uint64_t ChessBoard::LegalMoves(int square) {
    // Validate move notation
    if (!(0 <= square <= 63)) return 0ULL;

    // Get all legal moves from the source square
    return board->getLegalMoves(square);
}


bool ChessBoard::isInCheck() {

    // Get all legal moves from the source square
    return board->isInCheck();
}

bool ChessBoard::isCheckmate() {

    // Get all legal moves from the source square
    return board->isCheckmate();
}


void ChessBoard::MovePiece(int source, int target) {
    // Validate move notations
    if (!(0 <= source <= 63) || !(0 <= target <= 63)) return;

    // Apply move in bitboard
    board->applyMove(source, target);
    board->switchTurn(); 
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
            char piece = board->getPieceType(square);

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
    std::string active_color = board->isWhite() ? " w " : " b ";
    fen += active_color;

    // 3. Castling Rights (assume all castling rights available for simplicity)
    fen += board->getCastlingRightsString();

    // 4. En Passant Target Square
    fen += " " + board->getEnPassantString();

    // 5. Half-Move Clock
    fen += " " + std::to_string(board->getHalfMoveClock());

    // 6. Full-Move Number
    fen += " " + std::to_string(board->getFullMoveNumber());

    return fen;
}



void ChessBoard::UpdateDebugMessage(const std::string& message) {
    debugMessage = message;
}

std::string ChessBoard::GetDebugMessage() const {
    return debugMessage;
}
