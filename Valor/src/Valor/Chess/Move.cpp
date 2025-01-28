#include "vlpch.h"
#include "Valor/Chess/Move.h"

namespace Valor {

	Move::Move(Tile source, Tile target, uint8_t flags, uint8_t promotion, uint8_t pieceType, uint8_t capturedPiece)
	{
		Data = (static_cast<uint32_t>(source) << 26) |
			(static_cast<uint32_t>(target) << 20) |
			(static_cast<uint32_t>(flags) << 16) |
			(static_cast<uint32_t>(promotion) << 12) |
			(static_cast<uint32_t>(pieceType) << 8) |
			(static_cast<uint32_t>(capturedPiece) << 4);
	}

	std::string Move::ToAlgebraic() const
	{
		std::string result;
		result += GetSource().ToAlgebraic();
		result += GetTarget().ToAlgebraic();
		return result;
	}

	Move Move::FromAlgebraic(const std::string& algebraic)
	{
		Tile source = Tile::FromAlgebraic(algebraic.substr(0, 2));
		Tile target = Tile::FromAlgebraic(algebraic.substr(2, 2));
		return Move(source, target);
	}

}
