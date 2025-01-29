#include "vlpch.h"
#include "Valor/Chess/Board.h"

#include "Valor/Chess/MoveGeneration/MoveGenerator.h"
#include "Valor/Chess/MoveGeneration/MagicBitboard.h"

namespace Valor {

	Board::Board()
		: m_IsWhiteTurn(true), m_EnPassantFile(0xff), m_HalfmoveCounter(0)
	{
		Reset();
	}

	void Board::Reset()
	{
		m_IsWhiteTurn = true;
		m_HalfmoveCounter = 0;
		m_EnPassantFile = 0xff;

		m_AllWhite = 0x000000000000FFFFull;
		m_AllBlack = 0xFFFF000000000000ull;

		m_Pawns = 0x00FF00000000FF00ull;
		m_Knights = 0x4200000000000042ull;
		m_Bishops = 0x2400000000000024ull;
		m_Rooks = 0x8100000000000081ull;
		m_Queens = 0x0800000000000008ull;
		m_Kings = 0x1000000000000010ull;
	}

	Move Board::ParseMove(Tile source, Tile target) const
	{
		uint16_t flags = 0;
		PieceType promotion = PieceType::None;
		PieceType pieceType = GetPiece(source).Type;
		PieceType capturedPiece = PieceType::None;

		// Determine if the move is a capture
		if (IsOccupied(target) && (GetPiece(target).Color != PieceColor::White ? m_IsWhiteTurn : !m_IsWhiteTurn))
		{
			flags |= Move::captureFlag;
			capturedPiece = GetPiece(target).Type;
		}
		else if (IsOccupied(target))
		{
			std::cout << "Crap" << std::endl;
		}

		// Handle pawn-specific metadata
		if (pieceType == PieceType::Pawn) {
			if (target.GetFile() == m_EnPassantFile) {
				flags |= Move::enPassantFlag;
				capturedPiece = PieceType::Pawn;
			}
			// Handle promotion (promotion rank: 7 for white, 0 for black)
			if (target.GetRank() == (m_IsWhiteTurn ? 7 : 0)) {
				flags |= Move::promotionFlag;
				promotion = PieceType::Queen; // Default to queen; adjust if needed.
			}
		}

		// Handle castling
		if (pieceType == PieceType::King) {
			if (std::abs(target.GetFile() - source.GetFile()) == 2) {
				flags |= Move::castlingFlag;
			}
		}

		// Disambiguity (rank/file)
		uint8_t disambiguityRank = 0;
		uint8_t disambiguityFile = 0;
		if (IsAmbiguousMove(source, target, pieceType)) {
			ResolveDisambiguity(source, target, pieceType, disambiguityRank, disambiguityFile);
		}

		// Simulate the move on a dummy board to check for legality
		Board simulatedBoard = *this; // Copy the current board
		Move move(source, target, flags, promotion, pieceType, capturedPiece, disambiguityRank, disambiguityFile);
		simulatedBoard.ApplyMove(move);

		// Check if the move causes check or checkmate
		if (simulatedBoard.IsCheck(simulatedBoard.IsWhiteTurn())) {
			flags |= Move::checkFlag;
			if (simulatedBoard.IsCheckmate()) {
				flags |= Move::checkmateFlag;
			}
		}

		return Move(source, target, flags, promotion, pieceType, capturedPiece, disambiguityRank, disambiguityFile);
	}

