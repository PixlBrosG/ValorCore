#include "vlpch.h"
#include "Valor/Chess/MoveGenerator.h"

#include <iostream>

namespace Valor {

	// TODO: and promotion
	// TODO: Handle check, checkmate, and stalemate

	std::vector<Move> MoveGenerator::GenerateMoves(const Game& game, bool onlyCaptures)
	{
		std::vector<Move> moves;
		for (int rank = 0; rank < 8; rank++)
		{
			for (int file = 0; file < 8; file++)
			{
				Tile source = { rank, file };
				Piece piece = game.GetBoard().GetPiece(source);
				if (piece.Color != game.GetTurn())
					continue;
				switch (piece.Type)
				{
				case PieceType::Pawn:
					GeneratePawnMoves(game, source, moves, onlyCaptures);
					break;
				case PieceType::Knight:
					GenerateKnightMoves(game, source, moves, onlyCaptures);
					break;
				case PieceType::Bishop:
					GenerateBishopMoves(game, source, moves, onlyCaptures);
					break;
				case PieceType::Rook:
					GenerateRookMoves(game, source, moves, onlyCaptures);
					break;
				case PieceType::Queen:
					GenerateQueenMoves(game, source, moves, onlyCaptures);
					break;
				case PieceType::King:
					GenerateKingMoves(game, source, moves, onlyCaptures);
					break;
				}
			}
		}

		std::cout << "Generated " << moves.size() << " moves" << std::endl;
		for (const Move& move : moves)
		{
			std::cout << move.Source << " -> " << move.Target << std::endl;
		}

		return moves;
	}

	void MoveGenerator::GeneratePawnMoves(const Game& game, Tile source, std::vector<Move>& moves, bool onlyCaptures)
	{
		int forward = game.GetTurn() == PieceColor::White ? 1 : -1;
		Tile target;

		// Capture moves
		for (int fileOffset : { -1, 1 })
		{
			if (source.File + fileOffset < 0 || source.File + fileOffset >= 8)
				continue;

			target = { source.Rank + forward, source.File + fileOffset };
			if (game.GetBoard().GetPiece(target).Color != game.GetTurn() && game.GetBoard().GetPiece(target).Color != PieceColor::None)
			{
				moves.emplace_back(Move(source, target));
			}
		}

		// En passant
		Tile enPassantTarget = game.GetEnPassantTarget();
		if (enPassantTarget.Rank != -1 && enPassantTarget.File != -1)
		{
			if (abs(enPassantTarget.Rank - source.Rank) == 1 && abs(enPassantTarget.File - source.File) == 1)
			{
				target = enPassantTarget;
				moves.emplace_back(Move(source, target));
			}
		}

		if (onlyCaptures)
			return;

		// Basic forward move
		target = { source.Rank + forward, source.File };
		if (game.GetBoard().GetPiece(target).Type == PieceType::None)
		{
			moves.emplace_back(Move(source, target));
		}

		// Double forward move
		if ((source.Rank == 1 && game.GetTurn() == PieceColor::White) ||
			(source.Rank == 6 && game.GetTurn() == PieceColor::Black))
		{
			target = { source.Rank + 2 * forward, source.File };
			if (game.GetBoard().GetPiece(target).Type == PieceType::None)
			{
				moves.emplace_back(Move(source, target));
			}
		}
	}

	void MoveGenerator::GenerateKnightMoves(const Game& game, Tile source, std::vector<Move>& moves, bool onlyCaptures)
	{
		for (int rankOffset : { -2, -1, 1, 2 })
		{
			for (int fileOffset : { -2, -1, 1, 2 })
			{
				if (abs(rankOffset) == abs(fileOffset))
					continue;
				Tile target = { source.Rank + rankOffset, source.File + fileOffset };
				if (target.Rank > 0 && target.Rank <= 8 && target.File > 0 && target.File <= 8)
				{
					if (game.GetBoard().GetPiece(target).Color != game.GetTurn())
					{
						if (onlyCaptures && game.GetBoard().GetPiece(target).Color == PieceColor::None)
							continue;
						moves.emplace_back(Move(source, target));
					}
				}
			}
		}
	}

