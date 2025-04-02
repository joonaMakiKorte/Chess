#include "pch.h"
#include "Magic.h"
#include "Utils.h"

namespace Magic {
    // Static allocation of tables
    // Declared as global arrays
    MagicMoves MAGIC_TABLE_BISHOP[64];
    MagicMoves MAGIC_TABLE_ROOK[64];

    uint64_t maskBishopAttackRays(int square) {
        uint64_t attacks = 0ULL;

        // Get rank and file as bitmasks
        uint64_t rank = Utils::getRank(square);
        uint64_t file = Utils::getFile(square);

        // Generate attack rays
        for (int r = static_cast<int>(rank) + 1, f = static_cast<int>(file) + 1; r < 7 && f < 7; r++, f++) {
            attacks |= (1ULL << Utils::getSquare(r, f));
        }

        for (int r = static_cast<int>(rank) + 1, f = static_cast<int>(file) - 1; r < 7 && f > 0; r++, f--) {
            attacks |= (1ULL << Utils::getSquare(r, f));
        }

        for (int r = static_cast<int>(rank) - 1, f = static_cast<int>(file) + 1; r > 0 && f < 7; r--, f++) {
            attacks |= (1ULL << Utils::getSquare(r, f));
        }

        for (int r = static_cast<int>(rank) - 1, f = static_cast<int>(file) - 1; r > 0 && f > 0; r--, f--) {
            attacks |= (1ULL << Utils::getSquare(r, f));
        }

        return attacks;
    }

    uint64_t maskRookAttackRays(int square) {
        uint64_t attacks = 0ULL;

        // Get rank and file as bitmasks
        uint64_t rank = Utils::getRank(square);
        uint64_t file = Utils::getFile(square);

        // Generate attack rays
        for (int r = static_cast<int>(rank) + 1; r < 7; r++) {
            attacks |= (1ULL << Utils::getSquare(r, static_cast<int>(file)));
        }

        for (int r = static_cast<int>(rank) - 1; r > 0; r--) {
            attacks |= (1ULL << Utils::getSquare(r, static_cast<int>(file)));
        }

        for (int f = static_cast<int>(file) + 1; f < 7; f++) {
            attacks |= (1ULL << Utils::getSquare(static_cast<int>(rank), f));
        }

        for (int f = static_cast<int>(file) - 1; f > 0; f--) {
            attacks |= (1ULL << Utils::getSquare(static_cast<int>(rank), f));
        }

        return attacks;
    }

    uint64_t maskBishopXrayAttacks(int square, uint64_t blockers) {
        uint64_t attacks = 0ULL;

        // Get rank and file as bitmasks
        uint64_t rank = Utils::getRank(square);
        uint64_t file = Utils::getFile(square);

        // Generate xrays, stop traversing a ray after encountering a blocker
        for (int r = static_cast<int>(rank) + 1, f = static_cast<int>(file) + 1; r < 8 && f < 8; r++, f++) {
            attacks |= (1ULL << Utils::getSquare(r, f));
            if ((1ULL << Utils::getSquare(r, f) & blockers)) break;
        }

        for (int r = static_cast<int>(rank) + 1, f = static_cast<int>(file) - 1; r < 8 && f >= 0; r++, f--) {
            attacks |= (1ULL << Utils::getSquare(r, f));
            if ((1ULL << Utils::getSquare(r, f) & blockers)) break;
        }

        for (int r = static_cast<int>(rank) - 1, f = static_cast<int>(file) + 1; r >= 0 && f < 8; r--, f++) {
            attacks |= (1ULL << Utils::getSquare(r, f));
            if ((1ULL << Utils::getSquare(r, f) & blockers)) break;
        }

        for (int r = static_cast<int>(rank) - 1, f = static_cast<int>(file) - 1; r >= 0 && f >= 0; r--, f--) {
            attacks |= (1ULL << Utils::getSquare(r, f));
            if ((1ULL << Utils::getSquare(r, f) & blockers)) break;
        }

        return attacks;
    }

    uint64_t maskRookXrayAttacks(int square, uint64_t blockers) {
        uint64_t attacks = 0ULL;

        // Get rank and file as bitmasks
        uint64_t rank = Utils::getRank(square);
        uint64_t file = Utils::getFile(square);

        // Generate xrays, stop traversing a ray after encountering a blocker
        for (int r = static_cast<int>(rank) + 1; r < 8; r++) {
            attacks |= (1ULL << Utils::getSquare(r, static_cast<int>(file)));
            if ((1ULL << Utils::getSquare(r, static_cast<int>(file)) & blockers)) break;
        }

        for (int r = static_cast<int>(rank) - 1; r >= 0; r--) {
            attacks |= (1ULL << Utils::getSquare(r, static_cast<int>(file)));
            if ((1ULL << Utils::getSquare(r, static_cast<int>(file)) & blockers)) break;
        }

        for (int f = static_cast<int>(file) + 1; f < 8; f++) {
            attacks |= (1ULL << Utils::getSquare(static_cast<int>(rank), f));
            if ((1ULL << Utils::getSquare(static_cast<int>(rank), f) & blockers)) break;
        }

        for (int f = static_cast<int>(file) - 1; f >= 0; f--) {
            attacks |= (1ULL << Utils::getSquare(static_cast<int>(rank), f));
            if ((1ULL << Utils::getSquare(static_cast<int>(rank), f) & blockers)) break;
        }

        return attacks;
    }

    void initMagicTables() {
        // Set flag to prevent unnecessary reinitialization
        static bool initialized = false;
        if (initialized) return;
        initialized = true;

        // Init bishop
        for (int sq = 0; sq < 64; sq++) {
            MAGIC_TABLE_BISHOP[sq].mask = maskBishopAttackRays(sq);
            MAGIC_TABLE_BISHOP[sq].magic = MAGICS_BISHOP[sq];
            MAGIC_TABLE_BISHOP[sq].shift = 64 - RELEVANT_BITS_COUNT_BISHOP[sq];
        }

        // Init rook
        for (int sq = 0; sq < 64; sq++) {
            MAGIC_TABLE_ROOK[sq].mask = maskRookAttackRays(sq);
            MAGIC_TABLE_ROOK[sq].magic = MAGICS_ROOK[sq];
            MAGIC_TABLE_ROOK[sq].shift = 64 - RELEVANT_BITS_COUNT_ROOK[sq];
        }
    }
}

