#include "vlpch.h"
#include "Valor/Engine/Evaluator/Evaluator.h"

#include <bit>

namespace Valor::Engine {

	int PieceValueEvaluator::Evaluate(const Board& board)
	{
		if (board.IsCheckmate())
			return board.IsWhiteTurn() ? -MateScore : MateScore;
		else if (board.IsStalemate())
			return 0;

		int score = 0;

		uint64_t allWhite = board.WhitePieces();
		uint64_t allBlack = board.BlackPieces();

		uint64_t pawns = board.Pawns();
		score += PawnValue * (std::popcount(pawns & allWhite) - std::popcount(pawns & allBlack));

		uint64_t knights = board.Knights();
		score += KnightValue * (std::popcount(knights & allWhite) - std::popcount(knights & allBlack));

		uint64_t bishops = board.Bishops();
		score += BishopValue * (std::popcount(bishops & allWhite) - std::popcount(bishops & allBlack));

		uint64_t rooks = board.Rooks();
		score += RookValue * (std::popcount(rooks & allWhite) - std::popcount(rooks & allBlack));

		uint64_t queens = board.Queens();
		score += QueenValue * (std::popcount(queens & allWhite) - std::popcount(queens & allBlack));

		return score;
	}

}
