#pragma once

#include "Valor/Chess/Board.h"

#include <cstdint>

namespace Valor::ZobristHasher {

	uint64_t Hash(const Board& board);
	void UpdateHash(uint64_t& hash, const Board& board, Move move);

}
