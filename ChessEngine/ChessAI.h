#ifndef CHESSAI_H
#define CHESSAI_h

#include "Bitboard.h"
/*
The ChessAI structures a move in 32 bits:

0000 0000 0000 0000 0000 0000 0011 1111  -> from (6 bits)
0000 0000 0000 0000 0000 1111 1100 0000  -> to (6 bits)
0000 0000 0000 0000 1111 0000 0000 0000  -> piece type (4 bits)
0000 0000 0000 1111 0000 0000 0000 0000  -> captured piece type (4 bits)
0000 0000 1111 0000 0000 0000 0000 0000  -> move type (4 bits)
0000 1111 0000 0000 0000 0000 0000 0000  -> promotion type (4 bits)
0001 0000 0000 0000 0000 0000 0000 0000  -> special flags (1 bit for en passant)

*/
class ChessAI {
public:
    // Each piece is assigned a unique integer (4 bits)
    enum PieceType : uint8_t {
        EMPTY = 0,   // No piece
        PAWN = 1,
        KNIGHT = 2,
        BISHOP = 3,
        ROOK = 4,
        QUEEN = 5,
        KING = 6
    };

    // Defines the type of move (4 bits)
    enum MoveType : uint8_t {
        NORMAL = 0,        // Standard move
        CAPTURE = 1,       // Capturing a piece
        CASTLING = 2,      // Castling (O-O, O-O-O)
        EN_PASSANT = 3,    // En passant capture
        PROMOTION = 4,     // Pawn promotion
        PROMOTION_CAPTURE = 5  // Pawn promotion with capture
    };

    // Pack a move into an integer
    uint32_t encodeMove(int from, int to, PieceType piece, PieceType captured, MoveType type, PieceType promotion, bool enPassant) {
        return (from & 0x3F) |
            ((to & 0x3F) << 6) |
            ((piece & 0xF) << 12) |
            ((captured & 0xF) << 16) |
            ((type & 0xF) << 20) |
            ((promotion & 0xF) << 24) |
            ((enPassant ? 1 : 0) << 28);
    }

private:
    // Extract data from a packed move
    int from(uint32_t move) { return move & 0x3F; }
    int to(uint32_t move) { return (move >> 6) & 0x3F; }
    PieceType piece(uint32_t move) { return static_cast<PieceType>((move >> 12) & 0xF); }
    PieceType capturedPiece(uint32_t move) { return static_cast<PieceType>((move >> 16) & 0xF); }
    MoveType moveType(uint32_t move) { return static_cast<MoveType>((move >> 20) & 0xF); }
    PieceType promotion(uint32_t move) { return static_cast<PieceType>((move >> 24) & 0xF); }
    bool isEnPassant(uint32_t move) { return (move >> 28) & 1; }
};

#endif // !CHESSAI_H
