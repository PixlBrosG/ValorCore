#include "vlpch.h"
#include "Valor/Engine/Minimax.h"

#include "Valor/Chess/MoveGeneration/MoveGenerator.h"

namespace Valor::Engine {

	Move Minimax::FindBestMove(Game game, int maxDepth, Evaluator* evaluator)
	{
		m_MaxDepth = maxDepth;
		m_Evaluator = evaluator;
		m_BestMove = Move(Tile::None, Tile::None);

		constexpr float alpha = -std::numeric_limits<float>::infinity();
		constexpr float beta = std::numeric_limits<float>::infinity();

		Run(game, maxDepth, alpha, beta, game.GetBoard().IsWhiteTurn());

		return m_BestMove;
	}

	float Minimax::Run(Game& game, int depth, float alpha, float beta, bool isMaximizing)
	{
		if (depth == 0)
			return Evaluate(game.GetBoard());
		float bestValue = isMaximizing ? -std::numeric_limits<float>::infinity()
			: std::numeric_limits<float>::infinity();
		auto moves = MoveGenerator::GenerateLegalMoves(game.GetBoard(), game.GetBoard().IsWhiteTurn());
		for (const Move& move : moves)
		{
			game.MakeMove(move);
			float value = Run(game, depth - 1, alpha, beta, !isMaximizing);
			game.UndoMove();
			
			if (isMaximizing)
			{
				if (value > bestValue)
				{
					bestValue = value;
					alpha = std::max(alpha, bestValue);
					if (depth == m_MaxDepth)
					{
						m_BestMove = move;
						m_BestValue = bestValue;
					}
				}
			}
			else
			{
				if (value < bestValue)
				{
					bestValue = value;
					beta = std::min(beta, bestValue);
					if (depth == m_MaxDepth)
					{
						m_BestMove = move;
						m_BestValue = bestValue;
					}
				}
			}

			if (beta <= alpha)
				break;
		}

		return bestValue;
	}

}
