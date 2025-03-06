#include "pch.h"
#include "ChessEngineExports.h"
#include "ChessBoard.h"

extern "C" CHESSENGINE_API void* CreateBoard() {
    return new ChessBoard(); // Return a pointer to the new Board object
}

extern "C" CHESSENGINE_API void DestroyBoard(void* board) {
    if (board) {
        delete static_cast<ChessBoard*>(board); // Cast the void* back to ChessBoard* and delete it
    }
}

extern "C" CHESSENGINE_API bool ValidateMove(void* board, const char* move) {
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* to ChessBoard*
    return b->ValidateMove(move); // Return move validity
}

extern "C" CHESSENGINE_API void GetBoardState(void* board, char* output, int size) {
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* back to ChessBoard*
    std::string state = b->GetBoardState(); // Get board state as FEN string
    strncpy_s(output, size, state.c_str(), _TRUNCATE); // Convert state to C-style string and copy into the output buffer 
}