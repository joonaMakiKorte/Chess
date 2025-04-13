#ifndef CHESSENGINEEXPORTS_H
#define CHESSENGINEEXPORTS_H

#ifdef CHESSENGINE_EXPORTS
#define CHESSENGINE_API __declspec(dllexport)
#else
#define CHESSENGINE_API __declspec(dllimport)
#endif

extern "C" {
    // Function to create and initialize the board
    CHESSENGINE_API void* CreateBoard();

    // Function to destroy the board and free memory
    // Takes a void pointer to the board as parameter
    CHESSENGINE_API void DestroyBoard(void* board);

    // Get valid moves from a square as a bitboard
    // Takes a void pointer to the board and the source as a square index
    CHESSENGINE_API uint64_t ValidMoves(void* board, int square);

    // Make move in bitboard
    // Takes a void pointer to the board and the source and target as square indexes
    CHESSENGINE_API void MakeMove(void* board, int source, int target, char promotion);

    // Evaluate and execute the best move for white/black in bitboard
    CHESSENGINE_API void MakeBestMove(void* board, int depth, bool white);

    // Retvieve board state as fen, previous move, and game state, all in one JSON copied to output buffer
    // JSON
    /*
    {
      "move": "e2e4", // The move the engine made
      "state": "ongoing", // The state *after* the move
      "fen": "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1" // FEN *after* the move
    }
    */
    CHESSENGINE_API void GetBoardJSON(void* board, char* output, int size);
}

#endif // CHESSENGINEEXPORTS_H