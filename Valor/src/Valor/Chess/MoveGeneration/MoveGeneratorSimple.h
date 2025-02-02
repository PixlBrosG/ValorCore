#pragma once

#include "Valor/Chess/Board.h"

namespace Valor::MoveGeneratorSimple {

	std::vector<Move> GeneratePseudoLegalMoves(const Board& board);
	std::vector<Move> GenerateLegalMoves(const Board& board);

	bool IsMoveLegal(const Board& board, Move move);

	void GeneratePawnMoves(const Board& board, std::vector<Move>& moves);
	uint64_t GetPawnMoves(int square, uint64_t occupied, bool isWhite, int enPassantFile, uint8_t& moveFlags);

	void GenerateCastlingMoves(const Board& board, std::vector<Move>& moves);

	inline bool IsPromotionRank(int rank, bool isWhite) { return isWhite ? rank == 7 : rank == 0; }

}
