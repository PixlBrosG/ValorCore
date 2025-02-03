#include "vlpch.h"
#include "Valor/Engine/Minimax.h"

#include "Valor/Chess/MoveGeneration/MoveGeneratorSimple.h"

namespace Valor::Engine {

	Move Minimax::FindBestMove(Board& board, int maxDepth, Evaluator* evaluator)
	{
		m_MaxDepth = maxDepth;
		m_Evaluator = evaluator;
		m_BestMove = Move(Tile::None, Tile::None);

		constexpr int alpha = std::numeric_limits<int>::min();
		constexpr int beta = std::numeric_limits<int>::max();

		Run(board, maxDepth, alpha, beta, board.IsWhiteTurn());

		return m_BestMove;
	}

	int Minimax::Run(Board& board, int depth, int alpha, int beta, bool isMaximizing)
	{
		if (depth == 0)
			return Evaluate(board);

		int bestValue = isMaximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

		std::vector<Move> moves = MoveGeneratorSimple::GenerateLegalMoves(board);

		if (moves.empty())
		{
			if (board.IsCheck(true))
				return isMaximizing ? std::numeric_limits<int>::min() + (m_MaxDepth - depth)
				: std::numeric_limits<int>::max() - (m_MaxDepth - depth);
			else
				return 0; // Stalemate (Draw)
		}

		for (const Move& move : moves)
		{
			Board tempBoard = board;
			tempBoard.MakeMove(move);

			int value = Run(tempBoard, depth - 1, alpha, beta, !isMaximizing);
			
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
