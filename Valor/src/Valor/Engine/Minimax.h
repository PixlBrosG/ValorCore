#pragma once

#include "Valor/Chess/Game.h"
#include "Valor/Engine/Evaluator/Evaluator.h"

namespace Valor::Engine {

	class Minimax
	{
	public:
		Minimax()
			: m_MaxDepth(1), m_Evaluator(nullptr) {}

		Move FindBestMove(Game game, int maxDepth, Evaluator* evaluator);
	private:
		int m_MaxDepth;
		Evaluator* m_Evaluator;

		Move m_BestMove = Move(Tile::None, Tile::None);
		float m_BestValue = 0.0f;

		float Run(Game& game, int depth, float alpha, float beta, bool isMaximizing);

		float Evaluate(const Board& board) const { return m_Evaluator->Evaluate(board); }
	};

}
