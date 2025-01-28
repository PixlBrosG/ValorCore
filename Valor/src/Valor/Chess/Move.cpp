#include "vlpch.h"
#include "Valor/Chess/Move.h"

#include "Valor/Chess/Piece.h"

#include <sstream>

namespace Valor {

	Move::Move(Tile source, Tile target, uint16_t flags, PieceType promotion, PieceType pieceType, PieceType capturedPiece,
		uint8_t disambiguityRank, uint8_t disambiguityFile, uint8_t pawnCaptureFile, uint16_t evaluation)
	{
		Data = 0;
		Data |= (static_cast<uint64_t>(source) & 0b111111) << 58;
		Data |= (static_cast<uint64_t>(target) & 0b111111) << 52;
		Data |= (static_cast<uint64_t>(flags) & 0b111111) << 46;
		Data |= (static_cast<uint64_t>(promotion) & 0b1111) << 42;
		Data |= (static_cast<uint64_t>(pieceType) & 0b1111) << 38;
		Data |= (static_cast<uint64_t>(capturedPiece) & 0b1111) << 34;
		Data |= (static_cast<uint64_t>(disambiguityRank) & 0b111) << 31;
		Data |= (static_cast<uint64_t>(disambiguityFile) & 0b111) << 28;
		Data |= (static_cast<uint64_t>(pawnCaptureFile) & 0b111) << 25;
		Data |= (static_cast<uint64_t>(evaluation) & 0x7FFF) << 10;
	}

	std::string Move::ToAlgebraic() const
	{
		Tile target = GetTarget();

		if (IsCastling()) {
			return target.GetFile() == 2 ? "O-O-O" : "O-O";
		}
		if (IsEnPassant()) {
			return target.ToAlgebraic() + " e.p.";
		}

		std::stringstream result;

		PieceType pieceType = static_cast<PieceType>(GetPieceType());
		if (pieceType == PieceType::Pawn)
		{
			if (IsCapture())
				result << target.FileAlgebraic() << 'x';
			result << target.ToAlgebraic();

			if (IsPromotion()) {
				result << '=' + Piece::PieceTypeToChar(static_cast<PieceType>(GetPromotion()));
			}
		}
		else {
			result << Piece::PieceTypeToChar(pieceType);
			if (IsCapture()) {
				result << "x";
			}
			result << target.ToAlgebraic();
		}

		if (IsCheckmate())
			result << '#';
		else if (IsCheck())
			result << '+';

		return result.str();
	}

	Move Move::FromAlgebraic(const std::string& algebraic)
	{
		Tile source = Tile::FromAlgebraic(algebraic.substr(0, 2));
		Tile target = Tile::FromAlgebraic(algebraic.substr(2, 2));
		return Move(source, target);
	}

}
