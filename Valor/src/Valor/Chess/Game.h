#pragma once

#include "Valor/Chess/Board.h"
#include "Valor/Chess/Move.h"

#include <deque>
#include <unordered_map>

namespace Valor {

	class Game
	{
	public:
		Game();

		// Initialization and gameplay
		void Init();
		void MakeMove(Move move);
		void UndoMove();

		// Game state
		bool IsDraw() const { return m_Board.IsStalemate() || m_Board.IsFiftyMoveRule() || IsThreefoldRepetition() || m_Board.IsInsufficientMaterial(); }
		bool IsGameOver() const { return m_Board.IsCheckmate() || IsDraw(); }

		bool IsThreefoldRepetition() const;

		// Getters
		const Board& GetBoard() const { return m_Board; }
	private:
		Board m_Board;
		std::deque<Move> m_MoveHistory;
		std::unordered_map<size_t, int> m_BoardHistoryHashes;
		std::vector<Board> m_BoardHistory;
	};

}
