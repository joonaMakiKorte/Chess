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

    PinData pin_data; // Data of pinned pieces
    AttackData attack_data; // Data of attack squares and attack ray to king

    uint64_t hash_key; // Unique key updated incrementally after each move

    // Zobrist hashing for threefold repetition detection
    // Updated after applying an actual move (not ai searches)
    std::unordered_map<uint64_t, int> position_history;

    // Stack to store undo-info for efficient board state restoring
    // Used by ai in minimax and q-search
    std::vector<UndoInfo> undo_stack;

    // Stack to hold the position history for the current search path
    // Used by ai for draw detection in search paths
    std::vector<uint64_t> search_history;

public:
    // Initialize each piece with starting pos
    Bitboard();

    // Store the game state as a bitmask
    BoardState state;

    // Helpers for FEN-string creation
    char getPieceTypeChar(int square) const;
    std::string getCastlingRightsString() const;
    std::string getEnPassantString() const;
    std::string getGameState(bool white);
    int getHalfMoveClock() const; 
    int getFullMoveNumber() const;
    std::string squareToString(int square) const;

    // Get all legal moves from a square as a bitboard
    // Takes the source square and turn as the parameters
    uint64_t getLegalMoves(int from, bool white);

    // Apply move by updating bitboards
    // Takes the source and target as parameters (+promoting piece if available)
    // Move is applied only after making sure its legal, meaning no need to check for validity
    // Incrementally updates game phase and material scores
    // Returns the move encoded
    uint32_t applyMove(int source, int target, PieceType promotion, bool white);

    // Evaluate if we are in the endgame
    bool isEndgame();

    // Check if the move resulted in and update state accordingly
    void updateDrawByRepetition();

private:
    // Initialize board data at the beginning of the game
    void initBoard();

    // Compute Zobrist hash-key at the beginning of the game
    // Updated incrementally during game, meaning no need for full re-calculation
    uint64_t computeZobristHash();

    // Get locations of white or black pieces (bitboard)
    // Uses bitwise OR operation to combine occupancy of all pieces of same color
    // To get all occupied squares, combine these two functions with bitwise OR
    uint64_t whitePieces();
    uint64_t blackPieces();

    // Functions for castling
    uint64_t getCastlingMoves(bool white); // Get currently possible castling moves for a king
    void updateRookCastling(bool white, int source); // Update castling rights when rook was moved or captured
    void handleCastling(bool white, int target); // Perform castling by moving king and rook in correct places
    void undoCastling(bool white, bool kingside); // Undo castling, used by AI

    // Helper to get all the attack squares of opponent (squares that are possible to attack)
    // Also determines if king is in check and calculates the attack ray
    void getAttackSquares(int enemy_king, const uint64_t& white_pieces, const uint64_t black_pieces, bool white);

    // Determine if the attacking ray can be blocked by any of the own pieces
    // Returns bool indicating result
    bool canBlock(bool white);

    bool isCheckmate(bool white); // If in check, must be checked if in checkmate
    bool isStalemate(bool white); // If not in check/mate, check for possibility of stalemate

    // Each time after applying a move set the new board state
    // Includes check, checkmate and stalemate information
    // Only updating the necessary side
    void updateBoardState(bool white);

    // Calculate positional scores of pieces
    // Expensive function call since iterates over every piece, but only called after human applied move so no visible effect
    void updatePositionalScore();

public:
    /**************************************************************
    The functions below are used directly by the chessAI in minimax
    **************************************************************/

    // Undo stack and search history are cleared
    void startNewSearch();

    // Used for draw detection
    uint64_t getHashKey();

    // Function for ChessAI to generate the legal moves
    // Only handle queen promotions
    // Fills the movelist taken as parameter depending if we are minimizing/maximizing (which turn)
	// Sorts the moves with MVV-LVA (Most Valuable Victim - Least Valuable Aggressor) heuristic
    void generateMoves(std::array<uint32_t, MAX_MOVES>& move_list, int& move_count, int depth, bool white, uint32_t move_hint);

	// Function for ChessAI to generate noisy moves
	// Used for quiescence search to reduce horizon effect
	// Consider captures to resolve immediate tactical volatility
    // + only queen promotions
	void generateNoisyMoves(std::array<uint32_t, MAX_MOVES>& move_list, int& move_count, bool white);

    // Generate all legal moves sorted with endgame heuristic
    // Check moves are highest priority, also prioritize passed pawn advancement and king centrality
    void generateEndgameMoves(std::array<uint32_t, MAX_MOVES>& move_list, int& move_count, int depth, bool white, uint32_t move_hint);

	// Generate noisy moves sorted with endgame heuristic
	// Noisy moves are captures and promotions + all check moves
	void generateEndgameNoisyMoves(std::array<uint32_t, MAX_MOVES>& move_list, int& move_count, bool white);

	// Function for ChessAI to apply the move
	// Takes the encoded move as a parameter and applies it to the board
    // Also saves the en passant target and castling rights before applying move for later undoign
	void applyMoveAI(uint32_t move, bool white);

	// Function for ChessAI to undo the move
	// Takes the encoded move as a parameter and undoes it
	void undoMoveAI(uint32_t move, bool white);

    // Function to assign a score to the board
	// Used for evaluation of the board state in midgame
	int evaluateBoard();

    // Score king safety in the midgame
    // Bonus for non-open king file and pawn shields
    // +white safety, -black safety
    int evaluateKingSafety();

	// Function to check if the game is over
	// Checkmate or stalemate for either side
	bool isGameOver();

    // Check for repetitions by threefold rule
    bool isDrawByRepetition();

    // Used in quiescence search for delta pruning noisy moves
    int estimateCaptureValue(uint32_t move);
    int estimateEndgameCaptureValue(uint32_t move, bool white); // Bonus for passed pawn capturing

    // Get distance between kings
    // Used in endgame eval heuristic
    // Closer kings get higher bonus
    int calculateKingDistance();

    // Get king distance from center squares (d4,e4,d5,e5)
    // Critical in endgame evaluations
    int getKingCentralization();

    // Evaluate passed pawns
    // Critical in endgame evaluations
    int evaluatePassedPawns(bool white);

private: 
	// Helper to get correct move type depending on the target square and piece type
	// Used for encoding moves
	MoveType getMoveType(int source_square, int target_square, PieceType piece, PieceType target_piece, bool white) const;

    // Calculate positional score of a piece
    inline int getPositionalScore(int square, float game_phase,  PieceType piece, bool white);

    // Helper to determine if a pawn if passed
    // Passed pawns = pawns with no opposing pawns blocking their promoting path
    bool isPassedPawn(int pawn, bool white);

    // Compute whether move gets the enemy king in check
    bool isCheckMove(const KingDanger& king_danger, int to, PieceType piece);

    int evaluateSingleKingSafety(int king_sq, bool white);
};

#endif BITBOARD_H