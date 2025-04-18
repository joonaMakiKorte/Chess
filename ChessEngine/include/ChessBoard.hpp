#ifndef CHESSBOARD_H
#define CHESSBOARD_H	

#include "Bitboard.hpp"

class ChessBoard {
private:
    // Store the board logic
    std::unique_ptr<Bitboard> board;

    // Keep track of player turns
    bool white;

    // Track full moves
    // Incremented after black
    int full_moves;

    // Flag to track game state
    // Once endgame, calls different AI function for getting best move
    bool isEndgame;

    // Store the previously applied move
    std::string previous_move;

public:
    // Initialize the chessboard
    ChessBoard();

    ~ChessBoard() = default; // unique_ptr handles bitboard cleanup

    // Get legal moves from the parameter square as a bitboard
    uint64_t LegalMoves(int square);

    // Move a chessboard piece according to given parameter
    // Takes promotion type as char ('-' if none)
    // Update chessboard status accordingly
    // Is called in C# if validating move successful
    void MovePiece(int source, int target, char promotion);

    // Get best move for ai and apply it
    // Determined by search depth
    void MakeMoveAI(int depth, bool maximizing);


    // Return board state as a FEN string (Forsyth-Edwards Notation)
    // For starting position the FEN string would be: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    // Explanation:
    // 8 ranks, '/' separates the ranks
    // 'p'=pawn, 'r'=rook, 'n'=knight, 'b'=bishop, 'q'=queen, 'k'=king
    // Lowercase = black, uppercase = white
    // 
    // 'w' = active color
    // 'KQkq' = catling rights
    // '-' = En Passant Target Square
    // '0' = half move clock
    // '1' = full move number
    std::string GetFEN();

    // Get game state
    // Different states:
    /*
    ongoing
    check
    mate
    stalemate
    draw_repetition
    draw_50
    draw_insufficient
    */
    std::string GetGameState();

    // Get previous move in algrebraic notation
    std::string GetPrevMove() const;

private:
    // Set previous move dynamically
    void UpdatePrevMove(const std::string& message);

    // Print bitboard in algebraic notation for debug
    std::string printBitboardAsSquares(uint64_t bitboard) const;

    char getPieceLetter(PieceType piece) const;

    char getFileLetter(int square) const;

    // Transform the applied move to algebraic notation
    std::string getMoveNotation(uint32_t move) const;
};

#endif // CHESSBOARD_H