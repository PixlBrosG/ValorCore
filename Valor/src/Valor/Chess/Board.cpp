#include "vlpch.h"
#include "Valor/Chess/Board.h"

#include "Valor/Chess/MoveGeneration/MoveGenerator.h"
#include "Valor/Chess/MoveGeneration/MagicBitboard.h"

namespace Valor {

	Board::Board()
		: m_Turn(PieceColor::White), m_EnPassantTarget(Tile::None)
	{
		Reset();
	}

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

	Move Board::BasicParseMove(Tile source, Tile target) const
	{
		// Only checks for legality, does not handle ambiguity, check, checkmate, etc.
		uint16_t flags = 0;
		PieceType promotion = PieceType::None;
		PieceType pieceType = GetPiece(source).Type;
		PieceType capturedPiece = PieceType::None;

		// Determine if the move is a capture
		if (IsOccupied(target) && GetPiece(target).Color != GetTurn()) {
			flags |= Move::captureFlag;
			capturedPiece = GetPiece(target).Type;
		}

		// Handle pawn-specific metadata
		if (pieceType == PieceType::Pawn) {
			if (target == m_EnPassantTarget) {
				flags |= Move::enPassantFlag;
				capturedPiece = PieceType::Pawn;
			}
			// Handle promotion (promotion rank: 7 for white, 0 for black)
			if (target.GetRank() == (GetTurn() == PieceColor::White ? 7 : 0)) {
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

		return Move(source, target, flags, promotion, pieceType, capturedPiece);
	}

	Move Board::ParseMove(Tile source, Tile target) const
	{
		uint16_t flags = 0;
		PieceType promotion = PieceType::None;
		PieceType pieceType = GetPiece(source).Type;
		PieceType capturedPiece = PieceType::None;

		// Determine if the move is a capture
		if (IsOccupied(target) && GetPiece(target).Color != GetTurn()) {
			flags |= Move::captureFlag;
			capturedPiece = GetPiece(target).Type;
		}

		// Handle pawn-specific metadata
		if (pieceType == PieceType::Pawn) {
			if (target == m_EnPassantTarget) {
				flags |= Move::enPassantFlag;
				capturedPiece = PieceType::Pawn;
			}
			// Handle promotion (promotion rank: 7 for white, 0 for black)
			if (target.GetRank() == (GetTurn() == PieceColor::White ? 7 : 0)) {
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
		if (simulatedBoard.IsCheck(simulatedBoard.GetTurn())) {
			flags |= Move::checkFlag;
			if (simulatedBoard.IsCheckmate(simulatedBoard.GetTurn())) {
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
			Tile enPassantTarget = GetTurn() == PieceColor::White ? Tile(target.GetRank() - 1, target.GetFile()) : Tile(target.GetRank() + 1, target.GetFile());
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
			m_EnPassantTarget = GetTurn() == PieceColor::White ? Tile(target.GetRank() - 1, target.GetFile()) : Tile(target.GetRank() + 1, target.GetFile());
		else
			m_EnPassantTarget = Tile::None;
		// Toggle turn
		ToggleTurn();
	}

	bool Board::IsLegalMove(const Move& move) const
	{
		std::vector<Move> moves = MoveGenerator::GenerateLegalMoves(*this, m_Turn);
		for (const Move& legalMove : moves)
		{
			if (move.GetSource() == legalMove.GetSource() && move.GetTarget() == legalMove.GetTarget())
				return true;
		}
		return false;
	}

	bool Board::IsAmbiguousMove(Tile source, Tile target, PieceType pieceType) const
	{
		uint64_t sameTypePieces = GetPieceBitboard(GetTurn(), pieceType);

		sameTypePieces &= ~(1ULL << source);

		uint64_t attackMask = MoveGenerator::AttackBitboard(*this, target, pieceType, m_Turn);

		return (attackMask & sameTypePieces) != 0;
	}


	void Board::ResolveDisambiguity(Tile source, Tile target, PieceType pieceType, uint8_t& disambiguityRank, uint8_t& disambiguityFile) const
	{
		disambiguityRank = 0;
		disambiguityFile = 0;

		uint64_t sameTypePieces = GetPieceBitboard(GetTurn(), pieceType);

		sameTypePieces &= ~(1ULL << source);

		uint64_t attackMask = MoveGenerator::AttackBitboard(*this, target, pieceType, m_Turn);

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
			return MoveGenerator::GeneratePawnMovesForSquare(*this, square, m_Turn);
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

	bool Board::IsSquareAttacked(int square, PieceColor attacker) const
	{
		// Pawns
		uint64_t pawnAttacks = (attacker == PieceColor::White)
			? ((Pawns(attacker) << 7) & ~FileH) | ((Pawns(attacker) << 9) & ~FileA)
			: ((Pawns(attacker) >> 7) & ~FileA) | ((Pawns(attacker) >> 9) & ~FileH);
		if (pawnAttacks & (1ULL << square)) return true;

		// Knights
		if (MoveGenerator::GetKnightMoves(square) & Knights(attacker)) return true;

		// Kings
		if (MoveGenerator::GetKingMoves(square) & Kings(attacker)) return true;

		// Sliding Pieces
		uint64_t blockers = Occupied();
		if (MagicBitboard::GetSlidingAttack(square, blockers, PieceType::Rook) & Rooks(attacker)) return true;
		if (MagicBitboard::GetSlidingAttack(square, blockers, PieceType::Bishop) & Bishops(attacker)) return true;
		if (MagicBitboard::GetSlidingAttack(square, blockers, PieceType::Queen) & Queens(attacker)) return true;

		return false;
	}


	bool Board::IsCheck(PieceColor attacker) const
	{
		return IsSquareAttacked(KingSquare(SWAP_COLOR(attacker)), attacker);
	}

	std::vector<Move> Board::GenerateLegalMoves() const
	{
		return MoveGenerator::GenerateLegalMoves(*this, m_Turn);
	}

}
