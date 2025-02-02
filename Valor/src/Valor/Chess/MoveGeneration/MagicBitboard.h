#pragma once

#include <cstdint>
#include <vector>
#include <array>

namespace Valor {

	class MagicBitboard
	{
	public:
		MagicBitboard() = delete;

		// Initialize everything
		static void Init();

		// Get attack bitboards
		static uint64_t GetRookAttacks(int square, uint64_t occupancy);
		static uint64_t GetBishopAttacks(int square, uint64_t occupancy);
		static uint64_t GetQueenAttacks(int square, uint64_t occupancy) {
			return GetRookAttacks(square, occupancy) | GetBishopAttacks(square, occupancy);
		}

		// Precomputed attacks (Knight, King, Pawns)
		static uint64_t GetKnightAttacks(int square) { return s_KnightAttacks[square]; }
		static uint64_t GetKingAttacks(int square) { return s_KingAttacks[square]; }
		static uint64_t GetWhitePawnAttacks(int square) { return s_WhitePawnAttacks[square]; }
		static uint64_t GetBlackPawnAttacks(int square) { return s_BlackPawnAttacks[square]; }

		static uint64_t GetKnightAttacks(int square, uint64_t occupancy) { return GetKnightAttacks(square); }
		static uint64_t GetKingAttacks(int square, uint64_t occupancy) { return GetKingAttacks(square); }

		// Castling masks
		static uint64_t GetKingsideCastleMask(bool isWhite) { return s_KingsideCastleMask[isWhite]; }
		static uint64_t GetQueensideCastleMask(bool isWhite) { return s_QueensideCastleMask[isWhite]; }
	private:
		// Attack computations (used for precomputing magic tables)
		static uint64_t ComputeRookAttacks(int square, uint64_t blockers);
		static uint64_t ComputeBishopAttacks(int square, uint64_t blockers);

		// Setup methods (called inside `Init()`)
		static void InitBlockerMasks();
		static void GenerateAttackTables();
		static void GenerateCastleMasks();
		static void GeneratePawnAttacks();
		static void LoadMagicNumbers();

		// Helpers for bit manipulation
		static uint64_t GenerateRookMask(int square);
		static uint64_t GenerateBishopMask(int square);
		static std::vector<uint64_t> GenerateBlockerBoards(uint64_t mask);

		static constexpr int GetIndex(int rank, int file) { return rank * 8 + file; }
		static constexpr std::pair<int, int> GetPosition(int square) { return { square / 8, square % 8 }; }

	private:
		// Magic numbers
		static std::array<uint64_t, 64> s_RookMagicNumbers;
		static std::array<uint64_t, 64> s_BishopMagicNumbers;

		// Attack tables
		static std::array<std::array<uint64_t, 1 << 12>, 64> s_RookAttacks;
		static std::array<std::array<uint64_t, 1 << 9>, 64> s_BishopAttacks;

		// Blocker masks
		static std::array<uint64_t, 64> s_RookBlockerMasks;
		static std::array<uint64_t, 64> s_BishopBlockerMasks;

		// Relevant bits
		static std::array<int, 64> s_RookRelevantBits;
		static std::array<int, 64> s_BishopRelevantBits;

		// Other attack tables
		static std::array<uint64_t, 64> s_KnightAttacks;
		static std::array<uint64_t, 64> s_KingAttacks;
		static std::array<uint64_t, 2> s_KingsideCastleMask;
		static std::array<uint64_t, 2> s_QueensideCastleMask;
		static std::array<uint64_t, 64> s_WhitePawnAttacks;
		static std::array<uint64_t, 64> s_BlackPawnAttacks;
	};

}
