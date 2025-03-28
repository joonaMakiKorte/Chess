#ifndef MOVES_H
#define MOVES_H

#include "BitboardConstants.h"

class Moves {
public:
    // Get pseudo legal moves depending on the board state
    // Doesn't take the game state (check) or pinned pieces into factor, filtered later
    // Pawns are handled separately
    static uint64_t getPseudoLegalMoves(int square, PieceType piece, uint64_t occupied);

    // Moves created with standard movetables
    static uint64_t getPawnMoves(int pawn, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white, int en_passant);
    static uint64_t getPawnCaptures(int pawn, bool white); // Used for attack squares
    static uint64_t getKnightMoves(int knight);
    static uint64_t getKingMoves(int king);

    // Moves created with magic movetables
    static uint64_t getBishopMoves(int bishop, uint64_t occ);
    static uint64_t getRookMoves(int rook, uint64_t occ);
    static uint64_t getQueenMoves(int queen, uint64_t occupied);
};

#endif
