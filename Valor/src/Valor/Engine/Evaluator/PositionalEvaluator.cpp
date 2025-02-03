#include "vlpch.h"
#include "Valor/Engine/Evaluator/Evaluator.h"

#include "Valor/Chess/MoveGeneration/MoveGeneratorSimple.h"

#include <vector>

namespace Valor::Engine {

	// Piece-square tables
	constexpr int PawnTable[64] = {
		 0,   5,  10,  20,  20,  10,   5,   0,
		 0,  10,  15,  25,  25,  15,  10,   0,
		 0,   5,  10,  20,  20,  10,   5,   0,
		 0,   0,   0,  15,  15,   0,   0,   0,
		 5,   5,   0, -10, -10,   0,   5,   5,
		 5,  10,  10, -20, -20,  10,  10,   5,
		10,  10,  20, -30, -30,  20,  10,  10,
		 0,   0,   0,   0,   0,   0,   0,   0
	};

	constexpr int KnightTable[64] = {
		-50, -40, -30, -30, -30, -30, -40, -50,
		-40, -20,   0,   0,   0,   0, -20, -40,
		-30,   0,  10,  15,  15,  10,   0, -30,
		-30,   5,  15,  20,  20,  15,   5, -30,
		-30,   0,  15,  20,  20,  15,   0, -30,
		-30,   5,  10,  15,  15,  10,   5, -30,
		-40, -20,   0,   5,   5,   0, -20, -40,
		-50, -40, -30, -30, -30, -30, -40, -50
	};

	constexpr int KingSafetyTable[64] = {
	   -30, -40, -40, -50, -50, -40, -40, -30,
	   -30, -40, -40, -50, -50, -40, -40, -30,
	   -30, -40, -40, -50, -50, -40, -40, -30,
	   -30, -40, -40, -50, -50, -40, -40, -30,
	   -20, -30, -30, -40, -40, -30, -30, -20,
	   -10, -20, -20, -20, -20, -20, -20, -10,
		20,  20,   0,   0,   0,   0,  20,  20,
		20,  30,  10,   0,   0,  10,  30,  20
	};

	// Mobility weight
	constexpr int MobilityWeight = 2;

	// Helper function to get square index from rank/file
	inline int GetSquareIndex(int rank, int file) {
		return rank * 8 + file;
	}

	// Compute mobility score
	int GetMobilityScore(const Board& board, const Valor::Piece& piece, int square)
	{
		std::vector<Move> moves = MoveGeneratorSimple::GenerateMovesForPiece(board, square, piece);
		return moves.size() * MobilityWeight;
	}

	// Evaluate function
	int PositionalEvaluator::Evaluate(const Board& board)
	{
		int score = 0;

		for (int square = 0; square < 64; ++square)
		{
			Valor::Piece piece = board.GetPiece(square);
			if (piece.Type == Valor::PieceType::None) continue;

			int pieceValue = 0;
			int positionBonus = 0;
			int mobilityBonus = GetMobilityScore(board, piece, square);

			switch (piece.Type)
			{
			case Valor::PieceType::Pawn:
				pieceValue = PawnValue;
				positionBonus = PawnTable[square];
				break;
			case Valor::PieceType::Knight:
				pieceValue = KnightValue;
				positionBonus = KnightTable[square];
				break;
			case Valor::PieceType::Bishop:
				pieceValue = BishopValue;
				break;
			case Valor::PieceType::Rook:
				pieceValue = RookValue;
				break;
			case Valor::PieceType::Queen:
				pieceValue = QueenValue;
				break;
			case Valor::PieceType::King:
				positionBonus = KingSafetyTable[square];
				break;
			default:
				break;
			}

			int pieceScore = pieceValue + positionBonus + mobilityBonus;

			if (piece.Color == Valor::PieceColor::White)
				score += pieceScore;
			else
				score -= pieceScore;
		}

		return score;
	}

}
