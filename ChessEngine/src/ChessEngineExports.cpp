#include "pch.h"
#include "ChessEngineExports.hpp"
#include "ChessBoard.hpp"
#include "MoveTables.hpp"
#include "Tables.hpp"

extern "C" CHESSENGINE_API void* CreateBoard() {
    // Init once, safely
    MoveTables::initMoveTables();
    Tables::initTables();

    return new ChessBoard(); // Return a pointer to the new Board object
}

extern "C" CHESSENGINE_API void DestroyBoard(void* board) {
    if (board) {
        delete static_cast<ChessBoard*>(board); // Cast the void* back to ChessBoard* and delete it
    }

    // Teardown after use
    Tables::teardownTables();
    MoveTables::teardownMoveTables();
}

extern "C" CHESSENGINE_API uint64_t ValidMoves(void* board, int square) {
    if (!board) return 0ULL; // Prevent crashes
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* to ChessBoard*
    return b->LegalMoves(square); // Return moves
}

extern "C" CHESSENGINE_API void MakeMove(void* board, int source, int target, char promotion) {
    if (!board) return; // Prevent crashes
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* to ChessBoard*
    b->MovePiece(source, target, promotion); // Apply move 
}

extern "C" CHESSENGINE_API void MakeBestMove(void* board, int depth, bool white) {
    if (!board) return; // Prevent crashes
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* to ChessBoard*
    b->MakeMoveAI(depth, white); // Apply move
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