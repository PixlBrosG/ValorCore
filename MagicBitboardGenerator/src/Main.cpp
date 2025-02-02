#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <cstdint>
#include <bit>
#include <future>
#include <fstream>
#include <chrono>

// Generate random sparse 64-bit number
static uint64_t RandomMagic()
{
	// Use random_device to generate sparse 64-bit number
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

	uint64_t magic = 0;
	for (int i = 0; i < 4; i++)
		magic |= (dis(gen) & 0xFFFF) << (i * 16);

	return magic;
}

static uint64_t GenerateSparseMagic()
{
	return RandomMagic() & RandomMagic() | (1ULL << (rand() % 64));
}

// Get bit index from (rank, file)
static int GetIndex(int rank, int file)
{
	return rank * 8 + file;
}

// Get rank and file from index
static std::pair<int, int> GetPosition(int index)
{
	return { index / 8, index % 8 };
}

// Generate occupancy bitboard for rook moves
static uint64_t GenerateRookMask(int square)
{
	uint64_t mask = 0;
	auto [rank, file] = GetPosition(square);

	for (int r = rank + 1; r < 7; r++) mask |= (1ULL << GetIndex(r, file));
	for (int r = rank - 1; r > 0; r--) mask |= (1ULL << GetIndex(r, file));
	for (int f = file + 1; f < 7; f++) mask |= (1ULL << GetIndex(rank, f));
	for (int f = file - 1; f > 0; f--) mask |= (1ULL << GetIndex(rank, f));

	return mask;
}

// Generate occupancy bitboard for bishop moves
static uint64_t GenerateBishopMask(int square)
{
	uint64_t mask = 0;
	auto [rank, file] = GetPosition(square);

	for (int r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++) mask |= (1ULL << GetIndex(r, f));
	for (int r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--) mask |= (1ULL << GetIndex(r, f));
	for (int r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++) mask |= (1ULL << GetIndex(r, f));
	for (int r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--) mask |= (1ULL << GetIndex(r, f));

	return mask;
}

// Generate all blocker configurations for a given mask
static std::vector<uint64_t> GenerateBlockerBoards(uint64_t mask)
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
			int bit_index = std::countr_zero(temp_mask);
			temp_mask &= temp_mask - 1;
			if (i & (1 << j)) blocker |= (1ULL << bit_index);
		}
		blockers.emplace_back(blocker);
	}
	return blockers;
}

// Check if a candidate magic number is valid
static bool IsValidMagic(uint64_t magic, const std::vector<uint64_t>& occupancies, int shift)
{
	std::vector<uint64_t> seen(1ull << shift, 0);
	for (uint64_t occupancy : occupancies)
	{
		size_t index = (occupancy * magic) >> (64 - shift);
		if (seen[index] != 0 && seen[index] != occupancy)
			return false;
		seen[index] = occupancy;
	}
	return true;
}

// Find a valid magic number for a given square
static uint64_t FindMagic(int square, bool is_rook)
{
	uint64_t mask = is_rook ? GenerateRookMask(square) : GenerateBishopMask(square);
	std::vector<uint64_t> occupancies = GenerateBlockerBoards(mask);
	int shift = std::popcount(mask);

	while (true)
	{
		uint64_t magic = GenerateSparseMagic();
		if (IsValidMagic(magic, occupancies, shift))
			return magic;
	}
}

// Generate magic numbers for all squares
static void GenerateMagicNumbers()
{
	std::array<std::future<uint64_t>, 64> rookFutures;
	std::array<std::future<uint64_t>, 64> bishopFutures;

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	for (int i = 0; i < 64; i++)
	{
		rookFutures[i] = std::async(std::launch::async, FindMagic, i, true);
		bishopFutures[i] = std::async(std::launch::async, FindMagic, i, false);
	}

	std::ofstream file("MagicNumbers.dat", std::ios::binary);

	for (int i = 0; i < 64; i++)
	{
		uint64_t rookMagic = rookFutures[i].get();
		uint64_t bishopMagic = bishopFutures[i].get();
		file.write(reinterpret_cast<char*>(&rookMagic), sizeof(uint64_t));
		file.write(reinterpret_cast<char*>(&bishopMagic), sizeof(uint64_t));
	}

	file.close();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Magic numbers generated in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
	std::cout << "Magic numbers saved to MagicNumbers.dat" << std::endl;
}

// Main function
int main()
{
	GenerateMagicNumbers();
}
