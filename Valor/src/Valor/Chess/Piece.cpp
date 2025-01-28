#include "vlpch.h"
#include "Valor/Chess/Piece.h"

namespace Valor {

	char Piece::PieceTypeToChar(PieceType type)
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

}
