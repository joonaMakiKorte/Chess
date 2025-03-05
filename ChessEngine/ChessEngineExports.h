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
    CHESSENGINE_API void DestroyBoard(void* board);
}

#endif // CHESSENGINEEXPORTS_H