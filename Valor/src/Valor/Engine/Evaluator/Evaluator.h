#pragma once

#include "Valor/Chess/Board.h"

namespace Valor::Engine {

	constexpr int PawnValue = 100;
	constexpr int KnightValue = 300;
	constexpr int BishopValue = 320;
	constexpr int RookValue = 500;
	constexpr int QueenValue = 900;

	constexpr int MateScore = 100000;

	class Evaluator
	{
	public:
		virtual ~Evaluator() = default;

		virtual int Evaluate(const Board& board) = 0;
	};

	class PieceValueEvaluator : public Evaluator
	{
	public:
		virtual ~PieceValueEvaluator() = default;

		virtual int Evaluate(const Board& board) override;
	};

}
