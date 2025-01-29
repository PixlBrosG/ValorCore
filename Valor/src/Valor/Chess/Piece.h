#pragma once

namespace Valor {

	enum class PieceType : unsigned char
	{
		None = 0xFF,
		Pawn = 0,
		Knight = 1,
		Bishop = 2,
		Rook = 3,
		Queen = 4,
		King = 5
	};

	enum class PieceColor : unsigned char
	{
		None = 0xFF,
		White = 0,
		Black = 1
	};

	struct Piece
	{
		PieceType Type;
		PieceColor Color;

		Piece(PieceType type = PieceType::None, PieceColor color = PieceColor::None)
			: Type(type), Color(color) {
		}

		constexpr char ToChar() const { return PieceTypeToChar(Type); }
		
		explicit operator char() const { return ToChar(); }

		constexpr static char PieceTypeToChar(PieceType type);

		constexpr static PieceColor GetOppositeColor(PieceColor color);

		constexpr static bool IsSlidingPiece(PieceType type);
		constexpr static bool IsMinorPiece(PieceType type);
		constexpr static bool IsMajorPiece(PieceType type);
	};

	constexpr char Piece::PieceTypeToChar(PieceType type)
	{
		switch (type)
		{
		case PieceType::Pawn: return 'P';
		case PieceType::Knight: return 'N';
		case PieceType::Bishop: return 'B';
		case PieceType::Rook: return 'R';
		case PieceType::Queen: return 'Q';
		case PieceType::King: return 'K';
		default: return ' ';
		}
	}

	constexpr PieceColor Piece::GetOppositeColor(PieceColor color)
	{
		switch (color)
		{
		case PieceColor::White: return PieceColor::Black;
		case PieceColor::Black: return PieceColor::White;
		default: return PieceColor::None;
		}
	}

	constexpr bool Piece::IsSlidingPiece(PieceType type)
	{
		return type == PieceType::Bishop || type == PieceType::Rook || type == PieceType::Queen;
	}

	constexpr bool Piece::IsMinorPiece(PieceType type)
	{
		return type == PieceType::Knight || type == PieceType::Bishop;
	}

	constexpr bool Piece::IsMajorPiece(PieceType type)
	{
		return type == PieceType::Rook || type == PieceType::Queen;
	}

}
