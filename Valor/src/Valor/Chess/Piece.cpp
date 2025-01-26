#include "vlpch.h"
#include "Valor/Chess/Piece.h"

namespace Valor {

	char Piece::ToChar() const
	{
		if (Color == PieceColor::None || Type == PieceType::None)
			return ' ';

		char pieceChar = ' ';
		switch (Type)
		{
			case PieceType::Pawn: pieceChar = 'P'; break;
			case PieceType::Knight: pieceChar = 'N'; break;
			case PieceType::Bishop: pieceChar = 'B'; break;
			case PieceType::Rook: pieceChar = 'R'; break;
			case PieceType::Queen: pieceChar = 'Q'; break;
			case PieceType::King: pieceChar = 'K'; break;
		}

		return Color == PieceColor::White ? pieceChar : std::tolower(pieceChar);
	}

}
