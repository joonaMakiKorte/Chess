#include "ChessBoard.h"

ChessBoard::Board::Board() {
	white_pawns = 0x000000000000FF00;      // a2, b2, c2, d2, e2, f2, g2, h2
	black_pawns = 0x00FF000000000000;      // a7, b7, c7, d7, e7, f7, g7, h7
	white_rooks = 0x0000000000000081;      // a1, h1
	black_rooks = 0x8100000000000000;      // a8, h8
	white_knights = 0x0000000000000042;    // b1, g1
	black_knights = 0x4200000000000000;    // b8, g8
	white_bishops = 0x0000000000000024;    // c1, f1
	black_bishops = 0x2400000000000000;    // c8, f8
	white_queen = 0x0000000000000010;      // d1
	black_queen = 0x1000000000000000;      // d8
	white_king = 0x0000000000000008;       // e1
	black_king = 0x0800000000000000;       // e8
}

ChessBoard::ChessBoard() {
	chess_board = Board();
}