	void MoveGenerator::GenerateBishopMoves(const Game& game, Tile source, std::vector<Move>& moves, bool onlyCaptures)
	{
		for (int rankOffset : { -1, 1 })
		{
			for (int fileOffset : { -1, 1 })
			{
				Tile target = { source.Rank + rankOffset, source.File + fileOffset };
				while (target.Rank > 0 && target.Rank <= 8 && target.File > 0 && target.File <= 8)
				{
					if (game.GetBoard().GetPiece(target).Color == game.GetTurn())
						break;
					if (!onlyCaptures || game.GetBoard().GetPiece(target).Color != PieceColor::None)
						moves.emplace_back(Move(source, target));
					if (game.GetBoard().GetPiece(target).Color != PieceColor::None)
						break;
					target.Rank += rankOffset;
					target.File += fileOffset;
				}
			}
		}
	}

	void MoveGenerator::GenerateRookMoves(const Game& game, Tile source, std::vector<Move>& moves, bool onlyCaptures)
	{
		for (int rankOffset : { -1, 1 })
		{
			Tile target = { source.Rank + rankOffset, source.File };
			while (target.Rank > 0 && target.Rank <= 8)
			{
				if (game.GetBoard().GetPiece(target).Color == game.GetTurn())
					break;
				if (!onlyCaptures || game.GetBoard().GetPiece(target).Color != PieceColor::None)
					moves.emplace_back(Move(source, target));
				if (game.GetBoard().GetPiece(target).Color != PieceColor::None)
					break;
				target.Rank += rankOffset;
			}
		}
	}

	void MoveGenerator::GenerateQueenMoves(const Game& game, Tile source, std::vector<Move>& moves, bool onlyCaptures)
	{
		GenerateBishopMoves(game, source, moves, onlyCaptures);
		GenerateRookMoves(game, source, moves, onlyCaptures);
	}

	void MoveGenerator::GenerateKingMoves(const Game& game, Tile source, std::vector<Move>& moves, bool onlyCaptures)
	{
		for (int rankOffset = -1; rankOffset <= 1; ++rankOffset)
		{
			for (int fileOffset = -1; fileOffset <= 1; ++fileOffset)
			{
				if (rankOffset == 0 && fileOffset == 0)
					continue;
				Tile target = { source.Rank + rankOffset, source.File + fileOffset };
				if (target.Rank > 0 && target.Rank <= 8 && target.File > 0 && target.File <= 8)
				{
					if (game.GetBoard().GetPiece(target).Color != game.GetTurn())
					{
						if (onlyCaptures && game.GetBoard().GetPiece(target).Color == PieceColor::None)
							continue;
						moves.emplace_back(Move(source, target));
					}
				}
			}
		}

		// Castling
		if (game.GetTurn() == PieceColor::White)
		{
			const auto& castlingRights = game.GetCastlingRights();
			if (castlingRights[0][0])
			{
				bool canCastle = true;
				for (int file = 1; file < 4; file++)
				{
					if (game.GetBoard().GetPiece({ 1, file }).Type != PieceType::None)
					{
						canCastle = false;
						break;
					}
				}
				if (canCastle)
				{
					moves.emplace_back(Move(source, { 1, 3 }));
				}
			}
		}
		else
		{
			const auto& castlingRights = game.GetCastlingRights();
			if (castlingRights[1][0])
			{
				bool canCastle = true;
				for (int file = 1; file < 4; file++)
				{
					if (game.GetBoard().GetPiece({ 8, file }).Type != PieceType::None)
					{
						canCastle = false;
						break;
					}
				}
				if (canCastle)
				{
					moves.emplace_back(Move(source, { 8, 3 }));
				}
			}
		}
	}

	void MoveGenerator::TryAddMove(const Game& game, Tile source, Tile target, std::vector<Move>& moves, bool onlyCaptures)
	{
		if (target.Rank < 1 || target.Rank > 8 || target.File < 1 || target.File > 8)
			return;
		if (game.GetBoard().GetPiece(target).Color != game.GetTurn())
		{
			if (onlyCaptures && game.GetBoard().GetPiece(target).Color == PieceColor::None)
				return;

			Game tempGame = game.CreateSnapshot();
			tempGame.MakeMove(Move(source, target));
			if (tempGame.IsCheck())
			{
				std::cout << "Move " << source << " -> " << target << " puts king in check" << std::endl;
				return;
			}

			moves.emplace_back(Move(source, target));
		}
	}

}
