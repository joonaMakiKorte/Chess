#ifndef MOVES_H
#define MOVES_H

#include "BitboardConstants.h"
#include "CustomTypes.h"
#include "Bitboard.h"

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

    // Compute pinned pieces 
    // Fills into pin_data reference param
    static void computePinnedPieces(PinData& pin_data, const int& king_sq, const uint64_t& occupied,
        const uint64_t& bishops, const uint64_t& rooks, const uint64_t& queen);

    // Compute squares where enemy can check the king
    // 
    static void computeKingDanger(KingDanger& king_danger, const int& king_sq, uint64_t occupied, bool white);
};

#endif