	void Board::ApplyMove(const Move& move)
	{
		Tile source = move.GetSource();
		Tile target = move.GetTarget();

		uint64_t sourceBit = 1ULL << source;
		uint64_t targetBit = 1ULL << target;

		// Update castling rights
		UpdateCastlingRights(source, target);
		m_HalfmoveCounter = move.IsCapture() || GetPiece(source).Type == PieceType::Pawn ? 0 : m_HalfmoveCounter + 1;

		// Move the piece
		Piece piece = GetPiece(source);
		RemovePiece(source);
		RemovePiece(target);
		PlacePiece(target, piece.Color, piece.Type);

		// Handle special moves
		if (move.IsEnPassant())
		{
			uint8_t enPassantRank = target.GetRank() + (m_IsWhiteTurn ? -1 : 1);
			Tile enPassantTarget(enPassantRank, target.GetFile());
			RemovePiece(enPassantTarget);
		}
		else if (move.IsCastling())
		{
			Tile rookSource, rookTarget;
			if (target.GetFile() == 2) // Queen-side castling
			{
				rookSource = Tile(target.GetRank(), 0);
				rookTarget = Tile(target.GetRank(), 3);
			}
			else // King-side castling
			{
				rookSource = Tile(target.GetRank(), 7);
				rookTarget = Tile(target.GetRank(), 5);
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
		if (piece.Type == PieceType::Pawn && std::abs(source.GetRank() - target.GetRank()) == 2)
			m_EnPassantFile = target.GetFile();
		else
			m_EnPassantFile = 0xff;
		// Toggle turn
		ToggleTurn();
	}

	bool Board::IsLegalMove(const Move& move) const
	{
		std::vector<Move> moves = MoveGenerator::GenerateLegalMoves(*this, m_IsWhiteTurn);
		for (const Move& legalMove : moves)
		{
			if (move.GetSource() == legalMove.GetSource() && move.GetTarget() == legalMove.GetTarget())
				return true;
		}
		return false;
	}

	bool Board::IsAmbiguousMove(Tile source, Tile target, PieceType pieceType) const
	{
		uint64_t sameTypePieces = GetPieceBitboard(m_IsWhiteTurn, pieceType);

		sameTypePieces &= ~(1ULL << source);

		uint64_t attackMask = MoveGenerator::AttackBitboard(*this, target, pieceType, m_IsWhiteTurn);

		return (attackMask & sameTypePieces) != 0;
	}


	void Board::ResolveDisambiguity(Tile source, Tile target, PieceType pieceType, uint8_t& disambiguityRank, uint8_t& disambiguityFile) const
	{
		disambiguityRank = 0;
		disambiguityFile = 0;

		uint64_t sameTypePieces = GetPieceBitboard(m_IsWhiteTurn, pieceType);

		sameTypePieces &= ~(1ULL << source);

		uint64_t attackMask = MoveGenerator::AttackBitboard(*this, target, pieceType, m_IsWhiteTurn);

		uint64_t attackers = attackMask & sameTypePieces;

		while (attackers) {
			int square = std::countr_zero(attackers);
			attackers &= attackers - 1;

			Tile attackerTile(square / 8, square % 8);

			// Check if this attacker shares the rank or file with the source
			if (attackerTile.GetRank() == source.GetRank()) disambiguityFile = 1;
			if (attackerTile.GetFile() == source.GetFile()) disambiguityRank = 1;
		}
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
		if (source == Tiles::E1 || source == Tiles::E8)
		{
			m_CastlingRights[m_IsWhiteTurn ? 0 : 2] = false; // Queen-side
			m_CastlingRights[m_IsWhiteTurn ? 1 : 3] = false; // King-side
		}

		else if (source == Tiles::A1 || target == Tiles::A1) // Rook moves (white queen-side)
			m_CastlingRights[0] = false;
		else if (source == Tiles::H1 || target == Tiles::H1) // Rook moves (white king-side)
			m_CastlingRights[1] = false;

		else if (source == Tiles::A8 || target == Tiles::A8) // Rook moves (black queen-side)
			m_CastlingRights[3] = false;
		else if (source == Tiles::H8 || target == Tiles::H8) // Rook moves (black king-side)
			m_CastlingRights[4] = false;
	}

	uint64_t Board::GetPieceBitboard(bool isWhiteturn, PieceType type) const
	{
		switch (type)
		{
			case PieceType::Pawn:   return Pawns(isWhiteturn);
			case PieceType::Knight: return Knights(isWhiteturn);
			case PieceType::Bishop: return Bishops(isWhiteturn);
			case PieceType::Rook:   return Rooks(isWhiteturn);
			case PieceType::Queen:  return Queens(isWhiteturn);
			case PieceType::King:   return Kings(isWhiteturn);
		}
	}

	uint64_t Board::GetMoveMask(int square, PieceType pieceType) const
	{
		switch (pieceType)
		{
		case PieceType::Pawn:
			return MoveGenerator::GeneratePawnMovesForSquare(*this, square, m_IsWhiteTurn);
		case PieceType::Knight:
			return MagicBitboard::s_KnightAttackMask[square];
		case PieceType::Bishop:
			return MagicBitboard::CalculateBishopAttacks(square, Occupied());
		case PieceType::Rook:
			return MagicBitboard::CalculateRookAttacks(square, Occupied());
		case PieceType::Queen:
			return MagicBitboard::CalculateRookAttacks(square, Occupied())
				| MagicBitboard::CalculateBishopAttacks(square, Occupied());
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

	bool Board::IsSquareAttacked(int square, bool isWhiteturn) const
	{
		uint64_t pawns = Pawns(isWhiteturn);
		uint64_t queens = Queens(isWhiteturn);

		// Pawns
		uint64_t pawnAttacks = isWhiteturn
			? ((pawns << 7) & ~FileH) | ((pawns << 9) & ~FileA)
			: ((pawns >> 7) & ~FileA) | ((pawns >> 9) & ~FileH);
		if (pawnAttacks & (1ULL << square)) return true;

		// Knights
		if (MoveGenerator::GetKnightMoves(square) & Knights(isWhiteturn)) return true;

		// Kings
		if (MoveGenerator::GetKingMoves(square) & Kings(isWhiteturn)) return true;

		// Sliding Pieces
		uint64_t blockers = Occupied();
		if (MagicBitboard::CalculateRookAttacks(square, blockers) & (Rooks(isWhiteturn) | queens)) return true;
		if (MagicBitboard::CalculateBishopAttacks(square, blockers) & (Bishops(isWhiteturn) | queens)) return true;

		return false;
	}

	bool Board::IsCheck(bool isWhite) const
	{
		return IsSquareAttacked(KingSquare(!isWhite), isWhite);
	}

	std::vector<Move> Board::GenerateLegalMoves() const
	{
		return MoveGenerator::GenerateLegalMoves(*this, m_IsWhiteTurn);
	}

}
