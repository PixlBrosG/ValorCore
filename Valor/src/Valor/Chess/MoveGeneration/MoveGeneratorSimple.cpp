#include "vlpch.h"
#include "Valor/Chess/MoveGeneration/MoveGeneratorSimple.h"

#include "Valor/Chess/MoveGeneration/MagicBitboard.h"

#include <iostream>
#include <bitset>

namespace Valor::MoveGeneratorSimple {

	using AttackFunction = uint64_t(*)(int, uint64_t);

	static void addMoves(uint64_t bitboard, AttackFunction getAttacks, std::vector<Move>& moves,
		uint64_t ownPieces, PieceType pieceType, const Board& board)
	{
		while (bitboard) {
			int square = std::countr_zero(bitboard);
			bitboard &= bitboard - 1;

			uint64_t attacks = getAttacks(square, board.Occupied()) & ~ownPieces;

			while (attacks)
			{
				int target = std::countr_zero(attacks);
				attacks &= attacks - 1;

				uint8_t moveFlags = 0;
				if (board.IsOccupied(target)) moveFlags |= MoveFlags::Capture;

				moves.emplace_back(Move(square, target, pieceType, moveFlags));
			}
		}
	}

	std::vector<Move> MoveGeneratorSimple::GeneratePseudoLegalMoves(const Board& board)
	{
		std::vector<Move> moves;
		moves.reserve(50);

		bool isWhite = board.IsWhiteTurn();
		uint64_t ownPieces = board.AllPieces(isWhite);

		// Using function pointers to avoid lambda-related issues
		addMoves(board.Knights() & ownPieces, MagicBitboard::GetKnightAttacks, moves, ownPieces, PieceType::Knight, board);
		addMoves(board.Bishops() & ownPieces, MagicBitboard::GetBishopAttacks, moves, ownPieces, PieceType::Bishop, board);
		addMoves(board.Rooks() & ownPieces, MagicBitboard::GetRookAttacks, moves, ownPieces, PieceType::Rook, board);
		addMoves(board.Queens() & ownPieces, MagicBitboard::GetQueenAttacks, moves, ownPieces, PieceType::Queen, board);
		addMoves(board.Kings() & ownPieces, MagicBitboard::GetKingAttacks, moves, ownPieces, PieceType::King, board);

		// Handle pawns separately
		GeneratePawnMoves(board, moves);
		GenerateCastlingMoves(board, moves);

		return moves;
	}

	std::vector<Move> GenerateLegalMoves(const Board& board)
	{
		std::vector<Move> moves = GeneratePseudoLegalMoves(board);
		std::vector<Move> legalMoves;
		legalMoves.reserve(moves.size());

		for (Move move : moves)
		{
			if (IsMoveLegal(board, move))
				legalMoves.emplace_back(move);
		}

		return legalMoves;
	}

	bool IsMoveLegal(const Board& board, Move move)
	{
		Board newBoard = board;
		newBoard.MakeMove(move);
		return !newBoard.IsCheck(board.IsWhiteTurn());
	}

	void GeneratePawnMoves(const Board& board, std::vector<Move>& moves)
	{
		bool isWhite = board.IsWhiteTurn();
		uint64_t pawns = board.Pawns(isWhite);
		uint64_t occupied = board.Occupied();
		int enPassantFile = board.GetEnPassantFile();

		while (pawns) {
			int square = std::countr_zero(pawns);
			pawns &= pawns - 1;
			uint8_t moveFlags = 0;

			uint64_t attacks = GetPawnMoves(square, occupied, isWhite, enPassantFile, moveFlags) & ~board.AllPieces(isWhite);
			while (attacks) {
				int target = std::countr_zero(attacks);
				attacks &= attacks - 1;
				if (IsPromotionRank(target, isWhite)) {
					moveFlags |= MoveFlags::Promotion;
					// Generate all promotion types
					moves.emplace_back(Move(square, target, PieceType::Pawn, moveFlags, PieceType::Knight));
					moves.emplace_back(Move(square, target, PieceType::Pawn, moveFlags, PieceType::Bishop));
					moves.emplace_back(Move(square, target, PieceType::Pawn, moveFlags, PieceType::Rook));
					moves.emplace_back(Move(square, target, PieceType::Pawn, moveFlags, PieceType::Queen));
				}
				else {
					moves.emplace_back(Move(square, target, PieceType::Pawn, moveFlags));
				}
			}
		}
	}

