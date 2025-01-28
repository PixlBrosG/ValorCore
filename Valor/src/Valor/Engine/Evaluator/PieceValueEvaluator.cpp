#include "vlpch.h"
#include "Valor/Engine/Evaluator/Evaluator.h"

#include <bit>

namespace Valor {

	float PieceValueEvaluator::Evaluate(const Board& board)
	{
		if (board.IsCheckmate(SWAP_COLOR(board.GetTurn())))
			return board.GetTurn() == PieceColor::White ? -std::numeric_limits<float>::infinity() : std::numeric_limits<float>::infinity();
		else if (board.IsStalemate(SWAP_COLOR(board.GetTurn())))
			return 0.0f;

		float score = 0.0f;

		uint64_t allWhite = board.AllPieces(PieceColor::White);

		uint64_t pawns = board.Pawns();
		while (pawns)
		{
			int square = std::countr_zero(pawns);
			score += 1.0f * (allWhite & (1ULL << square) ? 1 : -1);
			pawns &= pawns - 1;
		}

		uint64_t knightsAndBishops = board.Knights() | board.Bishops();
		while (knightsAndBishops)
		{
			int square = std::countr_zero(knightsAndBishops);
			score += 3.0f * (allWhite & (1ULL << square) ? 1 : -1);
			knightsAndBishops &= knightsAndBishops - 1;
		}

		uint64_t rooks = board.Rooks();
		while (rooks)
		{
			int square = std::countr_zero(rooks);
			score += 5.0f * (allWhite & (1ULL << square) ? 1 : -1);
			rooks &= rooks - 1;
		}

		uint64_t queens = board.Queens();
		while (queens)
		{
			int square = std::countr_zero(queens);
			score += 9.0f * (allWhite & (1ULL << square) ? 1 : -1);
			queens &= queens - 1;
		}

		return score;
	}

}
