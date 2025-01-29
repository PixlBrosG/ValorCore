#include "vlpch.h"
#include "Valor/Chess/Move.h"

#include "Valor/Chess/Piece.h"

#include <sstream>

namespace Valor {

	std::string Move::ToAlgebraic() const
	{
		std::stringstream result;
		result << Source.ToAlgebraic() << Target.ToAlgebraic();
		return result.str();
	}

	std::string MoveInfo::ToAlgebraic() const
	{
		if (IsCastling()) {
			return Target.GetFile() == 2 ? "O-O-O" : "O-O";
		}
		if (IsEnPassant()) {
			return Target.ToAlgebraic() + " e.p.";
		}

		std::stringstream result;

		PieceType pieceType = static_cast<PieceType>(Piece);
		if (pieceType == PieceType::Pawn)
		{
			if (IsCapture())
				result << Source.FileAlgebraic() << 'x';
			result << Target.ToAlgebraic();

			if (IsPromotion()) {
				result << '=' + Piece::PieceTypeToChar(Promotion);
			}
		}
		else {
			result << Piece::PieceTypeToChar(pieceType);
			if (IsCapture()) {
				result << "x";
			}
			result << Target.ToAlgebraic();
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
