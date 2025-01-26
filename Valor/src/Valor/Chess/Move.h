#pragma once

#include "Valor/Chess/Tile.h"
#include "Valor/Chess/Piece.h"

#include <string>
#include <iostream>

namespace Valor {

	enum class MoveStatus
	{
		Normal,
		Check,
		Checkmate,
		Stalemate
	};

	struct Move
	{
		Tile Source, Target;
		PieceType Promotion = PieceType::None;

		MoveStatus Status = MoveStatus::Normal;
		char disambiguationFile = '\0';

		Move() = default;
		Move(Tile source, Tile target, PieceType promotion = PieceType::None)
			: Source(source), Target(target), Promotion(promotion) {}

		static Move FromString(const std::string& moveStr)
		{
			if (moveStr.length() != 4) {
				throw std::invalid_argument("Invalid move format (expected format: e2e4)");
			}

			Tile source = Tile::FromAlgebraic(moveStr.substr(0, 2));
			Tile target = Tile::FromAlgebraic(moveStr.substr(2, 2));

			return Move(source, target);
		}

		operator std::string() const
		{
			std::string moveStr = std::string(1, Source.File + 'a') + std::to_string(8 - Source.Rank)
				+ std::string(1, Target.File + 'a') + std::to_string(8 - Target.Rank);
			return moveStr;
		}
	};

}
