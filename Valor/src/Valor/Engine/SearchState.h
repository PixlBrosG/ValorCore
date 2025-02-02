#pragma once

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
		int m_HistoryTable[64][64] = {};
	};

	struct SearchState
	{
		KillerMoves KillerMoves;
		HistoryHeuristics HistoryHeuristics;
		TranspositionTable TranspositionTable;
		int Depth;
	};

}
