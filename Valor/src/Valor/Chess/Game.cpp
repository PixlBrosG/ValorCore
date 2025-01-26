#include "vlpch.h"
#include "Valor/Chess/Game.h"

#include "Valor/Chess/MoveGenerator.h"

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
		m_Turn = PieceColor::White;
		m_MoveHistory.clear();
		m_EnPassantTarget = Tile(-1, -1);
		m_CastlingRights[0][0] = true;
		m_CastlingRights[0][1] = true;
		m_CastlingRights[1][0] = true;
		m_CastlingRights[1][1] = true;
		m_HalfmoveCounter = 0;
		m_BoardHistoryHashes.clear();
		m_GameStateHistory.clear();
	}

	void Game::MakeMove(const Move& move)
	{
		// Save the current game state
		m_GameStateHistory.emplace_back(GameState{
			m_Board, m_Turn, m_EnPassantTarget,
			{{m_CastlingRights[0][0], m_CastlingRights[0][1]},
			{m_CastlingRights[1][0], m_CastlingRights[1][1]}},
			m_HalfmoveCounter,
		});

		// Handle en passant
		if (IsEnPassant(move))
		{
			int captureRank = move.Source.Rank;
			int captureFile = move.Target.File;
			m_Board.SetPiece(captureRank, captureFile, Piece());
		}

		// Handle castling
		if (IsCastling(move))
		{
			int rank = move.Source.Rank;
			if (move.Target.File > move.Source.File) // Kingside
			{
				m_Board.SetPiece(rank, 7, Piece());
				m_Board.SetPiece(rank, 5, Piece(PieceType::Rook, m_Turn));
			}
			else // Queenside
			{
				m_Board.SetPiece(rank, 0, Piece());
				m_Board.SetPiece(rank, 3, Piece(PieceType::Rook, m_Turn));
			}
		}

		// Handle promotion
		if (IsPromotion(move))
		{
			// TODO: Allow the user to choose the promotion piece
			m_Board.SetPiece(move.Target, Piece(PieceType::Queen, m_Turn));
		}

		// Increment the halfmove counter
		if (m_Board.GetPiece(move.Source).Type == PieceType::Pawn || m_Board.GetPiece(move.Target).Type != PieceType::None)
			m_HalfmoveCounter = 0;
		else
			m_HalfmoveCounter++;

		// Update the board
		m_Board.SetPiece(move.Target, m_Board.GetPiece(move.Source));
		m_Board.SetPiece(move.Source, Piece());

		HandleEnPassant(move);
		UpdateCastlingRights(move);

		// Update board hash
		size_t hash = m_Board.GetHash();
		m_BoardHistoryHashes[hash]++;

		// Update turn
		m_Turn = SWAP_COLOR(m_Turn);
		m_Board.SetLastMove(move);
	}

	void Game::UndoMove()
	{
		if (m_GameStateHistory.empty())
			return;

		GameState state = m_GameStateHistory.back();
		m_GameStateHistory.pop_back();

		m_Board = state.board;
		m_Turn = state.turn;
		m_EnPassantTarget = state.enPassantTarget;
		std::copy(&state.castlingRights[0][0], &state.castlingRights[0][0] + 4, &m_CastlingRights[0][0]);
		m_HalfmoveCounter = state.halfmoveCounter;

		size_t hash = m_Board.GetHash();
		m_BoardHistoryHashes[hash]--;
		if (m_BoardHistoryHashes[hash] == 0)
			m_BoardHistoryHashes.erase(hash);
	}

	bool Game::IsCheck() const
	{
		Tile kingTile = m_Board.FindKing(m_Turn);

		Game game = CreateSnapshot();
		game.m_Turn = SWAP_COLOR(m_Turn);
		auto moves = MoveGenerator::GenerateMoves(game, true);

		for (const Move& move : moves)
		{
			if (move.Target == kingTile)
				return true;
		}
	}

	bool Game::IsCheckmate() const
	{
		if (!IsCheck())
			return false;
		auto moves = MoveGenerator::GenerateMoves(*this);
		return moves.empty();
	}

	bool Game::IsStalemate() const
	{
		auto moves = MoveGenerator::GenerateMoves(*this);
		return moves.empty();
	}

	void Game::Debug() const
	{
		std::cout << "Board:\n" << m_Board << '\n';
		std::cout << "Turn: " << (m_Turn == PieceColor::White ? "White" : "Black") << '\n';

		std::cout << "Castling rights:\n";
		std::cout << "White king: " << (m_CastlingRights[0][0] ? "Yes" : "No") << '\n';
		std::cout << "White queen: " << (m_CastlingRights[0][1] ? "Yes" : "No") << '\n';
		std::cout << "Black king: " << (m_CastlingRights[1][0] ? "Yes" : "No") << '\n';
		std::cout << "Black queen: " << (m_CastlingRights[1][1] ? "Yes" : "No") << '\n';

		std::cout << "En passant target: " << m_EnPassantTarget << '\n';
		std::cout << "Halfmove counter: " << m_HalfmoveCounter << std::endl;
	}

	void Game::UpdateCastlingRights(const Move& move)
	{
		const Piece& piece = m_Board.GetPiece(move.Source);

		if (piece.Type == PieceType::King)
		{
			m_CastlingRights[static_cast<int>(piece.Color)][0] = false;
			m_CastlingRights[static_cast<int>(piece.Color)][1] = false;
		}
		else if (piece.Type == PieceType::Rook)
		{
			if (move.Source.File == 0)
				m_CastlingRights[static_cast<int>(piece.Color)][1] = false;
			else if (move.Source.File == 7)
				m_CastlingRights[static_cast<int>(piece.Color)][0] = false;
		}
	}

	void Game::HandleEnPassant(const Move& move)
	{
		const Piece& piece = m_Board.GetPiece(move.Source);
		
		m_EnPassantTarget = Tile(-1, -1);

		if (piece.Type == PieceType::Pawn)
		{
			if (abs(move.Source.Rank - move.Target.Rank) == 2)
				m_EnPassantTarget = { (move.Source.Rank + move.Target.Rank) / 2, move.Source.File };
		}
	}

	bool Game::IsEnPassant(const Move& move) const
	{
		const Piece& piece = m_Board.GetPiece(move.Source);
		return piece.Type == PieceType::Pawn && move.Target == m_EnPassantTarget;
	}

	bool Game::IsCastling(const Move& move) const
	{
		const Piece& piece = m_Board.GetPiece(move.Source);
		return piece.Type == PieceType::King && abs(move.Source.File - move.Target.File) == 2;
	}

	bool Game::IsPromotion(const Move& move) const
	{
		const Piece& piece = m_Board.GetPiece(move.Source);
		return piece.Type == PieceType::Pawn && (move.Target.Rank == 0 || move.Target.Rank == 7);
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
		snapshot.m_Turn = this->m_Turn;
		snapshot.m_EnPassantTarget = this->m_EnPassantTarget;
		std::copy(&this->m_CastlingRights[0][0], &this->m_CastlingRights[0][0] + 4, &snapshot.m_CastlingRights[0][0]);
		snapshot.m_HalfmoveCounter = this->m_HalfmoveCounter;

		// Do not copy move history or game state history
		return snapshot;
	}


}
