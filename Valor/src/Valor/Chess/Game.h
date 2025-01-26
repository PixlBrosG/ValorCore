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
		void MakeMove(const Move& move);
		void UndoMove();

		// Game state
		bool IsCheck() const;
		bool IsCheckmate() const;
		bool IsStalemate() const;
		bool IsDraw() const { return IsStalemate() || IsFiftyMoveRule() || IsThreefoldRepetition() || IsInsufficientMaterial(); }
		bool IsGameOver() const { return IsCheckmate() || IsDraw(); }

		bool IsThreefoldRepetition() const;
		bool IsFiftyMoveRule() const { return m_HalfmoveCounter >= 50; }
		bool IsInsufficientMaterial() const;

		Game CreateSnapshot() const;

		// Getters
		const Board& GetBoard() const { return m_Board; }
		PieceColor GetTurn() const { return m_Turn; }

		Tile GetEnPassantTarget() const { return m_EnPassantTarget; }
		const bool (&GetCastlingRights() const)[2][2] { return m_CastlingRights; }

		// Debugging utility
		void Debug() const;
	private:
		void UpdateCastlingRights(const Move& move);
		void HandleEnPassant(const Move& move);

		bool IsEnPassant(const Move& move) const;
		bool IsCastling(const Move& move) const;
		bool IsPromotion(const Move& move) const;
	private:
		Board m_Board;
		PieceColor m_Turn = PieceColor::White;
		std::deque<Move> m_MoveHistory;
		Tile m_EnPassantTarget = Tile(-1, -1);
		bool m_CastlingRights[2][2]; // [white/black][king/queen]
		int m_HalfmoveCounter = 0;
		std::unordered_map<size_t, int> m_BoardHistoryHashes;

		struct GameState
		{
			Board board;
			PieceColor turn;
			Tile enPassantTarget;
			bool castlingRights[2][2];
			int halfmoveCounter;
		};
		std::deque<GameState> m_GameStateHistory;
	};

}
