#ifndef MOVES_H
#define MOVES_H

#include "BitboardConstants.h"

class Moves {
public:
    // Get pseudo legal moves depending on the board state
    // Doesn't take the game state (check) or pinned pieces into factor, filtered later
    // Takes the square moved from and piece type as param + board occupancy and turn (en passant for pawn moves)
    static uint64_t getPseudoLegalMoves(int square, PieceType piece, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white, int en_passant);

    // Moves created with standard movetables
    static uint64_t getPawnMoves(int pawn, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white, int en_passant);
    static uint64_t getPawnCaptures(int pawn, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white, int en_passant); // Used for attack squares
    static uint64_t getKnightMoves(int knight, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white);
    static uint64_t getKingMoves(int king, uint64_t white_pieces, uint64_t black_pieces, bool white);

    // Moves created with magic movetables
    static uint64_t getBishopMoves(int bishop, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white);
    static uint64_t getRookMoves(int rook, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white);
    static uint64_t getQueenMoves(int queen, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white);
};

#endif
