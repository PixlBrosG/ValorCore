#pragma once

#include "Valor/Chess/Move.h"

namespace Valor::Engine {

	enum class TTEntryFlag : unsigned char
	{
		Exact,      // Searched completely
		LowerBound, // Beta cutoff
		UpperBound  // Alpha cutoff
	};

	struct TTEntry
	{
		uint64_t Hash;
		int Score;
		Move BestMove;
		int Depth;
		TTEntryFlag Flag;
	};

	constexpr size_t TTSize = 1 << 20; // 1M entries

	class TranspositionTable
	{
	public:
		void Store(uint64_t hash, int score, Move bestMove, int depth, TTEntryFlag flag)
		{
			m_Entries[hash % TTSize] = { hash, score, bestMove, depth, flag };
		}

		TTEntry* Lookup(uint64_t hash)
		{
			TTEntry& entry = m_Entries[hash % TTSize];
			return entry.Hash == hash ? &entry : nullptr;
		}

		void Clear() const
		{
			memset((void*)m_Entries, 0, sizeof(m_Entries));
		}
	private:
		TTEntry m_Entries[TTSize];
	};

}
