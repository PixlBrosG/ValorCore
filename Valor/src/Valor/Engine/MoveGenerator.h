#pragma once

#include "Valor/Chess/Move.h"
#include "Valor/Chess/Board.h"

#include "Valor/Engine/SearchState.h"

namespace Valor::Engine::MoveGenerator {

	std::vector<Move> GenerateOrderedMoves(const Board& board, const SearchState& searchState);
	std::vector<Move> GeneratePseudoLegalMoves(const Board& board);

	std::vector<Move> GenerateCaptureMoves(const Board& board);
	std::vector<Move> GenerateQuietMoves(const Board& board);
	std::vector<Move> GenerateCheckMoves(const Board& board);

}
