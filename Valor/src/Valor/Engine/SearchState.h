#pragma once

#include "Valor/Chess/Move.h"
#include "Valor/Engine/TranspositionTable.h"

#include <array>

namespace Valor::Engine {

	constexpr int MaxDepth = 8;

	class KillerMoves
	{
	public:
		void StoreKillerMove(Move move, int depth);
		bool IsKillerMove(Move move, int depth) const;

		void Clear();
	private:
		std::array<std::array<Move, 2>, MaxDepth> m_KillerMoves;
	};

	class HistoryHeuristics
	{
	public:
		void UpdateHistory(Move move, int depth);
		int GetHistoryScore(Move move) const;

		void Clear();
	private:
		std::array<std::array<int, 64>, 64> m_History;
	};

	struct SearchState
	{
		KillerMoves KillerMoves;
		HistoryHeuristics HistoryHeuristics;
		TranspositionTable TranspositionTable;
		int Depth;
	};

}
