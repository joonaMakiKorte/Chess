#ifndef MAGIC_H
#define MAGIC_H

// Magic movetable definition for rooks and bishops (+queen)
struct MagicMoves {
    uint64_t mask; // Bit-mask for moves
    uint64_t magic; // Magic moves
    int shift; // Bit shifts
};

// Declare tables
extern MagicMoves BISHOP_MOVES[64];
extern MagicMoves ROOK_MOVES[64];

void generateMagicTables();

void initMagicTables();

#endif