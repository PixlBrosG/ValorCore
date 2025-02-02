#pragma once

#include "Valor/Chess/Game.h"
#include "Valor/Engine/TranspositionTable.h"
#include "Valor/Engine/SearchState.h"

namespace Valor::Engine {

	class ValorEngine
	{
	public:
		ValorEngine();
		
		Move SearchBestMove(const Board& board, int depth);
	private:
		float AlphaBeta(const Board& board, float alpha, float beta, int depth, bool isMaximizing);
		void UpdateKillerMoves(Move move, int depth);
		void UpdateHistoryHeuristics(Move move, int depth);

		TranspositionTable m_TranspositionTable;
		HistoryHeuristics m_HistoryHeuristics;
		KillerMoves m_KillerMoves;
	};

}
