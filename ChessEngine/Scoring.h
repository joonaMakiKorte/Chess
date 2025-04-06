#ifndef SCORING_H
#define SCORING_H

constexpr int KILLER_SCORE = 9000; // Score to prioritize killer moves
constexpr int TT_MOVE_SCORE = 100000; // Score for TT-hint moves
constexpr int QUEEN_PROMOTION = 20000; // Huge priority for queen promotion
constexpr int ROOK_PROMOTION = 8000; // Priority for rook promotion, lower than queen but higher than most captures
constexpr int BN_PROMOTION = 1500; // Bishop/knight promotions equal to minor captures (tactical)


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

// Endgame MVV_LVA[victim][aggressor] = (VictimValue * 6) - AggressorValue
// Scaled down to prioritize checks/promotions over raw captures.
constexpr int MVV_LVA_ENDGAME[6][6] = {
    // Aggressor: PAWN  KNIGHT BISHOP ROOK   QUEEN  KING
    /* PAWN    */ { 500,  480,   470,   300,    50,  -100 }, // Penalize KxP
    /* KNIGHT  */ { 1800, 1500,  1490,  1300,   900,  -200 }, // Discourage KxN
    /* BISHOP  */ { 1900, 1600,  1590,  1400,  1000,  -200 }, // Discourage KxB
    /* ROOK    */ { 3000, 2700,  2690,  2500,  2100,  -500 }, // Rarely trade R
    /* QUEEN   */ { 5400, 5100,  5090,  4900,  4500, -1000 }, // Keep queens
    /* KING    */ {    0,    0,     0,     0,     0,     0 }  // Illegal
};

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
    } };

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
    } };

#endif