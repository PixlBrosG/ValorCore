#include "vlpch.h"
#include "Valor/Chess/Tile.h"

namespace Valor {

	const Tile Tile::None{ 255 };

	std::string Tile::ToAlgebraic() const
	{
		std::string result;
		result += static_cast<char>('A' + File());
		result += std::to_string(Rank() + 1);
		return result;
	}

	Tile Tile::FromAlgebraic(const std::string& algebraic)
	{
		uint8_t file = algebraic[0] - 'a';
		uint8_t rank = algebraic[1] - '1';
		return Tile(rank, file);
	}

}
