#ifndef BOARD_H
#define BOARD_H

// Pre-computed variables
constexpr int UNASSIGNED = -1; // Sentinel value for unassigned variables
constexpr int MAX_MOVES = 32; // Max number of legal moves for a piece, in theory max would be 28 (for queen) but we use 32 for alignment and placement

// Masks for castling rights
constexpr uint64_t WHITE_KINGSIDE_CASTLE_SQUARES = (1ULL << 5) | (1ULL << 6); // (f1, g1)
constexpr uint64_t WHITE_QUEENSIDE_CASTLE_SQUARES = (1ULL << 1) | (1ULL << 3); // (b1, c1, d1)
constexpr uint64_t BLACK_KINGSIDE_CASTLE_SQUARES = (1ULL << 61) | (1ULL << 62); // (f8, g8)
constexpr uint64_t BLACK_QUEENSIDE_CASTLE_SQUARES = (1ULL << 57) | (1ULL << 59); // (b8, c8, d8)
constexpr uint64_t WHITE_KING = 1ULL << 4; // (e1)
constexpr uint64_t BLACK_KING = 1ULL << 60; // (e8)

class Bitboard {
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
    Bitboard();

    // Get player turn
    bool isWhite(); 

    void switchTurn();

    // Helper to get the piece type at a given square
    char getPieceType(int square) const;

    // Helper function to get castling rights as a string
    std::string getCastlingRightsString() const;

    // Get en passant target square as a string
    std::string getEnPassantString() const;


    // checks for checks
    bool isInCheck();

    // checks checkmates
    bool isCheckmate();

    // Get half moves
    int getHalfMoveClock() const;

    // Get full moves 
    int getFullMoveNumber() const;

    // Get all legal moves from a square as a bitboard
    // Takes the source square as the parameter
    uint64_t getLegalMoves(int from);

    // Apply move by updating bitboards
    // Takes the source and target as parameters
    // Move is applied only after making sure its legal, meaning no need to check for validity
    void applyMove(int source, int target);

private:
    // Get locations of white or black pieces (bitboard)
     // Uses bitwise OR operation to combine occupancy of all pieces of same color
    // To get all occupied squares, combine these two functions with bitwise OR
    uint64_t whitePieces();
    uint64_t blackPieces();

    // Helper function to convert a square index to algebraic notation
    std::string squareToString(int square) const;

    // Helper functions to create legal moves for different piece types
    uint64_t getPawnMoves(int pawn);
    uint64_t getPawnCaptures(int pawn); // Used for attack squares
    uint64_t getKnightMoves(int knight);
    uint64_t getBishopMoves(int bishop);
    uint64_t getRookMoves(int rook);
    uint64_t getQueenMoves(int queen);
    uint64_t getKingMoves(int king);

    // Helper to get the sliding moves of a movetable
    // Used for Bishop, Rook and Queen, since can't leap over other pieces
    // Uses LSB/FSB to isolate the occupied bit depending on move direction
    uint64_t getSlidingMoves(uint64_t direction_moves, bool reverse);

    // Helper to get castling moves for a king
    uint64_t getCastlingMoves();

    // Helper to get all the attack squares of opponent (squares that are possible to attack)
    // If white turn, we get all the squares black could attack, and vice versa
    // Gets all the possible squares as a bitboard
    uint64_t getAttackSquares();

    // Helper function to find the index of first set bit and last set bit
    // Use inline to avoid function call overhead
    inline int findFirstSetBit(uint64_t value);
    inline int findLastSetBit(uint64_t value);
};

#endif CHESSLOGIC_H