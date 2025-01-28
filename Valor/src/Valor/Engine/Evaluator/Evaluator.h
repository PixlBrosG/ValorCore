#pragma once

#include "Valor/Chess/Board.h"

namespace Valor {

	class Evaluator
	{
	public:
		virtual ~Evaluator() = default;

		virtual float Evaluate(const Board& board) = 0;
	};

	class PieceValueEvaluator : public Evaluator
	{
	public:
		virtual ~PieceValueEvaluator() = default;

		virtual float Evaluate(const Board& board) override;
	};

}
