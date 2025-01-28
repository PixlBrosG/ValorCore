#include "vlpch.h"
#include "Valor/Chess/Board.h"

#include "Valor/Chess/MoveGeneration/MoveGenerator.h"
#include "Valor/Chess/MoveGeneration/MagicBitboard.h"

namespace Valor {

	void Board::Reset()
	{
		m_Turn = PieceColor::White;
		m_HalfmoveCounter = 0;
		m_EnPassantTarget = Tile::None;

		m_AllWhite = 0x000000000000FFFFull;
		m_AllBlack = 0xFFFF000000000000ull;

		m_Pawns = 0x00FF00000000FF00ull;
		m_Knights = 0x4200000000000042ull;
		m_Bishops = 0x2400000000000024ull;
		m_Rooks = 0x8100000000000081ull;
		m_Queens = 0x0800000000000008ull;
		m_Kings = 0x1000000000000010ull;
	}

	void Board::ApplyMove(const Move& move)
	{
		Tile source = move.GetSource();
		Tile target = move.GetTarget();

		uint64_t sourceBit = 1ULL << source;
		uint64_t targetBit = 1ULL << target;

		// Update castling rights
		UpdateCastlingRights(source, target);
		m_HalfmoveCounter = OpponentPieces() & targetBit || m_Pawns & sourceBit ? 0 : m_HalfmoveCounter + 1;

		// Move the piece
		Piece piece = GetPiece(source);
		RemovePiece(source);
		RemovePiece(target);
		PlacePiece(target, piece.Color, piece.Type);

		// Handle special moves
		if (move.IsEnPassant())
		{
			Tile enPassantTarget = GetTurn() == PieceColor::White ? Tile(target.Rank() - 1, target.File()) : Tile(target.Rank() + 1, target.File());
			RemovePiece(enPassantTarget);
		}
		else if (move.IsCastling())
		{
			Tile rookSource, rookTarget;
			if (target.File() == 2) // Queen-side castling
			{
				rookSource = Tile(target.Rank(), 0);
				rookTarget = Tile(target.Rank(), 3);
			}
			else // King-side castling
			{
				rookSource = Tile(target.Rank(), 7);
				rookTarget = Tile(target.Rank(), 5);
			}
			Piece rook = GetPiece(rookSource);
			RemovePiece(rookSource);
			RemovePiece(rookTarget);
			PlacePiece(rookTarget, rook.Color, rook.Type);
		}
		// Handle promotion
		if (move.IsPromotion())
		{
			Piece promotedPiece = Piece(static_cast<PieceType>(move.GetPromotion()), piece.Color);
			RemovePiece(target);
			PlacePiece(target, promotedPiece.Color, promotedPiece.Type);
		}
		// Update en passant target square
		if (piece.Type == PieceType::Pawn && std::abs(source.Rank() - target.Rank()) == 2)
			m_EnPassantTarget = GetTurn() == PieceColor::White ? Tile(target.Rank() - 1, target.File()) : Tile(target.Rank() + 1, target.File());
		else
			m_EnPassantTarget = Tile::None;
		// Toggle turn
		ToggleTurn();
	}

	void Board::RemovePiece(Tile tile)
	{
		uint64_t bit = 1ULL << tile;
		m_AllWhite &= ~bit;
		m_AllBlack &= ~bit;
		m_Pawns &= ~bit;
		m_Knights &= ~bit;
		m_Bishops &= ~bit;
		m_Rooks &= ~bit;
		m_Queens &= ~bit;
		m_Kings &= ~bit;
	}

	void Board::PlacePiece(Tile tile, PieceColor color, PieceType type)
	{
		uint64_t bit = 1ULL << tile;
		if (color == PieceColor::White)
			m_AllWhite |= bit;
		else
			m_AllBlack |= bit;
		switch (type)
		{
			case PieceType::Pawn:   m_Pawns |= bit; break;
			case PieceType::Knight: m_Knights |= bit; break;
			case PieceType::Bishop: m_Bishops |= bit; break;
			case PieceType::Rook:   m_Rooks |= bit; break;
			case PieceType::Queen:  m_Queens |= bit; break;
			case PieceType::King:   m_Kings |= bit; break;
		}
	}

