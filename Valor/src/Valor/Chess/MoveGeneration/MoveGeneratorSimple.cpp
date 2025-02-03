#include "vlpch.h"
#include "Valor/Chess/MoveGeneration/MoveGeneratorSimple.h"

#include "Valor/Chess/MoveGeneration/MagicBitboard.h"

namespace Valor::MoveGeneratorSimple {

	using AttackFunction = uint64_t(*)(int, uint64_t);

	static void addMoves(uint64_t bitboard, AttackFunction getAttacks, std::vector<Move>& moves,
		uint64_t ownPieces, const Board& board)
	{
		while (bitboard) {
			int square = std::countr_zero(bitboard);
			bitboard &= bitboard - 1;

			uint64_t attacks = getAttacks(square, board.Occupied()) & ~ownPieces;

			while (attacks)
			{
				int target = std::countr_zero(attacks);
				attacks &= attacks - 1;

				moves.emplace_back(Move(square, target));
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
		addMoves(board.Knights() & ownPieces, MagicBitboard::GetKnightAttacks, moves, ownPieces, board);
		addMoves(board.Bishops() & ownPieces, MagicBitboard::GetBishopAttacks, moves, ownPieces, board);
		addMoves(board.Rooks() & ownPieces, MagicBitboard::GetRookAttacks, moves, ownPieces, board);
		addMoves(board.Queens() & ownPieces, MagicBitboard::GetQueenAttacks, moves, ownPieces, board);
		addMoves(board.Kings() & ownPieces, MagicBitboard::GetKingAttacks, moves, ownPieces, board);

		// Handle pawns and castling separately
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
		return !newBoard.IsCheck(false);
	}

	void GeneratePawnMoves(const Board& board, std::vector<Move>& moves)
	{
		bool isWhite = board.IsWhiteTurn();
		uint64_t pawns = board.Pawns(isWhite);
		uint64_t occupied = board.Occupied();
		int enPassantFile = board.GetEnPassantFile();

		while (pawns)
		{
			int square = std::countr_zero(pawns);
			pawns &= pawns - 1;

			uint64_t attacks = GetPawnMoves(square, occupied, isWhite, enPassantFile) & ~board.AllPieces(isWhite);
			while (attacks) {
				int target = std::countr_zero(attacks);
				attacks &= attacks - 1;
				if (IsPromotionRank(target, isWhite))
				{
					moves.emplace_back(Move(square, target, PieceType::Pawn, 0, PieceType::Queen));
					moves.emplace_back(Move(square, target, PieceType::Pawn, 0, PieceType::Rook));
					moves.emplace_back(Move(square, target, PieceType::Pawn, 0, PieceType::Bishop));
					moves.emplace_back(Move(square, target, PieceType::Pawn, 0, PieceType::Knight));
				}
				else
				{
					moves.emplace_back(Move(square, target, PieceType::Pawn));
				}
			}
		}
	}

	uint64_t GetPawnMoves(int square, uint64_t occupied, bool isWhite, int enPassantFile)
	{
		uint64_t moves = 0ull;
		uint64_t singlePush, doublePush, attacks, enPassantCapture;

		int rank = square / 8;
		int file = square % 8;

		if (isWhite)
		{
			singlePush = (1ULL << (square + 8)) & ~occupied;
			doublePush = (singlePush && rank == 1) ? (1ULL << (square + 16)) & ~occupied : 0;

			attacks = MagicBitboard::GetWhitePawnAttacks(square) & occupied;

			enPassantCapture = 0;
			if (enPassantFile != -1 && (file == enPassantFile - 1 || file == enPassantFile + 1)) {
				enPassantCapture = (1ULL << (square + 7)) | (1ULL << (square + 9));
				enPassantCapture &= (1ULL << ((rank + 1) * 8 + enPassantFile));
			}
		}
		else
		{
			singlePush = (1ULL << (square - 8)) & ~occupied;
			doublePush = (singlePush && rank == 6) ? (1ULL << (square - 16)) & ~occupied : 0;

			attacks = MagicBitboard::GetBlackPawnAttacks(square) & occupied;

			enPassantCapture = 0;
			if (enPassantFile != -1 && (file == enPassantFile - 1 || file == enPassantFile + 1)) {
				enPassantCapture = (1ULL << (square - 7)) | (1ULL << (square - 9));
				enPassantCapture &= (1ULL << ((rank - 1) * 8 + enPassantFile));
			}
		}

		moves |= singlePush | doublePush | attacks | enPassantCapture;
		return moves;
	}

	void GenerateCastlingMoves(const Board& board, std::vector<Move>& moves)
	{
		bool isWhite = board.IsWhiteTurn();
		int kingSquare = board.GetKingSquare(isWhite);

		if (board.IsCheck(isWhite))
			return;

		bool canCastleKingside = board.CanCastle(isWhite, true);
		bool canCastleQueenside = board.CanCastle(isWhite, false);

		auto squaresAreEmpty = [&](std::initializer_list<int> squares)
		{
			uint64_t occ = board.Occupied();
			for (int sq : squares)
			{
				if (occ & (1ULL << sq))
					return false;
			}
			return true;
		};

		auto squaresAreSafe = [&](std::initializer_list<int> squares)
		{
			for (int sq : squares)
			{
				if (board.IsSquareAttacked(sq, isWhite))
					return false;
			}
			return true;
		};

		//----------------------------------------
		// KINGSIDE CASTLE
		//----------------------------------------
		if (canCastleKingside)
		{
			int sq1 = kingSquare + 1;
			int sq2 = kingSquare + 2;

			if (squaresAreEmpty({ sq1, sq2 }) &&
				squaresAreSafe({ kingSquare, sq1, sq2 }))
			{
				moves.emplace_back(Move(kingSquare, sq2, PieceType::King));
			}
		}

		//----------------------------------------
		// QUEENSIDE CASTLE
		//----------------------------------------
		if (canCastleQueenside)
		{
			int sq1 = kingSquare - 1;
			int sq2 = kingSquare - 2;
			int sq3 = kingSquare - 3;

			if (squaresAreEmpty({ sq1, sq2, sq3 }) &&
				squaresAreSafe({ kingSquare, sq1, sq2 }))
			{
				moves.emplace_back(Move(kingSquare, sq2, PieceType::King));
			}
		}
	}

	std::vector<Move> GenerateMovesForPiece(const Board& board, int square, const Piece& piece)
	{
		std::vector<Move> moves;
		moves.reserve(10);
		uint64_t ownPieces = board.AllPieces(piece.Color == PieceColor::White);
		uint64_t attacks = 0;
		switch (piece.Type)
		{
		case PieceType::Pawn:
			attacks = GetPawnMoves(square, board.Occupied(), piece.Color == PieceColor::White, board.GetEnPassantFile());
			break;
		case PieceType::Knight:
			attacks = MagicBitboard::GetKnightAttacks(square);
			break;
		case PieceType::Bishop:
			attacks = MagicBitboard::GetBishopAttacks(square, board.Occupied());
			break;
		case PieceType::Rook:
			attacks = MagicBitboard::GetRookAttacks(square, board.Occupied());
			break;
		case PieceType::Queen:
			attacks = MagicBitboard::GetQueenAttacks(square, board.Occupied());
			break;
		case PieceType::King:
			attacks = MagicBitboard::GetKingAttacks(square);
			break;
		default:
			break;
		}
		while (attacks)
		{
			int target = std::countr_zero(attacks);
			attacks &= attacks - 1;
			moves.emplace_back(Move(square, target, piece.Type));
		}
		return moves;
	}

}
