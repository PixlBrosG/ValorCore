#pragma once

#include "Valor/Chess/Game.h"

namespace Valor {

	class Evaluator
	{
	public:
		virtual ~Evaluator() = default;

		virtual float Evaluate(const Game& game) = 0;
	};

	class PieceValueEvaluator : public Evaluator
	{
	public:
		virtual ~PieceValueEvaluator() = default;

		virtual float Evaluate(const Game& game) override;
	};

}
