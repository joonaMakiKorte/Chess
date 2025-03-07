#include "pch.h"
#include "ChessBoard.h"

ChessBoard::ChessBoard() {
    board = Board();
}

bool ChessBoard::ValidateMove(const char* move) {
    // Validate move notation
    if (strlen(move) != 4) {
        return false;
    }

    // Extract source and target squares
    char source_square[3] = { move[0], move[1], '\0' }; // First 2 chars + null terminator
    char target_square[3] = { move[2], move[3], '\0' }; // Next 2 chars + null terminator

    // Get bitboard represantations of moves
    uint64_t source_bitb = SquareToBitboard(source_square);
    uint64_t target_bitb = SquareToBitboard(target_square);

    // Get all legal moves from the source square
    uint64_t legal_moves = board.getLegalMoves(source_bitb);

    // Bitwise AND operation to check if target is in legal moves
    // Returns bool indicating if result is non-zero, meaning target exists in moves
    return (target_bitb & legal_moves) != 0;
}

std::string ChessBoard::GetBoardState() {
	// Generate the FEN string based on the current board state
	std::string fen;

    // 1. Piece Placement
    for (int rank = 7; rank >= 0; rank--) {
        int empty_squares = 0;
        for (int file = 0; file < 8; file++) {
            uint64_t square = 1ULL << (rank * 8 + file); // Get current square as bitboard

            // Get piece type at square
            char piece = board.getPieceType(square);

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
    std::string active_color = board.isWhite() ? " w " : " b ";
    fen += active_color;

    // 3. Castling Rights (assume all castling rights available for simplicity)
    fen += board.getCastlingRightsString();

    // 4. En Passant Target Square
    std::string enPassant = board.getEnPassantString();
    fen += " " + (enPassant.empty() ? "-" : enPassant);

    // 5. Half-Move Clock
    fen += " " + std::to_string(board.getHalfMoveClock());

    // 6. Full-Move Number
    fen += " " + std::to_string(board.getFullMoveNumber());

    return fen;
}

uint64_t ChessBoard::SquareToBitboard(const char* square) {
    // Validate square notation
    if (strlen(square) != 2) {
        throw std::invalid_argument("Invalid square notation");
    }

    // Extract file and rank from the square string
    // File=col, rank=row
    char file_char = square[0]; // File (a-h)
    char rank_char = square[1]; // Rank (1-8)

    // Convert file to 0-based index (a=0, b=1, ..., h=7)
    int file = tolower(file_char) - 'a';
    if (file < 0 || file > 7) {
        throw std::invalid_argument("Invalid file in square notation");
    }

    // Convert rank to 0-based index (1=0, 2=1, ..., 8=7)
    int rank = rank_char - '1';
    if (rank < 0 || rank > 7) {
        throw std::invalid_argument("Invalid rank in square notation");
    }

    // Calculate the square index (0-63)
    int square_index = (7 - rank) * 8 + file;

    // Create a bitboard with only this square set
    return 1ULL << square_index;
}
