#ifndef MOVES_H
#define MOVES_H
class Moves {
public:
    // Moves created with standard movetables
    uint64_t getPawnMoves(int pawn, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white, int en_passant);
    uint64_t getPawnCaptures(int pawn, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white, int en_passant); // Used for attack squares
    uint64_t getKnightMoves(int knight, const uint64_t& white_pieces, const uint64_t& black_pieces);
    uint64_t getKingMoves(int king, uint64_t white_pieces, uint64_t black_pieces, bool white);

    // Moves created with magic movetables
};

#endif
