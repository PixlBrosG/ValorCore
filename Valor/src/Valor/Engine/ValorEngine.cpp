#include "vlpch.h"
#include "Valor/Engine/ValorEngine.h"

namespace Valor::Engine {

	Move ValorEngine::FindBestMove(int maxDepth)
	{
		Minimax minimax;
		Game game = m_Game->CreateSnapshot();

		return minimax.FindBestMove(game, maxDepth, m_Evaluator.get());
	}

}
