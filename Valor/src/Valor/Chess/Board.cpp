#include "vlpch.h"
#include "Valor/Chess/Board.h"

#include "Valor/Chess/MoveGeneration/MagicBitboard.h"
#include "Valor/Chess/MoveGeneration/MoveGeneratorSimple.h"

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

		m_CastlingRights = { true, true, true, true };
	}

	MoveInfo Board::ParseMove(Tile source, Tile target) const
	{
		MoveInfo move;
		move.Source = source;
		move.Target = target;

		Piece piece = GetPiece(source);
		move.Piece = piece.Type;

		// Determine if the move is a capture
		if (IsOccupied(target) && (piece.Color != PieceColor::White ? m_IsWhiteTurn : !m_IsWhiteTurn))
		{
			move.Flags |= MoveFlags::Capture;
			move.CapturedPiece = piece.Type;
		}
		else if (IsOccupied(target))
		{
			std::cout << "Crap" << std::endl;
		}

		// Handle pawn-specific metadata
		if (piece.Type == PieceType::Pawn) {
			if (target.GetFile() == m_EnPassantFile) {
				move.Flags |= MoveFlags::EnPassant;
				move.CapturedPiece = PieceType::Pawn;
			}
			// Handle promotion (promotion rank: 7 for white, 0 for black)
			if (target.GetRank() == (m_IsWhiteTurn ? 7 : 0)) {
				move.Flags |= MoveFlags::Promotion;
				move.Promotion = PieceType::Queen; // NOTE: Default to queen promotion
			}
		}

		// Handle castling
		if (piece.Type == PieceType::King)
		{
			if (std::abs(target.GetFile() - source.GetFile()) == 2)
				move.Flags |= MoveFlags::Castling;
		}

		// Disambiguity (rank/file)
		uint8_t disambiguityRank = 0;
		uint8_t disambiguityFile = 0;
		if (IsAmbiguousMove(source, target, piece.Type))
			ResolveDisambiguity(source, target, piece.Type, disambiguityRank, disambiguityFile);

		// Simulate the move on a dummy board to check for legality
		Board simulatedBoard = *this; // Copy the current board
		simulatedBoard.MakeMove(move);

		// Check if the move causes check or checkmate
		if (simulatedBoard.IsCheck()) {
			move.Flags |= MoveFlags::Check;
			if (simulatedBoard.IsCheckmate())
				move.Flags |= MoveFlags::Checkmate;
		}

		return move;
	}

	void Board::MakeMove(Move move)
	{
		uint64_t sourceBit = 1ULL << move.Source;
		uint64_t targetBit = 1ULL << move.Target;

		Piece piece = GetPiece(move.Source);

		// Store captured piece (for undo functionality, if needed)
		Piece capturedPiece = GetPiece(move.Target);

		// Update castling rights
		UpdateCastlingRights(move.Source, move.Target);
		m_HalfmoveCounter = (move.IsCapture() || piece.Type == PieceType::Pawn) ? 0 : m_HalfmoveCounter + 1;

		// Move the piece
		RemovePiece(move.Source);
		RemovePiece(move.Target); // Only matters if it's a capture
		PlacePiece(move.Target, piece.Color, piece.Type);

		// Handle en passant - isn't necessarily set, so check if it's a diagonal pawn move and the target square is empty
		if (move.IsEnPassant() || (piece.Type == PieceType::Pawn && move.Source.GetFile() != move.Target.GetFile() && capturedPiece.Type == PieceType::None))
		{
			uint8_t enPassantRank = move.Target.GetRank() + (m_IsWhiteTurn ? -1 : 1);
			Tile enPassantTarget(enPassantRank, move.Target.GetFile());
			RemovePiece(enPassantTarget);
		}

		// Handle castling - isn't necessarily set, so check if it's a king move and the distance is 2
		else if (move.IsCastling() || (piece.Type == PieceType::King && std::abs(move.Source.GetFile() - move.Target.GetFile()) == 2))
		{
			Tile rookSource, rookTarget;
			if (move.Target.GetFile() == 2)
			{
				rookSource = Tile(move.Target.GetRank(), 0);
				rookTarget = Tile(move.Target.GetRank(), 3);
			}
			else
			{
				rookSource = Tile(move.Target.GetRank(), 7);
				rookTarget = Tile(move.Target.GetRank(), 5);
			}
			Piece rook = GetPiece(rookSource);
			RemovePiece(rookSource);
			PlacePiece(rookTarget, rook.Color, rook.Type);
		}

		// Handle promotion - isn't necessarily set, so check if it's a pawn move to the promotion rank
		if (move.IsPromotion() || (piece.Type == PieceType::Pawn && (move.Target.GetRank() == 0 || move.Target.GetRank() == 7)))
		{
			// Default to queen unless specified
			PieceType promotion = move.Promotion == PieceType::None ? PieceType::Queen : move.Promotion;

			RemovePiece(move.Target);
			PlacePiece(move.Target, piece.Color, promotion);
		}

		// Update en passant target square
		if (piece.Type == PieceType::Pawn && std::abs(move.Source.GetRank() - move.Target.GetRank()) == 2) {
			m_EnPassantFile = move.Target.GetFile();
		}
		else {
			m_EnPassantFile = 0xFF; // No en passant available
		}

		// Toggle turn
		ToggleTurn();
	}

	bool Board::IsAmbiguousMove(Tile source, Tile target, PieceType pieceType) const
	{
		uint64_t occupancy = Occupied();
		uint64_t potentialSources = 0;

		switch (pieceType) {
		case PieceType::Rook:
			potentialSources = MagicBitboard::GetRookAttacks(target, occupancy) & Rooks(m_IsWhiteTurn);
			break;
		case PieceType::Bishop:
			potentialSources = MagicBitboard::GetBishopAttacks(target, occupancy) & Bishops(m_IsWhiteTurn);
			break;
		case PieceType::Queen:
			potentialSources = (MagicBitboard::GetRookAttacks(target, occupancy) |
				MagicBitboard::GetBishopAttacks(target, occupancy))
				& Queens(m_IsWhiteTurn);
			break;
		case PieceType::Knight:
			potentialSources = MagicBitboard::GetKnightAttacks(target) & Knights(m_IsWhiteTurn);
			break;
		case PieceType::King:
			potentialSources = MagicBitboard::GetKingAttacks(target) & Kings(m_IsWhiteTurn);
			break;
		case PieceType::Pawn:
			potentialSources = (m_IsWhiteTurn ?
				MagicBitboard::GetWhitePawnAttacks(target) :
				MagicBitboard::GetBlackPawnAttacks(target))
				& Pawns(m_IsWhiteTurn);
			break;
		default:
			return false;
		}

		// More than one piece of the same type can move to `target`
		return (potentialSources & ~(1ULL << source)) != 0;
	}

	void Board::ResolveDisambiguity(Tile source, Tile target, PieceType pieceType,
		uint8_t& disambiguityRank, uint8_t& disambiguityFile) const
	{
		uint64_t occupancy = Occupied();
		uint64_t potentialSources = 0;

		switch (pieceType) {
		case PieceType::Rook:
			potentialSources = MagicBitboard::GetRookAttacks(target, occupancy) & Rooks(m_IsWhiteTurn);
			break;
		case PieceType::Bishop:
			potentialSources = MagicBitboard::GetBishopAttacks(target, occupancy) & Bishops(m_IsWhiteTurn);
			break;
		case PieceType::Queen:
			potentialSources = (MagicBitboard::GetRookAttacks(target, occupancy) |
				MagicBitboard::GetBishopAttacks(target, occupancy))
				& Queens(m_IsWhiteTurn);
			break;
		case PieceType::Knight:
			potentialSources = MagicBitboard::GetKnightAttacks(target) & Knights(m_IsWhiteTurn);
			break;
		default:
			return;
		}

		// Exclude the source square itself
		potentialSources &= ~(1ULL << source);

		// Count how many remaining pieces can move to `target`
		int count = std::popcount(potentialSources);
		if (count == 0) {
			disambiguityRank = 0;
			disambiguityFile = 0;
			return;
		}

		// Check if multiple pieces share the same rank/file
		bool sameRank = false, sameFile = false;
		for (uint64_t temp = potentialSources; temp; temp &= temp - 1) {
			int square = std::countr_zero(temp);
			if (Tile(square).GetRank() == source.GetRank()) sameRank = true;
			if (Tile(square).GetFile() == source.GetFile()) sameFile = true;
		}

		// Assign disambiguation flags
		disambiguityRank = sameFile ? source.GetRank() : 0;
		disambiguityFile = sameRank ? source.GetFile() : 0;
	}

	void Board::RemovePiece(Tile tile)
	{
		uint64_t mask = ~(1ULL << tile);
		m_AllWhite &= mask;
		m_AllBlack &= mask;
		m_Pawns &= mask;
		m_Knights &= mask;
		m_Bishops &= mask;
		m_Rooks &= mask;
		m_Queens &= mask;
		m_Kings &= mask;
	}

	void Board::PlacePiece(Tile tile, PieceColor color, PieceType type)
	{
		RemovePiece(tile);

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
			m_CastlingRights[2] = false;
		else if (source == Tiles::H8 || target == Tiles::H8) // Rook moves (black king-side)
			m_CastlingRights[3] = false;
	}

	bool Board::IsInsufficientMaterial() const
	{
		int whitePieces = std::popcount(m_AllWhite);
		int blackPieces = std::popcount(m_AllBlack);

		bool whiteHasBishop = (m_Bishops & m_AllWhite) != 0;
		bool whiteHasKnight = (m_Knights & m_AllWhite) != 0;
		bool blackHasBishop = (m_Bishops & m_AllBlack) != 0;
		bool blackHasKnight = (m_Knights & m_AllBlack) != 0;

		if (whitePieces <= 1 && blackPieces <= 1) return true;
		if (whitePieces == 1 && blackPieces == 2 && blackHasBishop) return true;
		if (whitePieces == 1 && blackPieces == 2 && blackHasKnight) return true;
		if (blackPieces == 2 && whitePieces == 1 && whiteHasBishop) return true;
		if (blackPieces == 2 && whitePieces == 1 && whiteHasKnight) return true;

		return false;
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
			default: return 0;
		}
	}

	uint64_t Board::GetPieceBitboard(PieceType type) const
	{
		switch (type)
		{
			case PieceType::Pawn:   return Pawns();
			case PieceType::Knight: return Knights();
			case PieceType::Bishop: return Bishops();
			case PieceType::Rook:   return Rooks();
			case PieceType::Queen:  return Queens();
			case PieceType::King:   return Kings();
			default: return 0;
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

	int Board::GetKingSquare(bool isWhite) const
	{
		uint64_t kingBitboard = Kings(isWhite);
		return std::countr_zero(kingBitboard);
	}

	bool Board::IsCheck(bool isDefending) const
	{
		int kingSquare = GetKingSquare(isDefending ? m_IsWhiteTurn : !m_IsWhiteTurn);
		return IsSquareAttacked(Tile(kingSquare), isDefending ? m_IsWhiteTurn : !m_IsWhiteTurn);
	}

	bool Board::IsCheckmate() const
	{
		std::vector<Move> legalMoves = MoveGeneratorSimple::GenerateLegalMoves(*this);
		return legalMoves.empty() && IsCheck(true);
	}

	bool Board::IsStalemate() const
	{
		std::vector<Move> legalMoves = MoveGeneratorSimple::GenerateLegalMoves(*this);
		return legalMoves.empty() && !IsCheck(true);
	}

	bool Board::IsLegalMove(Move move) const
	{
		std::vector<Move> legalMoves = MoveGeneratorSimple::GenerateLegalMoves(*this);
		return std::find(legalMoves.begin(), legalMoves.end(), move) != legalMoves.end();
	}

	bool Board::IsSquareAttacked(Tile square, bool isWhite) const
	{
		uint64_t occupancy = Occupied();
		uint64_t enemyPieces = isWhite ? m_AllBlack : m_AllWhite;

		if (MagicBitboard::GetRookAttacks(square, occupancy) & (m_Rooks | m_Queens) & enemyPieces)
			return true;

		if (MagicBitboard::GetBishopAttacks(square, occupancy) & (m_Bishops | m_Queens) & enemyPieces)
			return true;

		if (MagicBitboard::GetKnightAttacks(square) & m_Knights & enemyPieces)
			return true;

		if (MagicBitboard::GetKingAttacks(square) & m_Kings & enemyPieces)
			return true;

		// Check for pawn attacks
		uint64_t pawnAttacks = isWhite ? MagicBitboard::GetWhitePawnAttacks(square)
			: MagicBitboard::GetBlackPawnAttacks(square);
		if (pawnAttacks & m_Pawns & enemyPieces)
			return true;

		return false;
	}

}
