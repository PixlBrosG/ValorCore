#pragma once

#include "Valor/Chess/Tile.h"
#include "Valor/Chess/Piece.h"

#include <string>
#include <cstdint>

namespace Valor {

	namespace MoveFlags
	{
		constexpr uint8_t Capture = 0b000001;
		constexpr uint8_t Castling = 0b000010;
		constexpr uint8_t Promotion = 0b000100;
		constexpr uint8_t EnPassant = 0b001000;
		constexpr uint8_t Check = 0b010000;
		constexpr uint8_t Checkmate = 0b100000;
	}

	struct Move
	{
		Tile Source = Tile::None;
		Tile Target = Tile::None;
		PieceType Piece = PieceType::None;
		uint8_t Flags = 0;
		PieceType Promotion = PieceType::None;
		uint8_t Priority = 0;

		Move() = default;
		Move(Tile source, Tile target, PieceType piece = PieceType::None, uint8_t flags = 0, PieceType promotion = PieceType::None, uint8_t priority = 0)
			: Source(source), Target(target), Piece(piece), Flags(flags), Promotion(promotion), Priority(priority)
		{
		}

		// Flags
		bool IsCapture()   const { return Flags & MoveFlags::Capture;   }
		bool IsCastling()  const { return Flags & MoveFlags::Castling;  }
		bool IsPromotion() const { return Flags & MoveFlags::Promotion; }
		bool IsEnPassant() const { return Flags & MoveFlags::EnPassant; }
		bool IsCheck()     const { return Flags & MoveFlags::Check;     }
		bool IsCheckmate() const { return Flags & MoveFlags::Checkmate; }

		std::string ToAlgebraic() const;
		static Move FromAlgebraic(const std::string& algebraic);
	};

	struct MoveInfo : public Move
	{
		PieceType CapturedPiece = PieceType::None;
		uint8_t DisambiguityRank = 0;
		uint8_t DisambiguityFile = 0;

		MoveInfo() = default;
		MoveInfo(Move move)
			: Move(move)
		{
		}

		std::string ToAlgebraic() const;
	};

}
