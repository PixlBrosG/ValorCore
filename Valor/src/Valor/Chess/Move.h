#pragma once

#include "Valor/Chess/Tile.h"

#include <string>

namespace Valor {

	struct Move
	{
		uint32_t Data; // [6] source, [6] target, [4] flags, [4] promotion, [4] pieceType, [4] capturedPiece, [4] reserved

		Move(Tile source, Tile target, uint8_t flags = 0, uint8_t promotion = 0, uint8_t pieceType = 0, uint8_t capturedPiece = 0);

		// Getters
		Tile GetSource() const { return Tile((Data >> 26) & 0b111111); }
		Tile GetTarget() const { return Tile((Data >> 20) & 0b111111); }
		uint8_t GetFlags() const { return (Data >> 16) & 0b1111; }
		uint8_t GetPromotion() const { return (Data >> 12) & 0b1111; }
		uint8_t GetPieceType() const { return (Data >> 8) & 0b1111; }
		uint8_t GetCapturedPiece() const { return (Data >> 4) & 0b1111; }

		// Flags
		bool IsCapture() const { return GetFlags() & captureFlag; }
		bool IsEnPassant() const { return GetFlags() & enPassantFlag; }
		bool IsCastling() const { return GetFlags() & castlingFlag; }
		bool IsPromotion() const { return GetFlags() & promotionFlag; }

		// Flags definition
		constexpr static uint8_t captureFlag = 0b0001;
		constexpr static uint8_t enPassantFlag = 0b0010;
		constexpr static uint8_t castlingFlag = 0b0100;
		constexpr static uint8_t promotionFlag = 0b1000;

		// Metadata sizes
		constexpr static uint8_t sourceBits = 6;
		constexpr static uint8_t targetBits = 6;
		constexpr static uint8_t flagsBits = 4;
		constexpr static uint8_t promotionBits = 4;
		constexpr static uint8_t pieceTypeBits = 4;
		constexpr static uint8_t capturedPieceBits = 4;
		constexpr static uint8_t reservedBits = 4;

		// Conversion methods
		std::string ToAlgebraic() const;
		static Move FromAlgebraic(const std::string& algebraic);
	};

}
