#ifndef BITBOARD_H
#define BITBOARD_H

#include "BitboardConstants.h"
#include "CustomTypes.h"

class Bitboard {
private:
    // Piece bitboards indexed by [color][pieceType]
    uint64_t piece_bitboards[2][6];

    // Lookup table for fast piece type checking
    PieceType piece_at_square[64];

    // Board state scores updated incrementally
    int material_score;
    int positional_score;
    int game_phase_score;

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

    UndoInfo undo_stack[MAX_SEARCH_DEPTH];  // Fixed-size stack for move undoing
    int undo_stack_top; // Index of stack top

    PinData pin_data;

    AttackData attack_data;

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
    // Incrementally updates game phase and material scores
    void applyMove(int source, int target, bool white);

    // Apply promotion by updating bitboards
	// Move has already been applied , so only need to promote the pawn
	// Takes the target square and promotion piece as parameters
    // Incrementally updates game phase and material scores
	void applyPromotion(int target, char promotion, bool white);

private:
    // Get locations of white or black pieces (bitboard)
    // Uses bitwise OR operation to combine occupancy of all pieces of same color
    // To get all occupied squares, combine these two functions with bitwise OR
    uint64_t whitePieces();
    uint64_t blackPieces();

    // Helper function to convert a square index to algebraic notation
    std::string squareToString(int square) const;

    // Helper to get castling moves for a king
    uint64_t getCastlingMoves(bool white);

    // Update castling rights when rook was moved or captured
    void updateRookCastling(bool white, int source);

    // Perform castling by moving king and rook in correct places
    void handleCastling(bool white, int target);

    // Helper to get all the attack squares of opponent (squares that are possible to attack)
    // Also determines if king is in check and calculates the attack ray
    void getAttackSquares(int enemy_king, const uint64_t& white_pieces, const uint64_t black_pieces, bool white);

    // Determine if the attacking ray can be blocked by any of the own pieces
    // Returns bool indicating result
    bool canBlock(bool white);

    bool isCheckmate(bool white);
    bool isStalemate(bool white);

    // Each time after applying a move set the new board state
    // Includes check, checkmate and stalemate information
    // Only updating the necessary side
    void updateBoardState(bool white);

    // Calculate positional scores of pieces
    // Expensive function call since iterates over every piece, but only called after human applied move so no visible effect
    void updatePositionalScore();

public:
    // Reset undo stack
    // Sets top element index to 0
    void resetUndoStack();

    // Function for ChessAI to generate the legal moves
    // Only handle queen promotions, underpromotions deferred to quiescence
    // Fills the movelist taken as parameter depending if we are minimizing/maximizing (which turn)
	// Sorts the moves with MVV-LVA (Most Valuable Victim - Least Valuable Aggressor) heuristic
    void generateMoves(std::array<uint32_t, MAX_MOVES>& move_list, int& move_count, int depth, bool white);

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

    // Evaluate if we are in endgame
    // Done by comparing current game phase score to endgame threshold
    bool isEndgame();

    // Get distance between kings
    // Used in endgame eval heuristic
    // Closer kings get higher bonus
    int calculateKingDistance();

    // Calculate opponent's kings distance from the closest board edge
    // Used in endgame eval heuristic
    int calculateKingEdgeDistance(bool white);

    // Evaluate passed pawns
    // Used in endgame eval heuristic
    // Critical in endgame evaluations
    int evaluatePassedPawns(bool white);

private: 
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

    inline int getPositionalScore(int square, float game_phase,  PieceType piece, bool white);

    // Helper to determine if a pawn if passed
    // Passed pawns = pawns with no opposing pawns blocking their promoting path
    bool isPassedPawn(int pawn, bool white);
};

#endif BITBOARD_H