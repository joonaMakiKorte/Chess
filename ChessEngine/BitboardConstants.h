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

constexpr uint16_t NULL_MOVE = 0xFFFF; // An impossible move (all bits set)
constexpr int MAX_HISTORY_KEY = 65536; // Max value for history table key (max 16-bit int)

constexpr int MAX_MOVES = 256; // Max number of legal moves for a player turn
// Absolute theoretical maximum would be 218 but we use 256 (a power of 2) for efficient memory alignment

constexpr int MAX_GAME_PHASE = 24; // Maximum game phase (total number of pieces on board)
// Game phase is used to determine if we are in the middle or endgame

constexpr int MAX_DEPTH = 64;  // Maximum playsible search depth for minimax

constexpr int MAX_SEARCH_DEPTH = 128; // Covers maximum plausible search depth for minimax + quiescence
// 128 for alignment + would be an extreme case which is near impossible

constexpr int KILLER_SCORE = 9000; // Score to prioritize killer moves

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

// Threshold for game phase score for which after we start using the endgame board evaluation in quiescence
// Opening 22-24     // Most pieces still on the board
// Middlegame 10-21 // Some exhanges, queens often present
// Endgame 0-9     // Few pieces left, kings active
constexpr int ENDGAME_THRESHOLD = 9; 

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