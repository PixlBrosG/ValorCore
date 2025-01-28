#pragma once

#include "Valor/Chess/Board.h"

#include <vector>

namespace Valor {

	class MoveGenerator
	{
	public:
		static std::vector<Move> GeneratePseudoLegalMoves(const Board& board);
		static std::vector<Move> GenerateLegalMoves(const Board& board);

		static uint64_t AttackBitboard(const Board& board);

		static uint64_t GeneratePawnMovesForSquare(const Board& board, int square);
	private:
		// Generate move bitboards
		static uint64_t GeneratePawnMoveBitboard(const Board& board);
		static uint64_t GeneratePawnCaptureBitboard(const Board& board);
		static uint64_t GenerateEnPassantMoveBitboard(const Board& board);

		static uint64_t GenerateKnightMoveBitboard(const Board& board);
		
		static uint64_t GenerateSlidingMoveBitboard(const Board& board, PieceType type);
		static uint64_t GenerateSlidingAttackBitboard(int square, uint64_t occupied, PieceType type);

		static uint64_t GenerateKingMoveBitboard(const Board& board);
		static uint64_t GenerateCastlingMoveBitboard(const Board& board);

		static std::vector<Move> GenerateMovesFromBitboard(const Board& board, uint64_t bitboard,
			PieceType pieceType, uint8_t flags = 0);

		static void PrecomputeKingAttackMask();
		static void PrecomputeKnightAttackMask();
	private:
		static uint64_t s_KingAttackMask[64];
		static uint64_t s_KnightAttackMask[64];

		static bool s_IsInitialized;
	};

}