	void Board::UpdateCastlingRights(Tile source, Tile target)
	{
		if (source == Tile(0, 4) || source == Tile(7, 4)) { // King moves
			m_CastlingRights[static_cast<size_t>(GetTurn()) - 1][0] = false; // Queen-side
			m_CastlingRights[static_cast<size_t>(GetTurn()) - 1][1] = false; // King-side
		}

		else if (source == Tile(0, 0) || target == Tile(0, 0)) // Rook moves (white queen-side)
			m_CastlingRights[static_cast<size_t>(PieceColor::White) - 1][0] = false;
		else if (source == Tile(0, 7) || target == Tile(0, 7)) // Rook moves (white king-side)
			m_CastlingRights[static_cast<size_t>(PieceColor::White) - 1][1] = false;

		else if (source == Tile(7, 0) || target == Tile(7, 0)) // Rook moves (black queen-side)
			m_CastlingRights[static_cast<size_t>(PieceColor::Black) - 1][0] = false;
		else if (source == Tile(7, 7) || target == Tile(7, 7)) // Rook moves (black king-side)
			m_CastlingRights[static_cast<size_t>(PieceColor::Black) - 1][1] = false;
	}

	uint64_t Board::GetPieceBitboard(PieceColor color, PieceType type) const
	{
		switch (type)
		{
			case PieceType::Pawn:   return Pawns(color);
			case PieceType::Knight: return Knights(color);
			case PieceType::Bishop: return Bishops(color);
			case PieceType::Rook:   return Rooks(color);
			case PieceType::Queen:  return Queens(color);
			case PieceType::King:   return Kings(color);
		}
	}

	uint64_t Board::GetMoveMask(int square, PieceType pieceType) const
	{
		switch (pieceType)
		{
		case PieceType::Pawn:
			return MoveGenerator::GeneratePawnMovesForSquare(*this, square);
		case PieceType::Knight:
			return MagicBitboard::s_KnightAttackMask[square];
		case PieceType::Bishop:
			return MagicBitboard::CalculateBishopAttacks(square, Occupied());
		case PieceType::Rook:
			return MagicBitboard::CalculateRookAttacks(square, Occupied());
		case PieceType::Queen:
			return MagicBitboard::CalculateRookAttacks(square, Occupied()) |
				MagicBitboard::CalculateBishopAttacks(square, Occupied());
		case PieceType::King:
			return MagicBitboard::s_KingAttackMask[square];
		default:
			return 0;
		}
	}

	Piece Board::GetPiece(Tile tile) const
	{
		uint64_t mask = 1ULL << tile;

		if (mask & m_AllWhite) {
			if (mask & m_Pawns) return Piece(PieceType::Pawn, PieceColor::White);
			if (mask & m_Knights) return Piece(PieceType::Knight, PieceColor::White);
			if (mask & m_Bishops) return Piece(PieceType::Bishop, PieceColor::White);
			if (mask & m_Rooks) return Piece(PieceType::Rook, PieceColor::White);
			if (mask & m_Queens) return Piece(PieceType::Queen, PieceColor::White);
			if (mask & m_Kings) return Piece(PieceType::King, PieceColor::White);
		}
		else if (mask & m_AllBlack) {
			if (mask & m_Pawns) return Piece(PieceType::Pawn, PieceColor::Black);
			if (mask & m_Knights) return Piece(PieceType::Knight, PieceColor::Black);
			if (mask & m_Bishops) return Piece(PieceType::Bishop, PieceColor::Black);
			if (mask & m_Rooks) return Piece(PieceType::Rook, PieceColor::Black);
			if (mask & m_Queens) return Piece(PieceType::Queen, PieceColor::Black);
			if (mask & m_Kings) return Piece(PieceType::King, PieceColor::Black);
		}

		return Piece(PieceType::None, PieceColor::None);
	}

	bool Board::IsSquareAttacked(int square) const
	{
		uint64_t attacks = MoveGenerator::AttackBitboard(*this);
		return attacks & (1ULL << square);
	}

	std::vector<Move> Board::GenerateLegalMoves() const
	{
		return MoveGenerator::GenerateLegalMoves(*this);
	}

}
