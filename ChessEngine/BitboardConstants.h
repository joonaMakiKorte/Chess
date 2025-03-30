#ifndef BITBOARD_CONSTANTS_H
#define BITBOARD_CONSTANTS_H

#include <cstdint>

// File masks
constexpr uint64_t FILE_A = 0x0101010101010101ULL;
constexpr uint64_t FILE_B = 0x0202020202020202ULL;
constexpr uint64_t FILE_G = 0x4040404040404040ULL;
constexpr uint64_t FILE_H = 0x8080808080808080ULL;

// Rank masks
constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
constexpr uint64_t RANK_2 = 0x000000000000FF00ULL;
constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;
constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;

// Pre-computed variables
constexpr int INF = 2147483647; // Infinity

constexpr int UNASSIGNED = -1; // Sentinel value for unassigned variables

constexpr int MAX_MOVES = 256; // Max number of legal moves for a player turn
// Absolute theoretical maximum would be 218 but we use 256 (a power of 2) for efficient memory alignment

constexpr int MAX_GAME_PHASE = 24; // Maximum game phase (total number of pieces on board)
// Game phase is used to determine if we are in the middle or endgame

constexpr int MAX_SEARCH_DEPTH = 128; // Covers maximum plausible search depth for minimax + quiescence
// 128 for alignment + would be an extreme case which is near impossible

// Masks for castling rights
constexpr uint64_t WHITE_KINGSIDE_CASTLE_SQUARES = (1ULL << 5) | (1ULL << 6); // (f1, g1)
constexpr uint64_t WHITE_QUEENSIDE_CASTLE_SQUARES = (1ULL << 1) | (1ULL << 2) | (1ULL << 3); // (b1, c1, d1)
constexpr uint64_t BLACK_KINGSIDE_CASTLE_SQUARES = (1ULL << 61) | (1ULL << 62); // (f8, g8)
constexpr uint64_t BLACK_QUEENSIDE_CASTLE_SQUARES = (1ULL << 57) | (1ULL << 58) | (1ULL << 59); // (b8, c8, d8)

// Masks for castling operations
constexpr uint64_t WHITE_KING = (1ULL << 4); // (e1)
constexpr uint64_t BLACK_KING = (1ULL << 60); // (e8)
constexpr uint64_t ROOK_A1 = (1ULL << 0); // (a1)
constexpr uint64_t ROOK_D1 = (1ULL << 3); // (d1)
constexpr uint64_t ROOK_F1 = (1ULL << 5); // (f1)
constexpr uint64_t ROOK_H1 = (1ULL << 7); // (h1)
constexpr uint64_t ROOK_A8 = (1ULL << 56); // (a8)
constexpr uint64_t ROOK_D8 = (1ULL << 59); // (d8)
constexpr uint64_t ROOK_F8 = (1ULL << 61); // (f8)
constexpr uint64_t ROOK_H8 = (1ULL << 63); // (h8)

// Sides are assigned an enum
const enum Color : uint8_t {
    WHITE = 0,
    BLACK = 1
};

// Each piece is assigned a unique integer (4 bits)
const enum PieceType : uint8_t {
    PAWN = 0,
    KNIGHT = 1,
    BISHOP = 2,
    ROOK = 3,
    QUEEN = 4,
    KING = 5,
    EMPTY = 6   // No piece
};

// Defines the type of move (4 bits)
const enum MoveType : uint8_t {
    NORMAL = 0,        // Standard move
    CAPTURE = 1,       // Capturing a piece
    CASTLING = 2,      // Castling (O-O, O-O-O)
    EN_PASSANT = 3,    // En passant capture
    PROMOTION = 4,     // Pawn promotion
    PROMOTION_CAPTURE = 5  // Pawn promotion with capture
};

// Direction type (8 possible directions + 0 for no direction)
const enum Direction : int8_t {
    NORTH = 8,
    SOUTH = -8,
    EAST = 1,
    WEST = -1,
    NORTH_EAST = 9,
    NORTH_WEST = 7,
    SOUTH_EAST = -7,
    SOUTH_WEST = -9,
    NONE = 0
};

// Board state is stored as a bitmask
struct BoardState {
    uint8_t flags = 0; // 8-bit bitfield to store state flags

    static constexpr uint8_t CHECK_WHITE = 1 << 0; // 0000 0001
    static constexpr uint8_t CHECK_BLACK = 1 << 1; // 0000 0010
    static constexpr uint8_t STALEMATE = 1 << 2; // 0000 0100
    static constexpr uint8_t CHECKMATE_WHITE = 1 << 3; // 0000 1000
    static constexpr uint8_t CHECKMATE_BLACK = 1 << 4; // 0001 0000

    bool isCheckWhite() const {
        return flags & CHECK_WHITE;
    }

