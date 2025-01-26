#pragma once
#include "Valor/Chess/Piece.h"

#include "Valor/Chess/Tile.h"
#include "Valor/Chess/Move.h"

#include <cstdint>
#include <ostream>

namespace Valor {

	class Board
	{
	public:
		Board();
		~Board() = default;

		void Reset();
		void UpdateBitBoards();

		const Piece& GetPiece(Tile tile) const { return m_Board[tile.Rank][tile.File]; }
		const Piece& GetPiece(int rank, int file) const { return m_Board[rank][file]; }

		void SetPiece(Tile tile, const Piece& piece) { m_Board[tile.Rank][tile.File] = piece; }
		void SetPiece(int rank, int file, const Piece& piece) { m_Board[rank][file] = piece; }

		void MovePiece(const Move& move);

		Tile FindKing(PieceColor color) const;

		void SetLastMove(const Move& move) { m_LastMove = move; }
		const Move& GetLastMove() const { return m_LastMove; }

		size_t GetHash() const;
	private:
		Piece m_Board[8][8];
		uint64_t m_BitBoards[2][6];

		Move m_LastMove;
	};

};

namespace std {

	inline std::ostream& operator<<(std::ostream& os, const Valor::Board& board) {
		// Constants for box-drawing characters
		const std::string topEdge = "  #========================#";
		const std::string midEdge = "  |------------------------|";
		const std::string bottomEdge = "  #========================#";
		const std::string rankSeparator = "|";
		const std::string fileLabels = "    A  B  C  D  E  F  G  H";

		// Get the last move
		const Valor::Move& lastMove = board.GetLastMove();

		// Print the top edge of the board
		os << "\033[1;40m\033[1;37m" << topEdge << "\033[0m" << std::endl;

		// Print the board rows
		for (int rank = 7; rank >= 0; rank--) {
			os << "\033[1;40m\033[1;37m " << (rank + 1) << rankSeparator;  // Rank number

			for (int file = 0; file < 8; file++) {
				// Determine background color for the tile
				bool isDarkSquare = (rank + file) % 2 != 0;
				std::string bgColor = isDarkSquare ? "\033[48;5;235m" : "\033[48;5;15m";  // Dark gray / Bright white

				// Highlight the source and target tiles of the last move
				Valor::Tile currentTile(rank, file);
				if (currentTile == lastMove.Source) {
					bgColor = "\033[48;5;44m";  // Light cyan for the source tile
				}
				else if (currentTile == lastMove.Target) {
					bgColor = "\033[48;5;42m";  // Light green for the target tile
				}

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
