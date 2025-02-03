#include "vlpch.h"
#include "Valor/Engine/ValorEngine.h"

#include "Valor/Engine/Minimax.h"

namespace Valor::Engine {

	ValorEngine::ValorEngine()
	{
	}

	Move ValorEngine::SearchBestMove(const Board& board, int depth)
	{
		Minimax minimax;
		std::unique_ptr<Evaluator> evaluator = std::make_unique<PositionalEvaluator>();

		return minimax.FindBestMove(const_cast<Board&>(board), depth, evaluator.get());
	}

	int ValorEngine::AlphaBeta(const Board& board, int alpha, int beta, int depth, bool isMaximizing)
	{
		return 0;
	}

	void ValorEngine::UpdateKillerMoves(Move move, int depth)
	{

	}

	void ValorEngine::UpdateHistoryHeuristics(Move move, int depth)
	{

	}

}