    bool isCheckBlack() const {
        return flags & CHECK_BLACK;
    }

    bool isStalemate() const {
        return flags & STALEMATE;
    }

    bool isCheckmateWhite() const {
        return flags & CHECKMATE_WHITE;
    }

    bool isCheckmateBlack() const {
        return flags & CHECKMATE_BLACK;
    }
};

// Piece values for board evaluation
// Source: https://www.chessprogramming.org/Simplified_Evaluation_Function
constexpr int PIECE_VALUES[7] = {
    100,   // PAWN
    320,   // KNIGHT
    330,   // BISHOP
    500,   // ROOK
    900,   // QUEEN
    20000, // KING (captures should be handled separately)
    0     // EMPTY (should never be accessed)
};

// MVV_LVA[victim][aggressor] = (VictimValue * 10) - AggressorValue
constexpr int MVV_LVA[6][6] = {
    // Aggressor: PAWN  KNIGHT BISHOP ROOK   QUEEN  KING
    /* PAWN    */ { 900,  880,   870,   500,   100,   0 },
    /* KNIGHT  */ { 3200, 2880,  2870,  2700,  2300,  0 },
    /* BISHOP  */ { 3300, 2980,  2970,  2800,  2400,  0 },
    /* ROOK    */ { 5000, 4680,  4670,  4500,  4100,  0 },
    /* QUEEN   */ { 9000, 8680,  8670,  8500,  8100,  0 },
    /* KING    */ { 0,    0,     0,     0,     0,     0 } // Illegal captures
};

// Game phase recalculation threshold in range of 0-1
// Meaning a phase change greater than this in percentages after applying a move results in full positional score recalculation
// This way we can avoid unnecessary recalculations to prioritize evaluation speed
constexpr float FULL_RECALC_THRESHOLD = 0.1f;

// Piece-square tables for positional scoring evaluation (PSTs)
// Separete tables for middle and endgame
// Tables are designed to be used with white pieces, to use with black pieces, flip the tables
// Source: https://www.chessprogramming.org/Simplified_Evaluation_Function

constexpr int PIECE_TABLE_MID[6][8][8] = {
// PAWN TABLE MID
// In the middle game, pawn structure and control of the center is important
{
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0},
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 5, 10, 10,-20,-20, 10, 10,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
},
// KNIGHT TABLE MID
// Knights are more valuable in the center of the board
// Knight positions don't change significantly in the endgame
{
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
},
// BISHOP TABLE MID
// Bishops are strongest on long diagonals
{
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
},
// ROOK TABLE MID
// Rooks are strongest on open files and the seventh rank
{
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0}
},
// QUEEN TABLE MID
// Queens are versatile and powerful in both phases, but their value increases in the endgame
{
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
},
// KING TABLE MID
// In the middle game, the king should stay safe (e.g., castled)
{
    { -30,-40,-40,-50,-50,-40,-40,-30 },
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20}
}};

constexpr int PIECE_TABLE_END[6][8][8] = {
// PAWN TABLE END
// Pawns are more valuable in the endgame as they approach promotion
// Pawn structure and control of the center is de-emphasized
{
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {80, 80, 80, 80, 80, 80, 80, 80},
    {60, 60, 60, 60, 60, 60, 60, 60},
    {40, 40, 40, 40, 40, 40, 40, 40},
    {20, 20, 20, 20, 20, 20, 20, 20},
    {10, 10, 10, 10, 10, 10, 10, 10},
    { 5,  5,  5,  5,  5,  5,  5,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
},
// KNIGHT TABLE END
{
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
},
// BISHOP TABLE END
// In the endgame, bishop value increases as the board opens up
{
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10,  0, 10, 20, 20, 10,  0,-10},
    {-10,  0, 10, 20, 20, 10,  0,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
},
// ROOK TABLE END
// Rooks are more valuable in the endgame as they can control the board
{
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {10, 20, 20, 20, 20, 20, 20, 10},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0, 10, 10,  0,  0,  0}
},
// QUEEN TABLE END  
{
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
},
// KING TABLE END
// In the endgame, the king becomes an active piece
{
    {-50,-40,-30,-20,-20,-30,-40,-50},
    {-30,-20,-10,  0,  0,-10,-20,-30},
    {-30,-10, 20, 30, 30, 20,-10,-30},
    {-30,-10, 30, 40, 40, 30,-10,-30},
    {-30,-10, 30, 40, 40, 30,-10,-30},
    {-30,-10, 20, 30, 30, 20,-10,-30},
    {-30,-30,  0,  0,  0,  0,-30,-30},
    {-50,-30,-30,-30,-30,-30,-30,-50}
}};

#endif // BITBOARD_CONSTANTS_H