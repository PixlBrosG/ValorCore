#pragma once

#include "Valor/Chess/Board.h"
#include "Valor/Chess/Move.h"

namespace Valor {

	class ZobristHasher
	{
	public:
		static size_t Hash(const Board& board);
		static size_t UpdateHash(size_t hash, const Move& move, const Board& board);
	};

}
