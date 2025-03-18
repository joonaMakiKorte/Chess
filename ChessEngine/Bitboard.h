#ifndef BOARD_H
#define BOARD_H

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

    // Get game state as a string
    std::string getGameState();

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
    uint64_t getPawnMoves(int pawn, const uint64_t& white_pieces, const uint64_t& black_pieces);
    uint64_t getPawnCaptures(int pawn, const uint64_t& white_pieces, const uint64_t& black_pieces); // Used for attack squares
    uint64_t getKnightMoves(int knight, const uint64_t& white_pieces, const uint64_t& black_pieces);
    uint64_t getBishopMoves(int bishop, const uint64_t& white_pieces, const uint64_t& black_pieces);
    uint64_t getRookMoves(int rook, const uint64_t& white_pieces, const uint64_t& black_pieces);
    uint64_t getQueenMoves(int queen, const uint64_t& white_pieces, const uint64_t& black_pieces);
    uint64_t getKingMoves(int king);

    // Helper to get the sliding moves of a movetable
    // Used for Bishop, Rook and Queen, since can't leap over other pieces
    // Uses LSB/FSB to isolate the occupied bit depending on move direction
    uint64_t getSlidingMoves(uint64_t direction_moves, bool reverse, const uint64_t& white_pieces, const uint64_t& black_pieces);

    // Helper to get castling moves for a king
    uint64_t getCastlingMoves();

    // Helper to get all the attack squares of opponent (squares that are possible to attack)
    // If white turn, we get all the squares black could attack, and vice versa
    // Takes bitboards of both of the pieces as the parameter
    // Gets all the possible squares as a bitboard
    uint64_t getAttackSquares(const uint64_t& white_pieces, const uint64_t& black_pieces);

    // Helper to get king attackers locations
    // Returns the attackers as a bitboard
    uint64_t getAttackers(uint64_t king, const uint64_t& white_pieces, const uint64_t& black_pieces);

    // Find the ray which must be blocked if the king is in check
    // If attacker is pawn or knight, returns only the piece location
    // If rook, bishop or queen, gets the whole attacking ray
    uint64_t getAttackingRay(int attacker, int king);

    // Helper for calculating the attacking ray between attacker and king
    // Calculates the square difference of the two pieces and forms the ray on that info
    uint64_t formAttackingRay(int attacker, int king);

    // Determine if the attacking ray can be blocked by any of the own pieces
    // Returns bool indicating result
    bool canBlock(const uint64_t& attack_ray);

    // Check game state
    bool isInCheck();
    bool isCheckmate();
    bool isStalemate();

private:
    // Helper function to find the index of first set bit and last set bit
    // Use inline to avoid function call overhead
    inline int findFirstSetBit(uint64_t value);
    inline int findLastSetBit(uint64_t value);

    // Helper to get direction between two squares from their difference
    // Done by normalizing the movement direction to match one of the 8 possible moving directions
    inline int get_direction(int diff);
};

#endif CHESSLOGIC_H