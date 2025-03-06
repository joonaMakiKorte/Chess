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

    // Validate moves and returns a boolean
    // Takes a void pointer to the board and the move as a const char pointer as params
    CHESSENGINE_API bool ValidateMove(void* board, const char* move);

    // Retrieve the current state of the chessboard and return it as a FEN string
    // Takes a void pointer to the board, pointer to char array to store the state (output buffer)
    // and size of the output buffer
    CHESSENGINE_API void GetBoardState(void* board, char* output, int size);
}

#endif // CHESSENGINEEXPORTS_H