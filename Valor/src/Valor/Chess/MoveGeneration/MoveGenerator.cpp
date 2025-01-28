#include "vlpch.h"
#include "Valor/Chess/MoveGeneration/MoveGenerator.h"

#include "Valor/Chess/MoveGeneration/MagicBitboard.h"

#include <unordered_map>
#include <bit>

namespace Valor {

	bool MoveGenerator::s_IsInitialized = []() {
		MoveGenerator::PrecomputeKingAttackMask();
		MoveGenerator::PrecomputeKnightAttackMask();
		return true;
	}();

	uint64_t MoveGenerator::s_KingAttackMask[64] = { 0 };
	uint64_t MoveGenerator::s_KnightAttackMask[64] = { 0 };

	uint64_t MoveGenerator::GeneratePawnMoveBitboard(const Board& board, PieceColor turn)
	{
		bool isWhiteTurn = turn == PieceColor::White;

		uint64_t pawns = board.Pawns(turn);
		uint64_t empty = ~board.Occupied();

		// Single push: pawns move one square forward to an empty square
		uint64_t singlePush = isWhiteTurn
			? (pawns << 8) & empty  // White pawns move up
			: (pawns >> 8) & empty; // Black pawns move down

		// Double push: pawns on their starting rank move two squares forward
		uint64_t doublePushRank = isWhiteTurn
			? 0x000000000000FF00ull // Rank 2 for white
			: 0x00FF000000000000ull; // Rank 7 for black

		uint64_t doublePush = isWhiteTurn
			? ((pawns & doublePushRank) << 8 & empty) << 8 & empty // Two empty squares ahead for white
			: ((pawns & doublePushRank) >> 8 & empty) >> 8 & empty; // Two empty squares ahead for black

		return singlePush | doublePush;
	}

	uint64_t MoveGenerator::GeneratePawnCaptureBitboard(const Board& board, PieceColor turn)
	{
		bool isWhiteTurn = turn == PieceColor::White;

		uint64_t pawns = board.Pawns(turn);
		uint64_t enemy = board.OpponentPieces();

		uint64_t leftCapture = isWhiteTurn
			? (pawns << 7) & enemy & ~Board::FileH
			: (pawns >> 7) & enemy & ~Board::FileA;

		uint64_t rightCapture = isWhiteTurn
			? (pawns << 9) & enemy & ~Board::FileA
			: (pawns >> 9) & enemy & ~Board::FileH;

		return leftCapture | rightCapture;
	}

	uint64_t MoveGenerator::GenerateEnPassantMoveBitboard(const Board& board, PieceColor turn)
	{
		if (board.GetEnPassantTarget() == Tile::None)
			return 0;

		bool isWhiteTurn = turn == PieceColor::White;

		uint64_t pawns = board.Pawns(turn);
		uint64_t enPassant = 1ull << board.GetEnPassantTarget();

		uint64_t leftCapture = isWhiteTurn
			? (pawns << 7) & enPassant & ~Board::FileH
			: (pawns >> 7) & enPassant & ~Board::FileA;

		uint64_t rightCapture = isWhiteTurn
			? (pawns << 9) & enPassant & ~Board::FileA
			: (pawns >> 9) & enPassant & ~Board::FileH;

		return leftCapture | rightCapture;
	}

	uint64_t MoveGenerator::GenerateKnightMoveBitboard(const Board& board, PieceColor turn)
	{
		bool isWhiteTurn = turn == PieceColor::White;

		uint64_t knights = board.Knights(turn);
		uint64_t emptyOrEnemy = ~board.PlayerPieces();

		uint64_t moves = 0;
		while (knights)
		{
			int square = std::countr_zero(knights);
			moves |= s_KnightAttackMask[square] & emptyOrEnemy;
			knights &= knights - 1;
		}

		return moves;
	}

	uint64_t MoveGenerator::GenerateSlidingMoveBitboard(const Board& board, PieceType type, PieceColor turn)
	{
		uint64_t pieces;
		switch (type)
		{
		case PieceType::Bishop:
			pieces = board.Bishops(turn);
			break;
		case PieceType::Rook:
			pieces = board.Rooks(turn);
			break;
		case PieceType::Queen:
			pieces = board.Queens(turn);
			break;
		default:
			return 0;
		}

		uint64_t moves = 0;
		while (pieces)
		{
			int square = std::countr_zero(pieces);
			moves |= GenerateSlidingAttackBitboard(square, board.Occupied(), type);
			pieces &= pieces - 1;
		}

		return moves;
	}

