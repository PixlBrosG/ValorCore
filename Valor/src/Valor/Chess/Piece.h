#pragma once

namespace Valor {

	enum class PieceType
	{
		None = 0,
		Pawn,
		Knight,
		Bishop,
		Rook,
		Queen,
		King
	};

	enum class PieceColor
	{
		None = 0,
		White,
		Black
	};

	struct Piece
	{
		PieceType Type;
		PieceColor Color;

		Piece(PieceType type = PieceType::None, PieceColor color = PieceColor::None)
			: Type(type), Color(color) {
		}

		char ToChar() const;

		explicit operator char() const { return ToChar(); }
	};

#define SWAP_COLOR(color) (color == PieceColor::White ? PieceColor::Black : PieceColor::White)
#define BOOL_TO_COLOR(b) (b ? PieceColor::White : PieceColor::Black)

}
