#include "vlpch.h"
#include "Valor/Chess/Board.h"

namespace Valor {

	Board::Board()
	{
		Reset();
	}

	void Board::Reset()
	{
		// Clear the board
		for (int rank = 0; rank < 8; rank++)
		{
			for (int file = 0; file < 8; file++)
			{
				m_Board[rank][file] = Piece(PieceType::None, PieceColor::None);
			}
		}

		// Set up the white pieces
		m_Board[0][0] = Piece(PieceType::Rook, PieceColor::White);
		m_Board[0][1] = Piece(PieceType::Knight, PieceColor::White);
		m_Board[0][2] = Piece(PieceType::Bishop, PieceColor::White);
		m_Board[0][3] = Piece(PieceType::Queen, PieceColor::White);
		m_Board[0][4] = Piece(PieceType::King, PieceColor::White);
		m_Board[0][5] = Piece(PieceType::Bishop, PieceColor::White);
		m_Board[0][6] = Piece(PieceType::Knight, PieceColor::White);
		m_Board[0][7] = Piece(PieceType::Rook, PieceColor::White);
		for (int file = 0; file < 8; file++)
		{
			m_Board[1][file] = Piece(PieceType::Pawn, PieceColor::White);
		}

		// Set up the black pieces
		m_Board[7][0] = Piece(PieceType::Rook, PieceColor::Black);
		m_Board[7][1] = Piece(PieceType::Knight, PieceColor::Black);
		m_Board[7][2] = Piece(PieceType::Bishop, PieceColor::Black);
		m_Board[7][3] = Piece(PieceType::Queen, PieceColor::Black);
		m_Board[7][4] = Piece(PieceType::King, PieceColor::Black);
		m_Board[7][5] = Piece(PieceType::Bishop, PieceColor::Black);
		m_Board[7][6] = Piece(PieceType::Knight, PieceColor::Black);
		m_Board[7][7] = Piece(PieceType::Rook, PieceColor::Black);
		for (int file = 0; file < 8; file++)
		{
			m_Board[6][file] = Piece(PieceType::Pawn, PieceColor::Black);
		}
	}

	void Board::UpdateBitBoards()
	{
		for (int color = 0; color < 2; color++)
		{
			for (int piece = 0; piece < 6; piece++)
			{
				m_BitBoards[color][piece] = 0;
			}
		}

		for (int rank = 0; rank < 8; rank++)
		{
			for (int file = 0; file < 8; file++)
			{
				const Piece& piece = m_Board[rank][file];
				if (piece.Color != PieceColor::None)
				{
					int colorIdx = piece.Color == PieceColor::White ? 0 : 1;
					int typeIdx = static_cast<int>(piece.Type) - 1; // PieceType::None is 0
					int index = rank * 8 + file;
					m_BitBoards[colorIdx][typeIdx] |= 1ULL << index;
				}
			}
		}
	}

	void Board::MovePiece(const Move& move)
	{
		m_LastMove = move;
		m_Board[move.Target.Rank][move.Target.File] = m_Board[move.Source.Rank][move.Source.File];
		m_Board[move.Source.Rank][move.Source.File] = Piece();
	}

	Tile Board::FindKing(PieceColor color) const
	{
		for (int rank = 0; rank < 8; rank++)
		{
			for (int file = 0; file < 8; file++)
			{
				const Piece& piece = m_Board[rank][file];
				if (piece.Type == PieceType::King && piece.Color == color)
					return { rank, file };
			}
		}
		return { -1, -1 };
	}

	size_t Board::GetHash() const
	{
		size_t hash = 0;
		for (int rank = 0; rank < 8; rank++)
		{
			for (int file = 0; file < 8; file++)
			{
				const Piece& piece = m_Board[rank][file];
				hash ^= static_cast<size_t>(piece.Type) << (rank * 8 + file);
			}
		}
		return hash;
	}

}
