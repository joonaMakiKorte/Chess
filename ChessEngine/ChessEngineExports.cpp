#include "ChessEngineExports.h"
#include "ChessBoard.h"

// Create and initialize the board
extern "C" CHESSENGINE_API void* CreateBoard() {
    return new ChessBoard(); // Return a pointer to the new Board object
}

// Destroy the board and free memory
extern "C" CHESSENGINE_API void DestroyBoard(void* board) {
    if (board) {
        delete static_cast<ChessBoard*>(board); // Cast the void* back to Board* and delete it
    }
}