#pragma once
#include "Valor/Chess/Piece.h"

#include "Valor/Chess/Tile.h"
#include "Valor/Chess/Move.h"

#include <cstdint>
#include <ostream>
#include <array>
#include <bit>
#include <vector>

namespace Valor {

	struct Board
	{
	public:
		Board();
		Board(const Board& other) = default;
		~Board() = default;

		void Reset();

		Move ParseMove(Tile source, Tile target) const;
		void ApplyMove(const Move& move);
		bool IsLegalMove(const Move& move) const;

		bool IsAmbiguousMove(Tile source, Tile target, PieceType pieceType) const;
		void ResolveDisambiguity(Tile source, Tile target, PieceType pieceType, uint8_t& disambiguityRank, uint8_t& disambiguityFile) const;

		void RemovePiece(Tile tile);
		void PlacePiece(Tile tile, PieceColor color, PieceType type);

		bool IsWhiteTurn() const { return m_IsWhiteTurn; }
		void ToggleTurn() { m_IsWhiteTurn ^= 1; }

		void UpdateCastlingRights(Tile source, Tile target);
		bool IsFiftyMoveRule() const { return m_HalfmoveCounter >= 100; }

		uint8_t GetEnPassantFile() const { return m_EnPassantFile; }
		bool GetCastlingRights(bool isWhite, bool kingSide) const { return m_CastlingRights[(isWhite ? 0ull : 2ull) + kingSide]; }

		uint64_t GetMoveMask(int square, PieceType type) const;

		Piece GetPiece(Tile tile) const;
		Piece GetPiece(int rank, int file) const { return GetPiece(Tile(rank, file)); }

		uint64_t Occupied() const { return m_AllWhite | m_AllBlack; }
		bool IsOccupied(Tile tile) const { return Occupied() & (1ULL << tile); }

		uint64_t GetPieceBitboard(bool isWhite, PieceType type) const;

		uint64_t AllPieces(bool isWhite) const { return isWhite ? m_AllWhite : m_AllBlack; }
		uint64_t WhitePieces() const { return m_AllWhite; }
		uint64_t BlackPieces() const { return m_AllBlack; }

		uint64_t PlayerPieces() const { return m_IsWhiteTurn ? m_AllWhite : m_AllBlack; }
		uint64_t OpponentPieces() const { return m_IsWhiteTurn ? m_AllBlack : m_AllWhite; }

		uint64_t Pawns() const { return m_Pawns; }
		uint64_t Knights() const { return m_Knights; }
		uint64_t Bishops() const { return m_Bishops; }
		uint64_t Rooks() const { return m_Rooks; }
		uint64_t Queens() const { return m_Queens; }
		uint64_t Kings() const { return m_Kings; }

		uint64_t Pawns(bool isWhite) const { return m_Pawns & (isWhite ? m_AllWhite : m_AllBlack); }
		uint64_t Knights(bool isWhite) const { return m_Knights & (isWhite ? m_AllWhite : m_AllBlack); }
		uint64_t Bishops(bool isWhite) const { return m_Bishops & (isWhite ? m_AllWhite : m_AllBlack); }
		uint64_t Rooks(bool isWhite) const { return m_Rooks & (isWhite ? m_AllWhite : m_AllBlack); }
		uint64_t Queens(bool isWhite) const { return m_Queens & (isWhite ? m_AllWhite : m_AllBlack); }
		uint64_t Kings(bool isWhite) const { return m_Kings & (isWhite ? m_AllWhite : m_AllBlack); }

		int KingSquare(bool isWhite) const { return std::countr_zero(Kings(isWhite)); }

		bool IsSquareAttacked(int square, bool isWhite) const;

		bool IsCheck(bool isWhite) const;
		bool IsCheckmate(bool isWhite = true) const { return IsCheck(isWhite) && GenerateLegalMoves().empty(); }
		bool IsStalemate(bool isWhite = true) const { return !IsCheck(isWhite) && GenerateLegalMoves().empty(); }
	public:
		constexpr static uint64_t FileA = 0x0101010101010101ull;
		constexpr static uint64_t FileH = 0x8080808080808080ull;
	private:
		std::vector<Move> GenerateLegalMoves() const;
	private:
		uint64_t m_AllWhite, m_AllBlack;
		uint64_t m_Pawns, m_Knights, m_Bishops, m_Rooks, m_Queens, m_Kings;
		
		bool m_IsWhiteTurn;
		uint8_t m_EnPassantFile;
		std::array<bool, 4> m_CastlingRights;  // [white/black][king/queen]

		uint8_t m_HalfmoveCounter;
	};

};

namespace std {

	inline std::ostream& operator<<(std::ostream& os, const Valor::Board& board)
	{
		// Constants for box-drawing characters
		const std::string topEdge = "  #========================#";
		const std::string midEdge = "  |------------------------|";
		const std::string bottomEdge = "  #========================#";
		const std::string rankSeparator = "|";
		const std::string fileLabels = "    A  B  C  D  E  F  G  H";

		// Print the top edge of the board
		os << "\033[1;40m\033[1;37m" << topEdge << "\033[0m" << std::endl;

		// Print the board rows
		for (int rank = 7; rank >= 0; rank--) {
			os << "\033[1;40m\033[1;37m " << (rank + 1) << rankSeparator;  // Rank number

			for (int file = 0; file < 8; file++) {
				// Determine background color for the tile
				bool isDarkSquare = (rank + file) % 2 == 0;
				std::string bgColor = isDarkSquare ? "\033[48;5;235m" : "\033[48;5;15m";  // Dark gray / Bright white

				// Get the piece and determine its color
				const Valor::Piece& piece = board.GetPiece(rank, file);
				std::string pieceColor = piece.Color == Valor::PieceColor::White ? "\033[1;96m" : "\033[1;90m";  // Bright cyan for white, dark gray for black

				// Print the tile with the piece
				os << bgColor << pieceColor;
				os << ' ' << piece.ToChar() << ' ';  // Piece character centered
			}

			os << "\033[1;40m\033[1;37m" << rankSeparator << (rank + 1) << "\033[0m" << std::endl;  // Rank number again

			// Print middle edge except for the last row
			if (rank > 0) {
				os << "\033[1;40m\033[1;37m" << midEdge << "\033[0m" << std::endl;
			}
		}

		// Print the bottom edge of the board
		os << "\033[1;40m\033[1;37m" << bottomEdge << "\033[0m" << std::endl;

		// Print the file labels
		os << "\033[1;40m\033[1;37m" << fileLabels << "\033[0m" << std::endl;

		return os;
	}

}
