#pragma once

#include "Valor/Chess/Piece.h"

#include <cstdint>
#include <vector>

namespace Valor {

	class MagicBitboard
	{
	public:
		// Initialization
		static void Init();

		// Attack generation for non-sliding pieces
		static uint64_t GenerateKingAttackMask(int square);
		static uint64_t GenerateKnightAttackMask(int square);

		// Attack generation for sliding pieces
		static uint64_t CalculateRookAttacks(int square, uint64_t blockers);
		static uint64_t CalculateBishopAttacks(int square, uint64_t blockers);
		static uint64_t CalculateQueenAttacks(int square, uint64_t blockers);
		static uint64_t GetSlidingAttack(int square, uint64_t blockers, PieceType type);
	public:
		// Static attack masks for kings and knights
		static uint64_t s_KingAttackMask[64];
		static uint64_t s_KnightAttackMask[64];
	private:
		// Helper for sliding pieces (shared by rook, bishop, and queen)
		static uint64_t CalculateSlidingAttacks(int square, uint64_t blockers, const int directions[], int numDirections);
	};

}