	uint64_t MoveGenerator::GenerateSlidingAttackBitboard(int square, uint64_t occupied, PieceType type)
	{
		return MagicBitboard::GetSlidingAttack(square, occupied, type);
	}

	uint64_t MoveGenerator::GenerateKingMoveBitboard(const Board& board, PieceColor turn)
	{
		bool isWhiteTurn = turn == PieceColor::White;
		uint64_t king = board.Kings(turn);
		uint64_t emptyOrEnemy = ~board.PlayerPieces();

		return s_KingAttackMask[std::countr_zero(king)] & emptyOrEnemy;
	}

	uint64_t MoveGenerator::GenerateCastlingMoveBitboard(const Board& board, PieceColor turn)
	{
		uint64_t moves = 0;
		if (turn == PieceColor::White)
		{
			if (board.GetCastlingRights(PieceColor::White, true)  && !(board.Occupied() & 0x0000000000000060)) moves |= (1 << 62); // White kingside
			if (board.GetCastlingRights(PieceColor::White, false) && !(board.Occupied() & 0x000000000000000E)) moves |= (1 << 58); // White queenside
		}
		else
		{
			if (board.GetCastlingRights(PieceColor::Black, true)  && !(board.Occupied() & 0x6000000000000000)) moves |= (1 << 6);  // Black kingside
			if (board.GetCastlingRights(PieceColor::Black, false) && !(board.Occupied() & 0x0E00000000000000)) moves |= (1 << 2);  // Black queenside
		}

		return moves;
	}

	std::vector<Move> MoveGenerator::GeneratePseudoLegalMoves(const Board& board, PieceColor turn)
	{
		std::vector<Move> moves;
		moves.reserve(200); // Preallocate sufficient space for efficiency

		// Pawn moves
		uint64_t pawnMoves = GeneratePawnMoveBitboard(board, turn);
		auto pawnMoveList = GenerateMovesFromBitboard(board, pawnMoves, PieceType::Pawn, turn);
		moves.insert(moves.end(), pawnMoveList.begin(), pawnMoveList.end());

		uint64_t pawnCaptures = GeneratePawnCaptureBitboard(board, turn);
		auto pawnCaptureList = GenerateMovesFromBitboard(board, pawnCaptures, PieceType::Pawn, turn);
		moves.insert(moves.end(), pawnCaptureList.begin(), pawnCaptureList.end());

		uint64_t enPassantMoves = GenerateEnPassantMoveBitboard(board, turn);
		auto enPassantList = GenerateMovesFromBitboard(board, enPassantMoves, PieceType::Pawn, turn);
		moves.insert(moves.end(), enPassantList.begin(), enPassantList.end());

		// Knight moves
		uint64_t knightMoves = GenerateKnightMoveBitboard(board, turn);
		auto knightMoveList = GenerateMovesFromBitboard(board, knightMoves, PieceType::Knight, turn);
		moves.insert(moves.end(), knightMoveList.begin(), knightMoveList.end());

		// Sliding moves (Bishop, Rook, Queen)
		uint64_t bishopMoves = GenerateSlidingMoveBitboard(board, PieceType::Bishop, turn);
		auto bishopMoveList = GenerateMovesFromBitboard(board, bishopMoves, PieceType::Bishop, turn);
		moves.insert(moves.end(), bishopMoveList.begin(), bishopMoveList.end());

		uint64_t rookMoves = GenerateSlidingMoveBitboard(board, PieceType::Rook, turn);
		auto rookMoveList = GenerateMovesFromBitboard(board, rookMoves, PieceType::Rook, turn);
		moves.insert(moves.end(), rookMoveList.begin(), rookMoveList.end());

		uint64_t queenMoves = GenerateSlidingMoveBitboard(board, PieceType::Queen, turn);
		auto queenMoveList = GenerateMovesFromBitboard(board, queenMoves, PieceType::Queen, turn);
		moves.insert(moves.end(), queenMoveList.begin(), queenMoveList.end());

		// King moves
		uint64_t kingMoves = GenerateKingMoveBitboard(board, turn);
		auto kingMoveList = GenerateMovesFromBitboard(board, kingMoves, PieceType::King, turn);
		moves.insert(moves.end(), kingMoveList.begin(), kingMoveList.end());

		// Castling moves
		uint64_t castlingMoves = GenerateCastlingMoveBitboard(board, turn);
		auto castlingMoveList = GenerateMovesFromBitboard(board, castlingMoves, PieceType::King, turn);
		moves.insert(moves.end(), castlingMoveList.begin(), castlingMoveList.end());

		return moves;
	}

