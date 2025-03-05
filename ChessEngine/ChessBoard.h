#ifndef CHESSBOARD_H
#define CHESSBOARD_H	

#include "pch.h"

class ChessBoard {
private:
    // Struct type to store state of chessboard
    struct Board {
        // Represent each piece type as a bitboard
        uint64_t white_pawns;
        uint64_t black_pawns;
        uint64_t white_knights;
        uint64_t black_knights;
        uint64_t white_bishops;
        uint64_t black_bishops;
        uint64_t white_rooks;
        uint64_t black_rooks;
        uint64_t white_queen;
        uint64_t black_queen;
        uint64_t white_king;
        uint64_t black_king;

        // Get locations of white or black pieces (bitboard)
        // Uses bitwise OR operation to combine occupancy of all pieces of same color
        const uint64_t white_pieces() const {
            return white_pawns | white_rooks | white_knights |
                white_bishops | white_queen | white_king;
        }
        const uint64_t black_pieces() const {
            return black_pawns | black_rooks | black_knights |
                black_bishops | black_queen | black_king;
        }

        // Get all occupied squares (bitboard)
        // Uses bitwise OR operation to combine occupancy of all white and black pieces
        const uint64_t occupied() const {
            return white_pieces() | black_pieces();
        }

        // Initialize each piece with starting pos
        Board();
    };

    // Store the board
    Board chess_board;

public:
    // Initialize the chessboard
    ChessBoard();
};


#endif // CHESSBOARD_H