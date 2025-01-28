#include "vlpch.h"
#include "Valor/Chess/MoveGeneration/MagicBitboard.h"

#include "Valor/Chess/Tile.h"

namespace Valor {

	static bool s_Initialized = []() { MagicBitboard::Init(); return true; }();

	uint64_t MagicBitboard::s_KingAttackMask[64] = { 0 };
	uint64_t MagicBitboard::s_KnightAttackMask[64] = { 0 };

	void MagicBitboard::Init() {
		std::cout << "Initializing dynamic bitboards..." << std::endl;

		// Precompute king and knight attack masks
		for (int square = 0; square < 64; ++square) {
			s_KingAttackMask[square] = GenerateKingAttackMask(square);
			s_KnightAttackMask[square] = GenerateKnightAttackMask(square);
		}

		std::cout << "Dynamic bitboards initialized!" << std::endl;
	}

	uint64_t MagicBitboard::GenerateKingAttackMask(int square) {
		uint64_t mask = 0;
		int rank = square / 8;
		int file = square % 8;

		// Generate king moves (one square in all directions)
		for (int r = -1; r <= 1; ++r) {
			for (int f = -1; f <= 1; ++f) {
				if (r == 0 && f == 0) continue; // Skip the king's own square
				int newRank = rank + r, newFile = file + f;
				if (newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8) {
					mask |= 1ULL << (newRank * 8 + newFile);
				}
			}
		}
		return mask;
	}

	uint64_t MagicBitboard::GenerateKnightAttackMask(int square) {
		uint64_t mask = 0;
		int rank = square / 8;
		int file = square % 8;

		// Generate knight moves (L-shape)
		int knightMoves[8][2] = {
			{-2, -1}, {-1, -2}, {1, -2}, {2, -1},
			{-2, 1}, {-1, 2}, {1, 2}, {2, 1}
		};

		for (auto& move : knightMoves) {
			int newRank = rank + move[0];
			int newFile = file + move[1];
			if (newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8) {
				mask |= 1ULL << (newRank * 8 + newFile);
			}
		}
		return mask;
	}

	uint64_t MagicBitboard::CalculateSlidingAttacks(int square, uint64_t blockers, const int directions[], int numDirections) {
		uint64_t attacks = 0;

		for (int i = 0; i < numDirections; ++i) {
			int direction = directions[i];
			int sq = square;

			while (true) {
				sq += direction;

				// Stop if the square goes out of bounds
				if (sq < 0 || sq >= 64) break;

				// Ensure sliding doesn't wrap around the board edges
				int fromRank = square / 8;
				int toRank = sq / 8;

				// Horizontal moves (left/right)
				if ((direction == 1 || direction == -1) && fromRank != toRank) break;

				// Add the attack to the bitboard
				attacks |= (1ULL << sq);

				// Stop if there's a blocker
				if (blockers & (1ULL << sq)) break;
			}
		}

		return attacks;
	}


	uint64_t MagicBitboard::CalculateRookAttacks(int square, uint64_t blockers) {
		static const int rookDirections[4] = { 1, -1, 8, -8 }; // Right, left, up, down
		return CalculateSlidingAttacks(square, blockers, rookDirections, 4);
	}

	uint64_t MagicBitboard::CalculateBishopAttacks(int square, uint64_t blockers) {
		static const int bishopDirections[4] = { 9, -9, 7, -7 }; // Diagonals
		return CalculateSlidingAttacks(square, blockers, bishopDirections, 4);
	}

	uint64_t MagicBitboard::CalculateQueenAttacks(int square, uint64_t blockers) {
		// Combine rook and bishop attacks
		return CalculateRookAttacks(square, blockers) | CalculateBishopAttacks(square, blockers);
	}

	uint64_t MagicBitboard::GetSlidingAttack(int square, uint64_t blockers, PieceType type) {
		switch (type) {
			case PieceType::Rook: return CalculateRookAttacks(square, blockers);
			case PieceType::Bishop: return CalculateBishopAttacks(square, blockers);
			case PieceType::Queen: return CalculateQueenAttacks(square, blockers);
			default: return 0;
		}
	}
}
