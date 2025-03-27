#ifndef MOVEFILTERING_H
#define MOVEFILTERING_H

#include "Bitboard.h"

class MoveFiltering {
public:
	void computePinnedPieces(Bitboard::PinData& pin_data, const int& king, const uint64_t& occupied,
		const uint64_t& bishops, const uint64_t& rooks, const uint64_t& queen);
};

#endif