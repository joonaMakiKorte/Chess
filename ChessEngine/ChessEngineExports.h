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
    CHESSENGINE_API void MakeMove(void* board, int source, int target);

    // Evaluate and execute the best move for black pieces in bitboard
    CHESSENGINE_API void MakeBestMove(void* board, int depth);

    // Retrieve the current state of the chessboard and return it as a FEN string
    // Takes a void pointer to the board, pointer to char array to store the state (output buffer)
    // and size of the output buffer
    CHESSENGINE_API void GetBoardState(void* board, char* output, int size);

    // Export error messages from DLL
    extern "C" CHESSENGINE_API void GetDebugMessage(void* board, char* output, int size);
}

#endif // CHESSENGINEEXPORTS_H