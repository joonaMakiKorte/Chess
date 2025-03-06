#ifndef BOARD_H
#define BOARD_H

class Board {
private:
    // Represent each piece type as a bitboard
    uint64_t white_pawns;
    uint64_t black_pawns;
    uint64_t white_knights;
    uint64_t black_knights;
    uint64_t white_bishops;
    uint64_t black_bishops;
    uint64_t white_rooks;
    uint64_t black_rooks;
    uint64_t white_queen;
    uint64_t black_queen;
    uint64_t white_king;
    uint64_t black_king;

    // Store castling rights as a bitmask
    // Bit 0 : White kingside(K)
    // Bit 1 : White queenside(Q)
    // Bit 2 : Black kingside(k)
    // Bit 3 : Black queenside(q)
    uint8_t castling_rights;

    // The square where a pawn can be captured en passant
    // If not possible, set UNASSIGNED
    int en_passant_target;

    bool white; // Track the player turns
    int half_moves; // Helps determine if a draw can be claimed
    int full_moves; // For game analysis and record keeping

public:
    // Initialize each piece with starting pos
    Board();

    // Get player turn
    bool isWhite(); 

    // Helper to get the piece type at a given square
    char getPieceType(uint64_t square) const;

    // Helper function to get castling rights as a string
    std::string getCastlingRightsString() const;

    // Get en passant target square as a string
    std::string getEnPassantString() const;

    // Get half moves
    int getHalfMoveClock() const;

    // Get full moves 
    int getFullMoveNumber() const;

    // Get all legal moves from a square as a bitboard
    // Takes the source square as the parameter
    uint64_t getLegalMoves(uint64_t from);

private:
    // Get locations of white or black pieces (bitboard)
     // Uses bitwise OR operation to combine occupancy of all pieces of same color
    const uint64_t whitePieces() const;
    const uint64_t blackPieces() const;

    // Get all occupied squares (bitboard)
    // Uses bitwise OR operation to combine occupancy of all white and black pieces
    const uint64_t getOccupied() const;

    // Helper function to convert a square index to algebraic notation
    std::string squareToString(int square) const;

    // Helper functions to create legal moves for different piece types
    uint64_t getPawnMoves(uint64_t pawn);
    uint64_t getKnightMoves(uint64_t knight);
    uint64_t getBishopMoves(uint64_t bishop);
    uint64_t getRookMoves(uint64_t rook);
    uint64_t getQueenMoves(uint64_t queen);
    uint64_t getKingMoves(uint64_t king);
};

#endif CHESSLOGIC_H