#include "pch.h"
#include "MoveFiltering.h"
#include "Utils.h"
#include "MoveTables.h"

void MoveFiltering::computePinnedPieces(Bitboard::PinData& pin_data, const int& king_sq,
	const uint64_t& occupied, const uint64_t& bishops, const uint64_t& rooks, const uint64_t& queen) {
	// Reset pin data
	pin_data.pinned = 0;
	for (int i = 0; i < 64; i++) {
		pin_data.pin_rays[i] = 0xFFFFFFFFFFFFFFFFULL;  // Default to allow all moves
	}

	// All potential pinners
	uint64_t sliders = bishops | rooks | queen;

	// Get all rays from the square king is at
	// Meaning possible rays enemy could attack from
	// Done by getting possible queen moves at this square
	while (sliders) {
		int slider_sq = Utils::findFirstSetBit(sliders);
		Utils::popBit(sliders, slider_sq);
		Direction direction = DIR[king_sq][slider_sq];

		if (!direction) continue; // Not aligned

		uint64_t between_mask = BETWEEN[king_sq][slider_sq]; // Bitmask of squares between king and slider
		uint64_t blockers = between_mask & occupied; // Get pieces that align with between mask

		// Check if exactly one blocker exists and extract it in one step
		if (Utils::countSetBits(blockers) == 1) {
			int pinned_sq = Utils::findFirstSetBit(blockers); // Get the pinned piece
			pin_data.pinned |= (1ULL << pinned_sq); // Add to pinned
			pin_data.pin_rays[pinned_sq] = LINE[king_sq][slider_sq]; // Get the pin ray
		}
	}
}
