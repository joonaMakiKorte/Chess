#ifndef BITBOARD_H
#define BITBOARD_H

#include "BitboardConstants.h"
#include "ChessAI.h"

class Bitboard {
private:
    // Piece bitboards indexed by [color][pieceType]
    uint64_t piece_bitboards[2][6];

    // Store castling rights as a bitmask
    // Bit 0 : White kingside(K)
    // Bit 1 : White queenside(Q)
    // Bit 2 : Black kingside(k)
    // Bit 3 : Black queenside(q)
    uint8_t castling_rights;

    // The square where a pawn can be captured en passant
    // If not possible, set UNASSIGNED
    int en_passant_target;

    int half_moves; // Helps determine if a draw can be claimed
    int full_moves; // For game analysis and record keeping

    // Save previous board states for faster state recovery in move undoing
    struct UndoInfo {
        // Save castling and en passant
        uint8_t castling_rights;
        int en_passant_target;

        // Flags of the game state
        uint8_t flags;
    };
    UndoInfo undo_stack[MAX_SEARCH_DEPTH];  // Fixed-size stack
    int undo_stack_top;

public:
    // Initialize each piece with starting pos
    Bitboard();

    // Store the game state as a bitmask
    BoardState state;

    // Helper to get the piece type at a given square
    char getPieceTypeChar(int square) const;

    // Helper function to get castling rights as a string
    std::string getCastlingRightsString() const;

    // Get en passant target square as a string
    std::string getEnPassantString() const;

    // Get game state as a string
    std::string getGameState(bool white);

    // Get half moves
    int getHalfMoveClock() const;

    // Get full moves 
    int getFullMoveNumber() const;

    // Get all legal moves from a square as a bitboard
    // Takes the source square and turn as the parameters
    uint64_t getLegalMoves(int from, bool white);

    // Apply move by updating bitboards
    // Takes the source and target as parameters
    // Move is applied only after making sure its legal, meaning no need to check for validity
    void applyMove(int source, int target, bool white);

    // Apply promotion by updating bitboards
	// Move has already been applied , so only need to promote the pawn
	// Takes the target square and promotion piece as parameters
	void applyPromotion(int target, char promotion, bool white);

private:
    // Get locations of white or black pieces (bitboard)
    // Uses bitwise OR operation to combine occupancy of all pieces of same color
    // To get all occupied squares, combine these two functions with bitwise OR
    uint64_t whitePieces();
    uint64_t blackPieces();

    // Helper function to convert a square index to algebraic notation
    std::string squareToString(int square) const;

    // Helper functions to create legal moves for different piece types
    uint64_t getKingMoves(int king, uint64_t white_pieces, uint64_t black_pieces, bool white);

    // Helper to get castling moves for a king
    uint64_t getCastlingMoves(bool white);

    // Update castling rights when rook was moved
    void updateRookCastling(bool white, int source);

    // Perform castling by moving king and rook in correct places
    void handleCastling(bool white, int target);

    // Helper to get all the attack squares of opponent (squares that are possible to attack)
    // If white turn, we get all the squares black could attack, and vice versa
    // Takes bitboards of both of the pieces as the parameter
    // Gets all the possible squares as a bitboard
    uint64_t getAttackSquares(const uint64_t& white_pieces, const uint64_t& black_pieces, bool white);

    // Helper to get king attackers locations
    // Returns the attackers as a bitboard
    uint64_t getAttackers(uint64_t king, const uint64_t& white_pieces, const uint64_t& black_pieces, bool white);

    // Find the ray which must be blocked if the king is in check
    // If attacker is pawn or knight, returns only the piece location
    // If rook, bishop or queen, gets the whole attacking ray
    uint64_t getAttackingRay(int attacker, int king);

    // Helper for calculating the attacking ray between attacker and king
    // Calculates the square difference of the two pieces and forms the ray on that info
    uint64_t formAttackingRay(int attacker, int king);

    // Determine if the attacking ray can be blocked by any of the own pieces
    // Returns bool indicating result
    bool canBlock(const uint64_t& attack_ray, bool white);

    // Each time after applying a move set the new board state
	// Includes check, checkmate and stalemate information
    void updateBoardState();

    // Check game state
    bool isInCheck(bool white);
    bool isCheckmate(bool white);
    bool isStalemate(bool white);

public:
    // Reset undo stack
    // Sets top element index to 0
    void resetUndoStack();

    // Function for ChessAI to generate the legal moves
    // Fills the movelist taken as parameter depending if we are minimizing/maximizing (which turn)
	// Sorts the moves with MVV-LVA (Most Valuable Victim - Least Valuable Aggressor) heuristic
    void generateMoves(std::array<uint32_t, MAX_MOVES>& move_list, int& move_count, bool white);

	// Function for ChessAI to generate noisy moves
	// Used for quiescence search to reduce horizon effect
	// Noisy moves are captures and promotions
	void generateNoisyMoves(std::array<uint32_t, MAX_MOVES>& move_list, int& move_count, bool white);

	// Function for ChessAI to apply the move
	// Takes the encoded move as a parameter and applies it to the board
    // Also saves the en passant target and castling rights before applying move for later undoign
	void applyMoveAI(uint32_t move, bool white);

	// Function for ChessAI to undo the move
	// Takes the encoded move as a parameter and undoes it
	void undoMoveAI(uint32_t move, bool white);

    // Function to assign a score to the board
	// Used for evaluation of the board state
	int evaluateBoard(bool white);

	// Function to check if the game is over
	// Checkmate or stalemate for either side
	bool isGameOver();

    // Calculate legal moves for the king
    // Used for evaluating the king mobility
	int calculateKingMobility(bool white);

    // Used in quiescence search for delta pruning
    int estimateCaptureValue(uint32_t move);

private: 
	// Helper to get correct piece enum corresponding to the piece type
	// Used for encoding moves
	PieceType getPieceType(int square) const;

    // Helper to get the correct piece bitboard as a reference from enum
	// For example if piece is PAWN, returns white_pawns or black_pawns depending on the color
	uint64_t& getPieceBitboard(PieceType piece, bool white);

	// Helper to get correct move type depending on the target square and piece type
	// Used for encoding moves
	MoveType getMoveType(int source_square, int target_square, PieceType piece, PieceType target_piece, bool white) const;

	// Helper for undoing castling, moves rook back to original position
	// Takes the active color and castling side as parameters
	void undoCastling(bool white, bool kingside);

    // Calculate the material score of the board
	int calculateMaterialScore(bool white);

	// Calculate the positional score of the board
	int calculatePositionalScore(bool white);

	// Determine the game phase score (middle or endgame)
    // Is based on the remaining pieces
	int calculateGamePhase();
};

#endif BITBOARD_H