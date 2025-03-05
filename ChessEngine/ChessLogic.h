#ifndef CHESSLOGIC_H
#define CHESSLOGIC_H

class ChessLogic {
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
    ChessLogic();

    // Get player turn
    bool isWhite(); 

    // Helper to get the piece type at a given square
    char getPieceType(int square) const;

    // Helper function to get castling rights as a string
    std::string getCastlingRightsString() const;

    // Get en passant target square as a string
    std::string getEnPassantString() const;

    std::string getHalfMoveString() const;

    std::string getFullMoveString() const;

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
};

#endif CHESSLOGIC_H