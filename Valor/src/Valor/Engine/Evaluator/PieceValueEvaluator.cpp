#include "vlpch.h"
#include "Valor/Engine/Evaluator/Evaluator.h"

#include <bit>

namespace Valor {

	float PieceValueEvaluator::Evaluate(const Board& board)
	{
		if (board.IsCheckmate(false))
			return board.IsWhiteTurn() ? std::numeric_limits<float>::infinity() : -std::numeric_limits<float>::infinity();
		else if (board.IsStalemate(false))
			return 0.0f;

		float score = 0.0f;

		uint64_t allWhite = board.WhitePieces();
		uint64_t allBlack = board.BlackPieces();

		uint64_t pawns = board.Pawns();
		score += std::popcount(pawns & allWhite) - std::popcount(pawns & allBlack);

		uint64_t knightsAndBishops = board.Knights() | board.Bishops();
		score += 3 * (std::popcount(knightsAndBishops & allWhite) - std::popcount(knightsAndBishops & allBlack));

		uint64_t rooks = board.Rooks();
		score += 5 * (std::popcount(rooks & allWhite) - std::popcount(rooks & allBlack));

		uint64_t queens = board.Queens();
		score += 9 * (std::popcount(queens & allWhite) - std::popcount(queens & allBlack));

		return score;
	}

}
