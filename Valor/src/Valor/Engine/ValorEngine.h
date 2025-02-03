#pragma once

#include "Valor/Chess/Board.h"
#include "Valor/Engine/TranspositionTable.h"
#include "Valor/Engine/SearchState.h"

namespace Valor::Engine {

	class ValorEngine
	{
	public:
		ValorEngine();
		
		Move SearchBestMove(const Board& board, int depth);
	private:
		int AlphaBeta(const Board& board, int alpha, int beta, int depth, bool isMaximizing);
		void UpdateKillerMoves(Move move, int depth);
		void UpdateHistoryHeuristics(Move move, int depth);

		TranspositionTable m_TranspositionTable;
		HistoryHeuristics m_HistoryHeuristics;
		KillerMoves m_KillerMoves;
	};

}
