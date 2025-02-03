#include "vlpch.h"
#include "Valor/Chess/MoveGeneration/MagicBitboard.h"

#include <fstream>
#include <iostream>
#include <bit>

namespace Valor {

	static bool isInitialized = []() { MagicBitboard::Init(); return true; }();

	// Define static variables
	std::array<uint64_t, 64> MagicBitboard::s_RookMagicNumbers = {};
	std::array<uint64_t, 64> MagicBitboard::s_BishopMagicNumbers = {};
	std::array<std::array<uint64_t, 1 << 12>, 64> MagicBitboard::s_RookAttacks = {};
	std::array<std::array<uint64_t, 1 << 9>, 64> MagicBitboard::s_BishopAttacks = {};
	std::array<uint64_t, 64> MagicBitboard::s_RookBlockerMasks = {};
	std::array<uint64_t, 64> MagicBitboard::s_BishopBlockerMasks = {};
	std::array<int, 64> MagicBitboard::s_RookRelevantBits = {};
	std::array<int, 64> MagicBitboard::s_BishopRelevantBits = {};
	std::array<uint64_t, 64> MagicBitboard::s_KnightAttacks = {};
	std::array<uint64_t, 64> MagicBitboard::s_KingAttacks = {};
	std::array<uint64_t, 2> MagicBitboard::s_KingsideCastleMask = {};
	std::array<uint64_t, 2> MagicBitboard::s_QueensideCastleMask = {};
	std::array<uint64_t, 64> MagicBitboard::s_WhitePawnAttacks = {};
	std::array<uint64_t, 64> MagicBitboard::s_BlackPawnAttacks = {};

	// Initialize all masks and attack tables
	void MagicBitboard::Init()
	{
		LoadMagicNumbers();
		InitBlockerMasks();
		GenerateAttackTables();
		GenerateCastleMasks();
		GeneratePawnAttacks();
	}

	uint64_t MagicBitboard::GetRookAttacks(int square, uint64_t occupancy)
	{
		uint64_t blockers = occupancy & s_RookBlockerMasks[square];
		int index = (blockers * s_RookMagicNumbers[square]) >> (64 - s_RookRelevantBits[square]);
		return s_RookAttacks[square][index];
	}

	uint64_t MagicBitboard::GetBishopAttacks(int square, uint64_t occupancy)
	{
		uint64_t blockers = occupancy & s_BishopBlockerMasks[square];
		int index = (blockers * s_BishopMagicNumbers[square]) >> (64 - s_BishopRelevantBits[square]);
		return s_BishopAttacks[square][index];
	}

	// Generate blocker masks for sliding pieces
	void MagicBitboard::InitBlockerMasks()
	{
		for (int i = 0; i < 64; i++)
		{
			s_RookBlockerMasks[i] = GenerateRookMask(i);
			s_BishopBlockerMasks[i] = GenerateBishopMask(i);

			s_RookRelevantBits[i] = std::popcount(s_RookBlockerMasks[i]);
			s_BishopRelevantBits[i] = std::popcount(s_BishopBlockerMasks[i]);
		}
	}

	void MagicBitboard::GenerateAttackTables()
	{
		// Precompute rook attacks
		for (int square = 0; square < 64; ++square)
		{
			std::vector<uint64_t> blockerBoards = GenerateBlockerBoards(s_RookBlockerMasks[square]);

			for (uint64_t blockers : blockerBoards)
			{
				int index = (blockers * s_RookMagicNumbers[square]) >> (64 - s_RookRelevantBits[square]);
				s_RookAttacks[square][index] = ComputeRookAttacks(square, blockers);
			}
		}

		// Precompute bishop attacks
		for (int square = 0; square < 64; ++square)
		{
			std::vector<uint64_t> blockerBoards = GenerateBlockerBoards(s_BishopBlockerMasks[square]);

			for (uint64_t blockers : blockerBoards)
			{
				int index = (blockers * s_BishopMagicNumbers[square]) >> (64 - s_BishopRelevantBits[square]);
				s_BishopAttacks[square][index] = ComputeBishopAttacks(square, blockers);
			}
		}

		// Precompute knight attacks
		for (int square = 0; square < 64; ++square)
		{
			auto [rank, file] = GetPosition(square);
			int directions[8] = { 6, 10, 15, 17, -6, -10, -15, -17 };
			uint64_t attacks = 0;
			for (int dir : directions)
			{
				int nextSquare = square + dir;
				if (nextSquare >= 0 && nextSquare < 64)
				{
					auto [nextRank, nextFile] = GetPosition(nextSquare);
					if (std::abs(nextRank - rank) == 2 && std::abs(nextFile - file) == 1)
						attacks |= (1ULL << nextSquare);
					else if (std::abs(nextRank - rank) == 1 && std::abs(nextFile - file) == 2)
						attacks |= (1ULL << nextSquare);
				}
			}
			s_KnightAttacks[square] = attacks;
		}

		// Precompute king attacks
		for (int square = 0; square < 64; ++square)
		{
			auto [rank, file] = GetPosition(square);
			int directions[8] = { 1, -1, 8, -8, 7, -7, 9, -9 };
			uint64_t attacks = 0;
			for (int dir : directions)
			{
				int nextSquare = square + dir;
				if (nextSquare >= 0 && nextSquare < 64)
				{
					auto [nextRank, nextFile] = GetPosition(nextSquare);
					if (std::abs(nextRank - rank) <= 1 && std::abs(nextFile - file) <= 1)
						attacks |= (1ULL << nextSquare);
				}
			}
			s_KingAttacks[square] = attacks;
		}
	}

