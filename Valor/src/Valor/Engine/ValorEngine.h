#pragma once

#include "Valor/Chess/Game.h"
#include "Valor/Chess/MoveGeneration/MoveGenerator.h"
#include "Valor/Engine/Evaluator/Evaluator.h"
#include "Valor/Engine/Minimax.h"

namespace Valor::Engine {

	class ValorEngine
	{
	public:
		ValorEngine() = default;
		~ValorEngine() = default;

		void Reset() { m_Game->Init(); }

		void SetGame(std::unique_ptr<Game> game) { m_Game = std::move(game); }
		void SetEvaluator(std::unique_ptr<Evaluator> evaluator) { m_Evaluator = std::move(evaluator); }

		float Evaluate(const Game& game) { return m_Evaluator->Evaluate(game.GetBoard()); }
		
		float Evaluate() { m_Score = Evaluate(*m_Game); return m_Score; }

		void MakeMove(const Move& move) { m_Game->MakeMove(move); }
		std::vector<Move> GenerateMoves() { return MoveGenerator::GenerateLegalMoves(m_Game->GetBoard()); }

		Move FindBestMove(int maxDepth, bool useAlphaBeta = true);

		Game* GetGame() { return m_Game.get(); }
		Evaluator* GetEvaluator() { return m_Evaluator.get(); }
	private:
		std::unique_ptr<Game> m_Game;
		std::unique_ptr<Evaluator> m_Evaluator;
		float m_Score = 0.0f;
	};

}
