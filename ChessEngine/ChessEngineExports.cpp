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
    if (!board || !move) return false; // Prevent crashes
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* to ChessBoard*

    std::string debugMessage = "Validating move: ";
    debugMessage += move;

    // Update the static debug message with selected squares
    b->UpdateDebugMessage(debugMessage); // Assuming this method updates the debug message in ChessBoard

    return b->ValidateMove(move); // Return move validity
}

extern "C" CHESSENGINE_API void GetBoardState(void* board, char* output, int size) {
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* back to ChessBoard*
    std::string state = b->GetBoardState(); // Get board state as FEN string
    strncpy_s(output, size, state.c_str(), _TRUNCATE); // Convert state to C-style string and copy into the output buffer 
}

extern "C" CHESSENGINE_API const char* GetDebugMessage(void* board) {
    if (!board) return nullptr; // Prevent crashes if the board is null
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* to ChessBoard*
    static std::string debugMessage = b->GetDebugMessage(); // Store the debug message in a static variable
    return debugMessage.c_str(); // Return the C-style string
}