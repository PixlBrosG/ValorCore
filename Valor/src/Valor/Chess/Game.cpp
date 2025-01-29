#include "vlpch.h"
#include "Valor/Chess/Game.h"

#include "Valor/Chess/MoveGeneration/MoveGenerator.h"

#include <iostream>
#include <algorithm>

namespace Valor {

	Game::Game()
	{
		Init();
	}

	void Game::Init()
	{
		m_Board = Board();
		m_MoveHistory.clear();
		m_BoardHistoryHashes.clear();
		m_BoardHistory.clear();
	}

	void Game::MakeMove(const Move& move)
	{
		m_MoveHistory.emplace_back(move);
		m_BoardHistory.emplace_back(m_Board);

		m_Board.ApplyMove(move);

		// Update the board history hash
		// TODO
//		size_t hash = m_Board.Hash();
//		m_BoardHistoryHashes[hash]++;
	}

	void Game::UndoMove()
	{
		if (m_BoardHistory.empty())
			return;

		// TODO: Update board history hash
		m_Board = m_BoardHistory.back();

		m_BoardHistory.pop_back();
		m_MoveHistory.pop_back();
	}

	bool Game::IsThreefoldRepetition() const
	{
		return std::any_of(m_BoardHistoryHashes.begin(), m_BoardHistoryHashes.end(),
			[](const std::pair<size_t, int>& pair) { return pair.second >= 3; });
	}

	bool Game::IsInsufficientMaterial() const
	{
		int whitePieces = 0, blackPieces = 0;
		bool whiteHasBishop = false, whiteHasKnight = false;
		bool blackHasBishop = false, blackHasKnight = false;

		for (int rank = 0; rank < 8; rank++)
		{
			for (int file = 0; file < 8; file++)
			{
				const Piece& piece = m_Board.GetPiece(rank, file);
				if (piece.Type != PieceType::None)
				{
					if (piece.Color == PieceColor::White)
					{
						whitePieces++;
						if (piece.Type == PieceType::Bishop)
							whiteHasBishop = true;
						else if (piece.Type == PieceType::Knight)
							whiteHasKnight = true;
					}
					else
					{
						blackPieces++;
						if (piece.Type == PieceType::Bishop)
							blackHasBishop = true;
						else if (piece.Type == PieceType::Knight)
							blackHasKnight = true;
					}
				}
			}
		}

		if (whitePieces <= 1 && blackPieces <= 1) return true;
		if (whitePieces == 1 && blackPieces == 2 && blackHasBishop) return true;
		if (whitePieces == 1 && blackPieces == 2 && blackHasKnight) return true;
		if (blackPieces == 2 && whitePieces == 1 && whiteHasBishop) return true;
		if (blackPieces == 2 && whitePieces == 1 && whiteHasKnight) return true;

		return false;
	}

	Game Game::CreateSnapshot() const
	{
		Game snapshot;
		snapshot.m_Board = this->m_Board;

		// Do not copy move or board history
		return snapshot;
	}

}
