#pragma once

#include "Valor/Chess/Board.h"

#include "Valor/Engine/Evaluator/Evaluator.h"

namespace Valor::Engine {

	class Minimax
	{
	public:
		Minimax()
			: m_MaxDepth(1), m_Evaluator(nullptr) {}

		Move FindBestMove(Board& board, int maxDepth, Evaluator* evaluator);
	private:
		int m_MaxDepth;
		Evaluator* m_Evaluator;

		Move m_BestMove = Move(Tile::None, Tile::None);
		int m_BestValue = 0;

		int Run(Board& board, int depth, int alpha, int beta, bool isMaximizing);

		int Evaluate(const Board& board) const { return m_Evaluator->Evaluate(board); }
	};

}
