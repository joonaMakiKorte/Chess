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

extern "C" CHESSENGINE_API uint64_t ValidMoves(void* board, int square) {
    if (!board) return 0ULL; // Prevent crashes
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* to ChessBoard*
    return b->LegalMoves(square); // Return moves
}

extern "C" CHESSENGINE_API void MakeMove(void* board, int source, int target) {
    if (!board) return; // Prevent crashes
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* to ChessBoard*
    b->MovePiece(source, target); // Apply move 
}

extern "C" CHESSENGINE_API void GetBoardState(void* board, char* output, int size) {
    if (!board) return; // Prevent crashes if the board is null
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* back to ChessBoard*
    std::string state = b->GetBoardState(); // Get board state as FEN string
    strncpy_s(output, size, state.c_str(), _TRUNCATE); // Convert state to C-style string and copy into the output buffer 
}

extern "C" CHESSENGINE_API void GetDebugMessage(void* board, char* output, int size) {
    if (!board) return; // Prevent crashes if the board is null
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* to ChessBoard*
    std::string debug_message = b->GetDebugMessage(); // Store the debug message in a static variable
    strncpy_s(output, size, debug_message.c_str(), _TRUNCATE); // Copy into the output buffer
}


extern "C" CHESSENGINE_API bool isInCheck(void* board) {
    if (!board) return false;
    ChessBoard* b = static_cast<ChessBoard*>(board);
    return b->isInCheck();
}

extern "C" CHESSENGINE_API bool isCheckmate(void* board) {
    if (!board) return false;
    ChessBoard* b = static_cast<ChessBoard*>(board);
    return b->isCheckmate();
}