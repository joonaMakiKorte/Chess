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

// Masks for castling rights
constexpr uint64_t WHITE_KINGSIDE_CASTLE_SQUARES = (1ULL << 5) | (1ULL << 6); // (f1, g1)
constexpr uint64_t WHITE_QUEENSIDE_CASTLE_SQUARES = (1ULL << 1) | (1ULL << 2) | (1ULL << 3); // (b1, c1, d1)
constexpr uint64_t BLACK_KINGSIDE_CASTLE_SQUARES = (1ULL << 61) | (1ULL << 62); // (f8, g8)
constexpr uint64_t BLACK_QUEENSIDE_CASTLE_SQUARES = (1ULL << 57) | (1ULL << 58) | (1ULL << 59); // (b8, c8, d8)
constexpr uint64_t WHITE_KING = (1ULL << 4); // (e1)
constexpr uint64_t BLACK_KING = (1ULL << 60); // (e8)

// Each piece is assigned a unique integer (4 bits)
const enum PieceType : uint8_t {
    EMPTY = 0,   // No piece
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6
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

constexpr int MAX_GAME_PHASE = 24; // Maximum game phase (total number of pieces on board)
// Game phase is used to determine if we are in the middle or endgame

// Piece values for board evaluation
// Source: https://www.chessprogramming.org/Simplified_Evaluation_Function
constexpr int PAWN_VALUE = 100;
constexpr int KNIGHT_VALUE = 320;
constexpr int BISHOP_VALUE = 330;
constexpr int ROOK_VALUE = 500;
constexpr int QUEEN_VALUE = 900;
constexpr int KING_VALUE = 20000;

// Piece-square tables for positional scoring evaluation (PSTs)
// Separete tables for middle and endgame
// Tables are designed to be used with white pieces, to use with black pieces, flip the tables
// Source: https://www.chessprogramming.org/Simplified_Evaluation_Function

// In the middle game, pawn structure and control of the center is important
constexpr int PawnTableMid[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0},
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 5, 10, 10,-20,-20, 10, 10,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

// Pawns are more valuable in the endgame as they approach promotion
// Pawn structure and control of the center is de-emphasized
constexpr int PawnTableEnd[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {80, 80, 80, 80, 80, 80, 80, 80},
    {60, 60, 60, 60, 60, 60, 60, 60},
    {40, 40, 40, 40, 40, 40, 40, 40},
    {20, 20, 20, 20, 20, 20, 20, 20},
    {10, 10, 10, 10, 10, 10, 10, 10},
    { 5,  5,  5,  5,  5,  5,  5,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

// Knights are more valuable in the center of the board
// Knight positions don't change significantly in the endgame
constexpr int KnightTableMid[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

constexpr int KnightTableEnd[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

// Bishops are strongest on long diagonals
constexpr int BishopTableMid[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

// In the endgame, bishop value increases as the board opens up
constexpr int BishopTableEnd[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10,  0, 10, 20, 20, 10,  0,-10},
    {-10,  0, 10, 20, 20, 10,  0,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

// Rooks are strongest on open files and the seventh rank
constexpr int RookTableMid[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0}
};

// Rooks are more valuable in the endgame as they can control the board
constexpr int RookTableEnd[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {10, 20, 20, 20, 20, 20, 20, 10},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0, 10, 10,  0,  0,  0}
};

// Queens are versatile and powerful in both phases, but their value increases in the endgame
constexpr int QueenTableMid[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

constexpr int QueenTableEnd[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

// In the middle game, the king should stay safe (e.g., castled)
constexpr int KingTableMid[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20}
};

// In the endgame, the king becomes an active piece
constexpr int KingTableEnd[8][8] = {
	{-50,-40,-30,-20,-20,-30,-40,-50},
	{-30,-20,-10,  0,  0,-10,-20,-30},
	{-30,-10, 20, 30, 30, 20,-10,-30},
	{-30,-10, 30, 40, 40, 30,-10,-30},
	{-30,-10, 30, 40, 40, 30,-10,-30},
	{-30,-10, 20, 30, 30, 20,-10,-30},
	{-30,-30,  0,  0,  0,  0,-30,-30},
	{-50,-30,-30,-30,-30,-30,-30,-50}
};

#endif // BITBOARD_CONSTANTS_H