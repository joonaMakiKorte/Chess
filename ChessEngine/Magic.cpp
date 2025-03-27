#include "pch.h"
#include "Magic.h"
#include "Utils.h"

// Static allocation of tables
// Declared as global arrays
MagicMoves MAGIC_TABLE_BISHOP[64];
MagicMoves MAGIC_TABLE_ROOK[64];

static uint64_t maskBishopAttackRays(int square) {
    uint64_t attacks = 0ULL;
    
    // Get rank and file as bitmasks
    uint64_t rank = Utils::getRank(square);
    uint64_t file = Utils::getFile(square);

    // Generate attack rays
    for (int r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++) {
        attacks |= (1ULL << Utils::getSquare(r, f));
    }

    for (int r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--) {
        attacks |= (1ULL << Utils::getSquare(r, f));
    }

    for (int r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++) {
        attacks |= (1ULL << Utils::getSquare(r, f));
    }

    for (int r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--) {
        attacks |= (1ULL << Utils::getSquare(r, f));
    }

    return attacks;
}

static uint64_t maskRookAttackRays(int square) {
    uint64_t attacks = 0ULL;

    // Get rank and file as bitmasks
    uint64_t rank = Utils::getRank(square);
    uint64_t file = Utils::getFile(square);

    // Generate attack rays
    for (int r = rank + 1; r < 7; r++) {
        attacks |= (1ULL << Utils::getSquare(r, file));
    }

    for (int r = rank - 1; r > 0; r--) {
        attacks |= (1ULL << Utils::getSquare(r, file));
    }

    for (int f = file + 1; f < 7; f++) {
        attacks |= (1ULL << Utils::getSquare(rank, f));
    }

    for (int f = file - 1; f > 0; f--) {
        attacks |= (1ULL << Utils::getSquare(rank, f));
    }

    return attacks;
}

static uint64_t maskBishopXrayAttacks(int square, uint64_t blockers) {
    uint64_t attacks = 0ULL;

    // Get rank and file as bitmasks
    uint64_t rank = Utils::getRank(square);
    uint64_t file = Utils::getFile(square);

    // Generate xrays, stop traversing a ray after encountering a blocker
    for (int r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++) {
        attacks |= (1ULL << Utils::getSquare(r, f));
        if ((1ULL << Utils::getSquare(r, f) & blockers)) break;
    }

    for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--) {
        attacks |= (1ULL << Utils::getSquare(r, f));
        if ((1ULL << Utils::getSquare(r, f) & blockers)) break;
    }

    for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++) {
        attacks |= (1ULL << Utils::getSquare(r, f));
        if ((1ULL << Utils::getSquare(r, f) & blockers)) break;
    }

    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
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
    for (int r = rank + 1; r < 8; r++) {
        attacks |= (1ULL << Utils::getSquare(r, file));
        if ((1ULL << Utils::getSquare(r, file) & blockers)) break;
    }

    for (int r = rank - 1; r >= 0; r--) {
        attacks |= (1ULL << Utils::getSquare(r, file));
        if ((1ULL << Utils::getSquare(r, file) & blockers)) break;
    }

    for (int f = file + 1; f < 8; f++) {
        attacks |= (1ULL << Utils::getSquare(rank, f));
        if ((1ULL << Utils::getSquare(rank, f) & blockers)) break;
    }

    for (int f = file - 1; f >= 0; f--) {
        attacks |= (1ULL << Utils::getSquare(rank, f));
        if ((1ULL << Utils::getSquare(rank, f) & blockers)) break;
    }

    return attacks;
}


static uint64_t generateMagicNumber(int square, int relevant_bits, uint64_t(*maskAttacks)(int), uint64_t(*maskXrayAttacks)(int, uint64_t)) {
    int occupancy_indices = 1 << relevant_bits;
    uint64_t attack_mask = maskAttacks(square);
    uint64_t occupancies[4096], attacks[4096], used_attacks[4096];

    for (int i = 0; i < occupancy_indices; i++) {
        occupancies[i] = Utils::setOccupancy(i, relevant_bits, attack_mask);
        attacks[i] = maskXrayAttacks(square, occupancies[i]);
    }

    for (int max_tries = 0; max_tries < 99999999; max_tries++)
    {
        uint64_t candidate = Utils::getMagicNumberCandidate();

        if (Utils::countSetBits((attack_mask * candidate) & 0xFF00000000000000) < 6) continue;

        memset(used_attacks, 0ULL, sizeof(used_attacks));

        int fail = 0;
        for (int index = 0; !fail && index < occupancy_indices; index++) {
            int magic_index = (int)((occupancies[index] * candidate) >> (64 - relevant_bits));
            if (used_attacks[magic_index] == 0ULL) {
                used_attacks[magic_index] = attacks[index];
            }
            else {
                fail = 1;
                break;
            }
        }

        if (!fail) {
            return candidate;
        }
    }

    return 0ULL;
}

void generateMagicTables() {

    for (int sq = 0; sq < 64; sq++) {
        int bit_count = RELEVANT_BITS_COUNT_ROOK[sq];
        uint64_t magic = generateMagicNumber(sq, bit_count, &maskRookAttackRays, &maskRookXrayAttacks);
        printf("%d : 0x%lxULL\n", sq, magic);
    }
    std::cout << std::endl;

    std::cout << "Bishop Magic Numbers" << std::endl;
    for (int sq = A1; sq < N_SQUARES; sq++)
    {
        int bit_count = tables::get_relevant_bits_count_bishop((Square)sq);
        u64 magic = generate_magic_number((Square)sq, bit_count, &attacks::mask_bishop_attack_rays, &attacks::mask_bishop_xray_attacks);
        printf("%d : 0x%lxULL\n", sq, magic);
    }
    std::cout << std::endl;
}

void initMagicTables()
{
    for (int sq = A1; sq < N_SQUARES; sq++)
    {
        MAGIC_TABLE_BISHOP[sq].mask = attacks::mask_bishop_attack_rays((Square)sq);
        MAGIC_TABLE_BISHOP[sq].magic = MAGICS_BISHOP[sq];
        MAGIC_TABLE_BISHOP[sq].shift = 64 - tables::get_relevant_bits_count_bishop((Square)sq);
    }

    for (int sq = A1; sq < N_SQUARES; sq++)
    {
        MAGIC_TABLE_ROOK[sq].mask = attacks::mask_rook_attack_rays((Square)sq);
        MAGIC_TABLE_ROOK[sq].magic = MAGICS_ROOK[sq];
        MAGIC_TABLE_ROOK[sq].shift = 64 - tables::get_relevant_bits_count_rook((Square)sq);
    }
}
