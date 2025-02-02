#include "vlpch.h"
#include "Valor/Core/ZobristHasher.h"

#include <random>

namespace Valor::ZobristHasher {

	constexpr size_t NumPieceTypes = 6;
	constexpr size_t NumSquares = 64;
	constexpr size_t NumColors = 2;
	constexpr size_t NumCastlingRights = 4;
	constexpr size_t NumEnPassantFiles = 8;

	static uint64_t s_PieceKeys[NumPieceTypes][NumColors][NumSquares];
	static uint64_t s_CastlingKeys[NumCastlingRights];
	static uint64_t s_EnPassantKeys[NumEnPassantFiles];
	static uint64_t s_SideKey;

	static uint64_t Random64()
	{
		std::mt19937_64 generator(std::random_device{}());
		std::uniform_int_distribution<uint64_t> distribution;
		return distribution(generator);
	}

	static void Init()
	{
		for (size_t piece = 0; piece < NumPieceTypes; ++piece)
			for (size_t color = 0; color < NumColors; ++color)
				for (size_t square = 0; square < NumSquares; ++square)
					s_PieceKeys[piece][color][square] = Random64();

		for (size_t i = 0; i < NumCastlingRights; ++i)
			s_CastlingKeys[i] = Random64();

		for (size_t i = 0; i < NumEnPassantFiles; ++i)
			s_EnPassantKeys[i] = Random64();

		s_SideKey = Random64();
	}

	uint64_t Hash(const Board& board)
	{
		uint64_t hash = 0;

		for (size_t piece = 0; piece < NumPieceTypes; ++piece)
		{
			uint64_t pieceBitboard = board.GetPieceBitboard(static_cast<PieceType>(piece + 1));

			while (pieceBitboard)
			{
				int square = std::countr_zero(pieceBitboard);
				int color = board.GetPiece(square).Color == PieceColor::White ? 0 : 1;
				hash ^= s_PieceKeys[piece][color][square];
				pieceBitboard &= pieceBitboard - 1;
			}
		}

		if (board.CanCastle(true, true)) hash ^= s_CastlingKeys[0];
		if (board.CanCastle(true, false)) hash ^= s_CastlingKeys[1];
		if (board.CanCastle(false, true)) hash ^= s_CastlingKeys[2];
		if (board.CanCastle(false, false)) hash ^= s_CastlingKeys[3];

		if (board.GetEnPassantFile() != 0xff)
			hash ^= s_EnPassantKeys[board.GetEnPassantFile()];

		if (board.IsWhiteTurn())
			hash ^= s_SideKey;

		return hash;
	}

	void UpdateHash(uint64_t& hash, const Board& board, Move move)
	{
		int color = board.GetPiece(move.Source).Color == PieceColor::White ? 0 : 1;
		int piece = static_cast<int>(board.GetPiece(move.Source).Type) - 1;

		// Remove old piece-square hash
		hash ^= s_PieceKeys[piece][color][move.Source];

		// Add new piece-square hash
		hash ^= s_PieceKeys[piece][color][move.Target];

		// **Update castling rights (only if king or rook moves)**
		if (move.Source == Tiles::A1 || move.Target == Tiles::A1)
			hash ^= s_CastlingKeys[0];
		else if (move.Source == Tiles::H1 || move.Target == Tiles::H1)
			hash ^= s_CastlingKeys[1];
		else if (move.Source == Tiles::A8 || move.Target == Tiles::A8)
			hash ^= s_CastlingKeys[2];
		else if (move.Source == Tiles::H8 || move.Target == Tiles::H8)
			hash ^= s_CastlingKeys[3];

		// **If king moves, remove castling rights**
		if (move.Source == Tiles::E1 || move.Target == Tiles::E1)
		{
			hash ^= s_CastlingKeys[0];
			hash ^= s_CastlingKeys[1];
		}
		else if (move.Source == Tiles::E8 || move.Target == Tiles::E8)
		{
			hash ^= s_CastlingKeys[2];
			hash ^= s_CastlingKeys[3];
		}

		// **Update en passant**
		if (board.GetEnPassantFile() != 0xff)
			hash ^= s_EnPassantKeys[board.GetEnPassantFile()];

		// **Flip side to move**
		hash ^= s_SideKey;
	}

	static bool s_IsInitialized = []()
	{
		Init();
		return true;
	}();

}
