#pragma once

#include "Valor/Chess/Game.h"
#include "Valor/Engine/Evaluator/Evaluator.h"

namespace Valor::Engine {

	class Minimax
	{
	public:
		Minimax(bool useAlphaBeta = true)
			: m_UseAlphaBeta(useAlphaBeta), m_MaxDepth(3), m_Evaluator(nullptr) {}

		Move FindBestMove(Game game, int maxDepth, Evaluator* evaluator);
	private:
		bool m_UseAlphaBeta;
		int m_MaxDepth;
		Evaluator* m_Evaluator;
		Move m_BestMove = Move(Tile::None, Tile::None);

		float MinimaxWithoutAlphaBeta(Game& game, int depth, bool isMaximizing);
		float MinimaxWithAlphaBeta(Game& game, int depth, float alpha, float beta, bool isMaximizing);

		float Evaluate(const Game& game) const { return m_Evaluator->Evaluate(game.GetBoard()); }
	};

}
