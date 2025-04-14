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

extern "C" CHESSENGINE_API void GetBoardJSON(void* board, char* output, int size) {
    if (!board) return; // Prevent crashes if the board is null

    // Get necessary data for JSON
    ChessBoard* b = static_cast<ChessBoard*>(board); // Cast void* back to ChessBoard*
    std::string fen = b->GetFEN(); // Get board fen
    std::string state = b->GetGameState(); // Get ongoing game state
    std::string prev_move = b->GetPrevMove(); // Get previous applied move

    // Using snprintf for safety: it won't write more than 'size' bytes (including null terminator)
    int written = snprintf(output, size, "{\"move\": \"%s\", \"state\": \"%s\", \"fen\": \"%s\"}",
        prev_move.c_str(), // Use .c_str() to get C-style strings for snprintf
        state.c_str(),
        fen.c_str());

    if (written < 0) {
        // Handle encoding error
        output[0] = '\0';
    }
}