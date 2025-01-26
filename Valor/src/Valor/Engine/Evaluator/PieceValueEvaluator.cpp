#include "vlpch.h"
#include "Valor/Engine/Evaluator/Evaluator.h"

namespace Valor {

	float PieceValueEvaluator::Evaluate(const Game& game)
	{
		if (game.IsCheckmate())
		{
			return game.GetTurn() == PieceColor::White ? -std::numeric_limits<float>::infinity() : std::numeric_limits<float>::infinity();
		}
		else if (game.IsStalemate())
		{
			return 0.0f;
		}

		float score = 0.0f;

		for (int rank = 0; rank < 8; rank++)
		{
			for (int file = 0; file < 8; file++)
			{
				Piece piece = game.GetBoard().GetPiece(rank, file);
				if (piece.Color == PieceColor::None)
					continue;
				float value = 0.0f;
				switch (piece.Type)
				{
				case PieceType::Pawn:
					value = 1.0f;
					break;
				case PieceType::Knight:
					value = 3.0f;
					break;
				case PieceType::Bishop:
					value = 3.0f;
					break;
				case PieceType::Rook:
					value = 5.0f;
					break;
				case PieceType::Queen:
					value = 9.0f;
					break;
				case PieceType::King:
					value = 0.0f;
					break;
				}
				score += piece.Color == PieceColor::White ? value : -value;
			}
		}

		return score;
	}

}
