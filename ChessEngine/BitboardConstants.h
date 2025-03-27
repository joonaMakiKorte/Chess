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
    0,     // EMPTY (should never be accessed)
    100,   // PAWN
    320,   // KNIGHT
    330,   // BISHOP
    500,   // ROOK
    900,   // QUEEN
    20000  // KING (captures should be handled separately)
};

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


// Magic tables for sliding piece move generations
constexpr uint64_t MAGICS_BISHOP[64] = {
      0x40040844404084ULL,
      0x2004208a004208ULL,
      0x10190041080202ULL,
      0x108060845042010ULL,
      0x581104180800210ULL,
      0x2112080446200010ULL,
      0x1080820820060210ULL,
      0x3c0808410220200ULL,
      0x4050404440404ULL,
      0x21001420088ULL,
      0x24d0080801082102ULL,
      0x1020a0a020400ULL,
      0x40308200402ULL,
      0x4011002100800ULL,
      0x401484104104005ULL,
      0x801010402020200ULL,
      0x400210c3880100ULL,
      0x404022024108200ULL,
      0x810018200204102ULL,
      0x4002801a02003ULL,
      0x85040820080400ULL,
      0x810102c808880400ULL,
      0xe900410884800ULL,
      0x8002020480840102ULL,
      0x220200865090201ULL,
      0x2010100a02021202ULL,
      0x152048408022401ULL,
      0x20080002081110ULL,
      0x4001001021004000ULL,
      0x800040400a011002ULL,
      0xe4004081011002ULL,
      0x1c004001012080ULL,
      0x8004200962a00220ULL,
      0x8422100208500202ULL,
      0x2000402200300c08ULL,
      0x8646020080080080ULL,
      0x80020a0200100808ULL,
      0x2010004880111000ULL,
      0x623000a080011400ULL,
      0x42008c0340209202ULL,
      0x209188240001000ULL,
      0x400408a884001800ULL,
      0x110400a6080400ULL,
      0x1840060a44020800ULL,
      0x90080104000041ULL,
      0x201011000808101ULL,
      0x1a2208080504f080ULL,
      0x8012020600211212ULL,
      0x500861011240000ULL,
      0x180806108200800ULL,
      0x4000020e01040044ULL,
      0x300000261044000aULL,
      0x802241102020002ULL,
      0x20906061210001ULL,
      0x5a84841004010310ULL,
      0x4010801011c04ULL,
      0xa010109502200ULL,
      0x4a02012000ULL,
      0x500201010098b028ULL,
      0x8040002811040900ULL,
      0x28000010020204ULL,
      0x6000020202d0240ULL,
      0x8918844842082200ULL,
      0x4010011029020020ULL 
};

constexpr uint64_t MAGICS_ROOK[64] = {
      0x8a80104000800020ULL,
      0x140002000100040ULL,
      0x2801880a0017001ULL,
      0x100081001000420ULL,
      0x200020010080420ULL,
      0x3001c0002010008ULL,
      0x8480008002000100ULL,
      0x2080088004402900ULL,
      0x800098204000ULL,
      0x2024401000200040ULL,
      0x100802000801000ULL,
      0x120800800801000ULL,
      0x208808088000400ULL,
      0x2802200800400ULL,
      0x2200800100020080ULL,
      0x801000060821100ULL,
      0x80044006422000ULL,
      0x100808020004000ULL,
      0x12108a0010204200ULL,
      0x140848010000802ULL,
      0x481828014002800ULL,
      0x8094004002004100ULL,
      0x4010040010010802ULL,
      0x20008806104ULL,
      0x100400080208000ULL,
      0x2040002120081000ULL,
      0x21200680100081ULL,
      0x20100080080080ULL,
      0x2000a00200410ULL,
      0x20080800400ULL,
      0x80088400100102ULL,
      0x80004600042881ULL,
      0x4040008040800020ULL,
      0x440003000200801ULL,
      0x4200011004500ULL,
      0x188020010100100ULL,
      0x14800401802800ULL,
      0x2080040080800200ULL,
      0x124080204001001ULL,
      0x200046502000484ULL,
      0x480400080088020ULL,
      0x1000422010034000ULL,
      0x30200100110040ULL,
      0x100021010009ULL,
      0x2002080100110004ULL,
      0x202008004008002ULL,
      0x20020004010100ULL,
      0x2048440040820001ULL,
      0x101002200408200ULL,
      0x40802000401080ULL,
      0x4008142004410100ULL,
      0x2060820c0120200ULL,
      0x1001004080100ULL,
      0x20c020080040080ULL,
      0x2935610830022400ULL,
      0x44440041009200ULL,
      0x280001040802101ULL,
      0x2100190040002085ULL,
      0x80c0084100102001ULL,
      0x4024081001000421ULL,
      0x20030a0244872ULL,
      0x12001008414402ULL,
      0x2006104900a0804ULL,
      0x1004081002402ULL 
};

constexpr int RELEVANT_BITS_COUNT_BISHOP[64] = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6,
};


constexpr int RELEVANT_BITS_COUNT_ROOK[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12,
};

#endif // BITBOARD_CONSTANTS_H