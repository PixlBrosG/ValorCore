#pragma once

#include "Valor/Chess/Tile.h"
#include "Valor/Chess/Piece.h"

#include <string>
#include <cstdint>

namespace Valor {

	struct Move
	{
		uint64_t Data;

		Move(Tile source, Tile target, uint16_t flags = 0, PieceType promotion = PieceType::None, PieceType pieceType = PieceType::None, PieceType capturedPiece = PieceType::None,
			uint8_t disambiguityRank = 0, uint8_t disambiguityFile = 0, uint8_t pawnCaptureFile = 0, uint16_t evaluation = 0);

		// Getters
		Tile GetSource() const { return Tile((Data >> 58) & 0b111111); }
		Tile GetTarget() const { return Tile((Data >> 52) & 0b111111); }
		uint16_t GetFlags() const { return (Data >> 46) & 0b111111; }
		PieceType GetPromotion() const { return static_cast<PieceType>((Data >> 42) & 0b1111); }
		PieceType GetPieceType() const { return static_cast<PieceType>((Data >> 38) & 0b1111); }
		PieceType GetCapturedPiece() const { return static_cast<PieceType>((Data >> 34) & 0b1111); }
		uint8_t GetDisambiguityRank() const { return (Data >> 31) & 0b111; }
		uint8_t GetDisambiguityFile() const { return (Data >> 28) & 0b111; }
		uint8_t GetPawnCaptureFile() const { return (Data >> 25) & 0b111; }
		uint16_t GetEvaluation() const { return (Data >> 10) & 0x7FFF; }

		// Flags
		bool IsCapture() const { return GetFlags() & captureFlag; }
		bool IsCastling() const { return GetFlags() & castlingFlag; }
		bool IsPromotion() const { return GetFlags() & promotionFlag; }
		bool IsEnPassant() const { return GetFlags() & enPassantFlag; }
		bool IsCheck() const { return GetFlags() & checkFlag; }
		bool IsCheckmate() const { return GetFlags() & checkmateFlag; }

		// Flags definition
		constexpr static uint16_t captureFlag = 0b000001;
		constexpr static uint16_t castlingFlag = 0b000010;
		constexpr static uint16_t promotionFlag = 0b000100;
		constexpr static uint16_t enPassantFlag = 0b001000;
		constexpr static uint16_t checkFlag = 0b010000;
		constexpr static uint16_t checkmateFlag = 0b100000;

		// Metadata sizes
		constexpr static uint8_t sourceBits = 6;
		constexpr static uint8_t targetBits = 6;
		constexpr static uint8_t flagsBits = 6; // Now includes check and checkmate
		constexpr static uint8_t promotionBits = 4;
		constexpr static uint8_t pieceTypeBits = 4;
		constexpr static uint8_t capturedPieceBits = 4;
		constexpr static uint8_t disambiguityRankBits = 3;
		constexpr static uint8_t disambiguityFileBits = 3;
		constexpr static uint8_t pawnCaptureFileBits = 3;
		constexpr static uint8_t evaluationBits = 15;
		constexpr static uint8_t reservedBits = 10;

		// Conversion methods
		std::string ToAlgebraic() const;
		static Move FromAlgebraic(const std::string& algebraic);
	};

}