	uint64_t GetPawnMoves(int square, uint64_t occupied, bool isWhite, int enPassantFile, uint8_t& moveFlags)
	{
		uint64_t moves = 0ULL;
		uint64_t singlePush, doublePush, attacks, enPassantCapture;

		int rank = square / 8;
		int file = square % 8;

		if (isWhite) {
			// Single and double pawn push
			singlePush = (1ULL << (square + 8)) & ~occupied;
			doublePush = (singlePush && rank == 1) ? (1ULL << (square + 16)) & ~occupied : 0;

			// Pawn captures
			attacks = MagicBitboard::GetWhitePawnAttacks(square) & occupied;

			// En passant capture
			enPassantCapture = 0;
			if (enPassantFile != -1 && (file == enPassantFile - 1 || file == enPassantFile + 1)) {
				enPassantCapture = (1ULL << (square + 7)) | (1ULL << (square + 9));
				// Use (rank + 1)
				enPassantCapture &= (1ULL << ((rank + 1) * 8 + enPassantFile));
			}
		}
		else { // Black
			// Single and double pawn push
			singlePush = (1ULL << (square - 8)) & ~occupied;
			doublePush = (singlePush && rank == 6) ? (1ULL << (square - 16)) & ~occupied : 0;

			// Pawn captures
			attacks = MagicBitboard::GetBlackPawnAttacks(square) & occupied;

			// En passant capture
			enPassantCapture = 0;
			if (enPassantFile != -1 && (file == enPassantFile - 1 || file == enPassantFile + 1)) {
				enPassantCapture = (1ULL << (square - 7)) | (1ULL << (square - 9));
				// Use (rank - 1)
				enPassantCapture &= (1ULL << ((rank - 1) * 8 + enPassantFile));
			}
		}

		// Set move flags
		if (attacks) moveFlags |= MoveFlags::Capture;
		if (enPassantCapture) moveFlags |= MoveFlags::EnPassant;

		// Combine all pawn moves
		moves |= singlePush | doublePush | attacks | enPassantCapture;
		return moves;
	}

	void GenerateCastlingMoves(const Board& board, std::vector<Move>& moves)
	{
		bool isWhite = board.IsWhiteTurn();
		int kingSquare = board.GetKingSquare(isWhite);

		// (1) Must not be in check at the start
		// We'll assume Board::IsCheck(side) checks if 'side' is in check.
		if (board.IsCheck(isWhite))
			return;

		// (2) Check each side's castling right
		bool canCastleKingside = board.CanCastle(isWhite, /*kingside=*/true);
		bool canCastleQueenside = board.CanCastle(isWhite, /*kingside=*/false);

		// We'll need a helper for "are these squares empty?"
		auto squaresAreEmpty = [&](std::initializer_list<int> squares)
			{
				uint64_t occ = board.Occupied();
				for (int sq : squares) {
					if (occ & (1ULL << sq))
						return false;
				}
				return true;
			};

		// We'll need a helper for "are these squares safe from enemy attacks?"
		// Typically you'd do board.IsSquareAttacked(sq, !isWhite) or replicate a small check routine.
		auto squaresAreSafe = [&](std::initializer_list<int> squares)
			{
				for (int sq : squares) {
					if (board.IsSquareAttacked(sq, isWhite)) // You need some method like this
						return false;
				}
				return true;
			};

		//----------------------------------------
		// KINGSIDE CASTLE
		//----------------------------------------
		if (canCastleKingside)
		{
			// For White: king on e1 => squares f1=5, g1=6 must be empty
			// The king travels e1 -> f1 -> g1, so squares 4,5,6 must not be attacked.
			int sq1 = kingSquare + 1;   // f1 (white) or f8 (black)
			int sq2 = kingSquare + 2;   // g1 (white) or g8 (black)

			if (squaresAreEmpty({ sq1, sq2 }) &&
				squaresAreSafe({ kingSquare, sq1, sq2 }))
			{
				// We’ve passed all checks. Add the castling move:
				moves.emplace_back(Move(kingSquare, sq2, PieceType::King, MoveFlags::Castling));
			}
		}

		//----------------------------------------
		// QUEENSIDE CASTLE
		//----------------------------------------
		if (canCastleQueenside)
		{
			// For White: king on e1 => squares d1=3, c1=2, b1=1 between e1 & a1=0
			// Typically we require b1,c1,d1 to be empty; the king passes e1->d1->c1.
			// Minimal requirement: c1 & d1 are empty for the king’s path,
			// but many engines also want b1 empty because the rook crosses it.
			// Adjust as your design requires.

			// For simplicity here, we check all squares between e1 and a1 are empty:
			int sq1 = kingSquare - 1; // d1=3
			int sq2 = kingSquare - 2; // c1=2
			int sq3 = kingSquare - 3; // b1=1  (optionally also check this is empty)
			// The king passes e1=4, d1=3, c1=2 (so squares 4,3,2 must be safe).

			// If you only want to check d1,c1 empty:
			if (squaresAreEmpty({ sq1, sq2, sq3 }) &&
				squaresAreSafe({ kingSquare, sq1, sq2 }))
			{
				moves.emplace_back(Move(kingSquare, sq2, PieceType::King, MoveFlags::Castling));
			}
		}
	}


}
