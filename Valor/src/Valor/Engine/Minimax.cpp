#include "vlpch.h"
#include "Valor/Engine/Minimax.h"

#include "Valor/Chess/MoveGeneration/MoveGenerator.h"

namespace Valor::Engine {

	Move Minimax::FindBestMove(Game game, int maxDepth, Evaluator* evaluator)
	{
		m_MaxDepth = maxDepth;
		m_Evaluator = evaluator;
		m_BestMove = Move(Tile::None, Tile::None);

		if (m_UseAlphaBeta)
		{
			constexpr float alpha = -std::numeric_limits<float>::infinity();
			constexpr float beta = std::numeric_limits<float>::infinity();

			MinimaxWithAlphaBeta(game, maxDepth, alpha, beta, game.GetBoard().IsWhiteTurn());
		}
		else
		{
			MinimaxWithoutAlphaBeta(game, maxDepth, game.GetBoard().IsWhiteTurn());
		}

		return m_BestMove;
	}

	float Minimax::MinimaxWithoutAlphaBeta(Game& game, int depth, bool isMaximizing)
	{
		if (depth == 0)
			return Evaluate(game);

		float bestValue = isMaximizing ? -std::numeric_limits<float>::infinity()
			: std::numeric_limits<float>::infinity();

		auto moves = MoveGenerator::GenerateLegalMoves(game.GetBoard(), game.GetBoard().IsWhiteTurn());
		if (moves.empty()) return isMaximizing ? -std::numeric_limits<float>::infinity()
			: std::numeric_limits<float>::infinity();

		if (depth == m_MaxDepth && moves.size() == 1)
		{
			m_BestMove = moves[0];
			return bestValue;
		}

		for (const Move& move : moves)
		{
			game.MakeMove(move);
			float value = MinimaxWithoutAlphaBeta(game, depth - 1, !isMaximizing);
			game.UndoMove();

			if (isMaximizing)
			{
				if (value > bestValue)
				{
					bestValue = value;
					if (depth == m_MaxDepth)
						m_BestMove = move;
				}
			}
			else
			{
				if (value < bestValue)
				{
					bestValue = value;
					if (depth == m_MaxDepth)
						m_BestMove = move;
				}
			}
		}

		return bestValue;
	}

	float Minimax::MinimaxWithAlphaBeta(Game& game, int depth, float alpha, float beta, bool isMaximizing)
	{
		if (depth == 0)
			return Evaluate(game);
		float bestValue = isMaximizing ? -std::numeric_limits<float>::infinity()
			: std::numeric_limits<float>::infinity();
		auto moves = MoveGenerator::GenerateLegalMoves(game.GetBoard(), game.GetBoard().IsWhiteTurn());
		for (const Move& move : moves)
		{
			game.MakeMove(move);
			float value = MinimaxWithAlphaBeta(game, depth - 1, alpha, beta, !isMaximizing);
			game.UndoMove();
			
			if (isMaximizing)
			{
				if (value > bestValue)
				{
					bestValue = value;
					alpha = std::max(alpha, bestValue);
					if (depth == m_MaxDepth)
						m_BestMove = move;
				}
			}
			else
			{
				if (value < bestValue)
				{
					bestValue = value;
					beta = std::min(beta, bestValue);
					if (depth == m_MaxDepth)
						m_BestMove = move;
				}
			}

			if (beta <= alpha)
				break;
		}

		return bestValue;
	}

}