	// Generate castling masks
	void MagicBitboard::GenerateCastleMasks()
	{
		s_KingsideCastleMask[0] = (1ull << 5) | (1ull << 6);  // White
		s_QueensideCastleMask[0] = (1ull << 1) | (1ull << 2) | (1ull << 3);

		s_KingsideCastleMask[1] = (1ull << 61) | (1ull << 62); // Black
		s_QueensideCastleMask[1] = (1ull << 57) | (1ull << 58) | (1ull << 59);
	}

	// Generate pawn attack bitboards
	void MagicBitboard::GeneratePawnAttacks()
	{
		constexpr uint64_t fileA = 0x0101010101010101;
		constexpr uint64_t fileH = 0x8080808080808080;

		for (int square = 0; square < 64; ++square)
		{
			uint64_t bit = 1ULL << square;
			s_WhitePawnAttacks[square] = ((bit & ~fileA) << 7) | ((bit & ~fileH) << 9);
			s_BlackPawnAttacks[square] = ((bit & ~fileA) >> 9) | ((bit & ~fileH) >> 7);
		}
	}

	void MagicBitboard::LoadMagicNumbers()
	{
		std::ifstream file("MagicNumbers.dat", std::ios::binary);
		if (!file.is_open())
		{
			std::cerr << "Failed to open magic numbers file" << std::endl;
			return;
		}

		for (int i = 0; i < 64; i++)
		{
			file.read(reinterpret_cast<char*>(&s_RookMagicNumbers[i]), sizeof(uint64_t));
			file.read(reinterpret_cast<char*>(&s_BishopMagicNumbers[i]), sizeof(uint64_t));
		}
	}

	// Compute Rook attacks given blocker pieces
	uint64_t MagicBitboard::ComputeRookAttacks(int square, uint64_t blockers)
	{
		uint64_t attacks = 0;
		auto [rank, file] = GetPosition(square);
		int directions[4] = { 1, -1, 8, -8 };

		for (int dir : directions)
		{
			for (int i = 1; i < 8; ++i)
			{
				int nextSquare = square + i * dir;

				auto [nextRank, nextFile] = GetPosition(nextSquare);
				if ((dir == 1 || dir == -1) && nextRank != rank) break;

				if (nextSquare < 0 || nextSquare >= 64) break;

				attacks |= (1ULL << nextSquare);

				if (blockers & (1ULL << nextSquare)) break;
			}
		}

		return attacks;
	}

	// Compute Bishop attacks given blocker pieces
	uint64_t MagicBitboard::ComputeBishopAttacks(int square, uint64_t blockers)
	{
		uint64_t attacks = 0;
		auto [rank, file] = GetPosition(square);
		int directions[4] = { 7, -7, 9, -9 };

		for (int dir : directions)
		{
			for (int i = 1; i < 8; ++i)
			{
				int nextSquare = square + i * dir;

				if (nextSquare < 0 || nextSquare >= 64) break;

				auto [nextRank, nextFile] = GetPosition(nextSquare);

				if (std::abs(nextRank - rank) != i || std::abs(nextFile - file) != i) break;

				attacks |= (1ULL << nextSquare);

				if (blockers & (1ULL << nextSquare)) break;
			}
		}

		return attacks;
	}


	// Generate all possible blocker bitboards
	std::vector<uint64_t> MagicBitboard::GenerateBlockerBoards(uint64_t mask)
	{
		int bits = std::popcount(mask);
		int variations = 1 << bits;

		std::vector<uint64_t> blockers;
		blockers.reserve(variations);

		for (int i = 0; i < variations; i++)
		{
			uint64_t blocker = 0, temp_mask = mask;
			for (int j = 0; temp_mask; j++)
			{
				int bitIndex = std::countr_zero(temp_mask);
				temp_mask &= temp_mask - 1;
				if (i & (1 << j)) blocker |= (1ULL << bitIndex);
			}
			blockers.emplace_back(blocker);
		}
		return blockers;
	}

	// Generate Rook move mask (excluding edge squares)
	uint64_t MagicBitboard::GenerateRookMask(int square)
	{
		uint64_t mask = 0;
		auto [rank, file] = GetPosition(square);

		for (int r = rank + 1; r < 7; r++) mask |= (1ULL << GetIndex(r, file));
		for (int r = rank - 1; r > 0; r--) mask |= (1ULL << GetIndex(r, file));
		for (int f = file + 1; f < 7; f++) mask |= (1ULL << GetIndex(rank, f));
		for (int f = file - 1; f > 0; f--) mask |= (1ULL << GetIndex(rank, f));

		return mask;
	}

	// Generate Bishop move mask (excluding edge squares)
	uint64_t MagicBitboard::GenerateBishopMask(int square)
	{
		uint64_t mask = 0;
		auto [rank, file] = GetPosition(square);

		for (int r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++) mask |= (1ULL << GetIndex(r, f));
		for (int r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--) mask |= (1ULL << GetIndex(r, f));
		for (int r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++) mask |= (1ULL << GetIndex(r, f));
		for (int r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--) mask |= (1ULL << GetIndex(r, f));

		return mask;
	}

}
