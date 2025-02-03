#include "vlpch.h"
#include "Valor/Chess/Game.h"

#include "Valor/Chess/MoveGeneration/MoveGeneratorSimple.h"

#include <iostream>
#include <algorithm>

namespace Valor {

	Game::Game()
	{
		Init();
	}

	void Game::Init()
	{
		m_Board.Reset();
		m_MoveHistory.clear();
		m_BoardHistoryHashes.clear();
		m_BoardHistory.clear();
	}

	void Game::MakeMove(Move move)
	{
		m_MoveHistory.emplace_back(move);
		m_BoardHistory.emplace_back(m_Board);

		m_Board.MakeMove(move);

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

}
