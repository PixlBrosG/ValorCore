#pragma once

#include "Valor/Chess/Game.h"

#include <vector>

namespace Valor {

	class MoveGenerator
	{
	public:
		static std::vector<Move> GenerateMoves(const Game& game, bool onlyCaptures = false);

		static void GeneratePawnMoves(const Game& game, Tile source, std::vector<Move>& moves, bool onlyCaptures = false);
		static void GenerateKnightMoves(const Game& game, Tile source, std::vector<Move>& moves, bool onlyCaptures = false);
		static void GenerateBishopMoves(const Game& game, Tile source, std::vector<Move>& moves, bool onlyCaptures = false);
		static void GenerateRookMoves(const Game& game, Tile source, std::vector<Move>& moves, bool onlyCaptures = false);
		static void GenerateQueenMoves(const Game& game, Tile source, std::vector<Move>& moves, bool onlyCaptures = false);
		static void GenerateKingMoves(const Game& game, Tile source, std::vector<Move>& moves, bool onlyCaptures = false);

	private:
		static void TryAddMove(const Game& game, Tile source, Tile target, std::vector<Move>& moves, bool onlyCaptures);
	};

}