	std::vector<Move> MoveGenerator::GenerateMovesFromBitboard(const Board& board, uint64_t bitboard, PieceType pieceType, PieceColor turn)
	{
		std::vector<Move> moves;

		uint64_t pieces = board.GetPieceBitboard(turn, pieceType);

		while (pieces) {
			// Get the index of the current piece's square
			int sourceSquare = std::countr_zero(pieces);
			Tile source(sourceSquare);

			// Get the target squares for the current piece
			uint64_t targets = bitboard & board.GetMoveMask(sourceSquare, pieceType);

			while (targets) {
				int targetSquare = std::countr_zero(targets);
				Tile target(targetSquare);

				// Add the move to the list
				if (!(board.PlayerPieces() & (1ULL << targetSquare)))
					moves.emplace_back(board.BasicParseMove(source, target));

				// Clear the least significant bit in the targets
				targets &= targets - 1;
			}

			// Clear the least significant bit in the pieces
			pieces &= pieces - 1;
		}

		return moves;
	}

	std::vector<Move> MoveGenerator::GenerateLegalMoves(const Board& board, PieceColor turn)
	{
		std::vector<Move> moves = GeneratePseudoLegalMoves(board, turn);
		std::vector<Move> legalMoves;
		legalMoves.reserve(moves.size());

		for (const Move& move : moves)
		{
			Board tempBoard = board;
			tempBoard.ApplyMove(move);
			if (!tempBoard.IsCheck(tempBoard.GetTurn()))
				legalMoves.emplace_back(move);
		}

		return legalMoves;
	}

	uint64_t MoveGenerator::AttackBitboard(const Board& board, PieceColor turn)
	{
		uint64_t attacks = 0;
		attacks |= GeneratePawnCaptureBitboard(board, turn);
		attacks |= GenerateEnPassantMoveBitboard(board, turn);
		attacks |= GenerateKnightMoveBitboard(board, turn);
		attacks |= GenerateSlidingMoveBitboard(board, PieceType::Bishop, turn);
		attacks |= GenerateSlidingMoveBitboard(board, PieceType::Rook, turn);
		attacks |= GenerateSlidingMoveBitboard(board, PieceType::Queen, turn);
		attacks |= GenerateKingMoveBitboard(board, turn);
		return attacks;
	}

	uint64_t MoveGenerator::AttackBitboard(const Board& board, int square, PieceType pieceType, PieceColor turn)
	{
		switch (pieceType)
		{
		case PieceType::Pawn:
			return GeneratePawnMovesForSquare(board, square, turn);
		case PieceType::Knight:
			return s_KnightAttackMask[square];
		case PieceType::Bishop:
			return GenerateSlidingAttackBitboard(square, board.Occupied(), PieceType::Bishop);
		case PieceType::Rook:
			return GenerateSlidingAttackBitboard(square, board.Occupied(), PieceType::Rook);
		case PieceType::Queen:
			return GenerateSlidingAttackBitboard(square, board.Occupied(), PieceType::Queen);
		case PieceType::King:
			return s_KingAttackMask[square];
		default:
			return 0;
		}
	}

