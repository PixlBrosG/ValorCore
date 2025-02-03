#pragma once

#include "Valor/Chess/Move.h"

#include <vector>
#include <cstring> // for memset

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
		TranspositionTable() : m_Entries(TTSize) {} // Allocate on heap

		void Store(uint64_t hash, int score, Move bestMove, int depth, TTEntryFlag flag)
		{
			m_Entries[hash % TTSize] = { hash, score, bestMove, depth, flag };
		}

		TTEntry* Lookup(uint64_t hash)
		{
			TTEntry& entry = m_Entries[hash % TTSize];
			return entry.Hash == hash ? &entry : nullptr;
		}

		void Clear()
		{
			memset(m_Entries.data(), 0, TTSize * sizeof(TTEntry));
		}

	private:
		std::vector<TTEntry> m_Entries; // Heap allocation
	};
}