	uint64_t MoveGenerator::GeneratePawnMovesForSquare(const Board& board, int square, PieceColor turn)
	{
		uint64_t moves = 0;

		bool isWhiteTurn = turn == PieceColor::White;

		uint64_t pawns = board.Pawns(turn);
		uint64_t empty = ~board.Occupied();
		uint64_t opponentPieces = board.OpponentPieces();

		// Check if the square has a pawn of the current turn
		if (!(pawns & (1ULL << square)))
			return 0;

		// Single push
		if (isWhiteTurn)
		{
			if (square < 56 && (empty & (1ULL << (square + 8))))  // Single push
			{
				moves |= (1ULL << (square + 8));

				// Double push from rank 2
				if (square >= 8 && square < 16 && (empty & (1ULL << (square + 16))))
					moves |= (1ULL << (square + 16));
			}

			// Captures
			if (square < 56 && square % 8 != 0 && (opponentPieces & (1ULL << (square + 7))))  // Capture left
				moves |= (1ULL << (square + 7));
			if (square < 56 && square % 8 != 7 && (opponentPieces & (1ULL << (square + 9))))  // Capture right
				moves |= (1ULL << (square + 9));

			// En Passant
			Tile enPassantTarget = board.GetEnPassantTarget();
			if (enPassantTarget != Tile::None)
			{
				int enPassantSquare = enPassantTarget;
				if ((square % 8 != 0 && enPassantSquare == square + 7) ||  // Capture left
					(square % 8 != 7 && enPassantSquare == square + 9))    // Capture right
				{
					moves |= (1ULL << enPassantSquare);
				}
			}
		}
		else  // Black's turn
		{
			if (square >= 8 && (empty & (1ULL << (square - 8))))  // Single push
			{
				moves |= (1ULL << (square - 8));

				// Double push from rank 7
				if (square >= 48 && square < 56 && (empty & (1ULL << (square - 16))))
					moves |= (1ULL << (square - 16));
			}

			// Captures
			if (square >= 8 && square % 8 != 0 && (opponentPieces & (1ULL << (square - 9))))  // Capture left
				moves |= (1ULL << (square - 9));
			if (square >= 8 && square % 8 != 7 && (opponentPieces & (1ULL << (square - 7))))  // Capture right
				moves |= (1ULL << (square - 7));

			// En Passant
			Tile enPassantTarget = board.GetEnPassantTarget();
			if (enPassantTarget != Tile::None)
			{
				int enPassantSquare = enPassantTarget;
				if ((square % 8 != 0 && enPassantSquare == square - 9) ||  // Capture left
					(square % 8 != 7 && enPassantSquare == square - 7))    // Capture right
				{
					moves |= (1ULL << enPassantSquare);
				}
			}
		}

		// Promotion
		uint64_t promotionRank = isWhiteTurn ? 0xFF00000000000000ull : 0x00000000000000FFull;
		moves &= ~promotionRank;  // Remove promotion moves from the general list

		return moves;
	}

	void MoveGenerator::PrecomputeKingAttackMask()
	{
		for (int square = 0; square < 64; ++square)
		{
			uint64_t mask = 0ULL;
			int rank = square / 8;
			int file = square % 8;

			// Iterate over all 8 possible king moves
			for (int dr = -1; dr <= 1; ++dr)
			{
				for (int df = -1; df <= 1; ++df)
				{
					if (dr == 0 && df == 0) continue; // Skip the king's current square
					int newRank = rank + dr;
					int newFile = file + df;

					if (newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8)
					{
						int targetSquare = newRank * 8 + newFile;
						mask |= (1ULL << targetSquare);
					}
				}
			}
			s_KingAttackMask[square] = mask;
		}
	}

	void MoveGenerator::PrecomputeKnightAttackMask()
	{
		for (int square = 0; square < 64; ++square)
		{
			uint64_t mask = 0ULL;
			int rank = square / 8;
			int file = square % 8;

			// Iterate over all 8 possible knight moves
			int knightMoves[8][2] = {
				{2, 1}, {2, -1}, {-2, 1}, {-2, -1},
				{1, 2}, {1, -2}, {-1, 2}, {-1, -2}
			};

			for (const auto& move : knightMoves)
			{
				int newRank = rank + move[0];
				int newFile = file + move[1];

				if (newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8)
				{
					int targetSquare = newRank * 8 + newFile;
					mask |= (1ULL << targetSquare);
				}
			}
			s_KnightAttackMask[square] = mask;
		}
	}

}
