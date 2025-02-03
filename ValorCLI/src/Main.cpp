#include "Valor/Chess/Game.h"
#include "Valor/Engine/ValorEngine.h"

#include <iostream>
#include <memory>
#include <string>

#ifdef _WIN32
#include <windows.h>
static void ClearConsole()
{
	system("cls");
}
#else
#include <cstdlib>
static void ClearConsole()
{
	system("clear");
}
#endif

int main()
{
	// Player vs. AI
	Valor::Game game;
	Valor::Engine::ValorEngine engine;

	// Print initial board before any moves
	ClearConsole();
	std::cout << game.GetBoard() << std::endl;

	while (!game.IsGameOver())
	{
		if (game.GetBoard().IsWhiteTurn())
		{
			std::string move;
			std::cout << "Enter move: ";
			std::cin >> move;

			if (move == "exit")
				break;

			Valor::Move playerMove = Valor::Move::FromAlgebraic(move);
			if (!game.GetBoard().IsLegalMove(playerMove))
			{
				std::cout << "Illegal move!" << std::endl;
				continue;
			}

			Valor::MoveInfo moveInfo = game.GetBoard().ParseMove(playerMove.Source, playerMove.Target);
			game.MakeMove(playerMove);

			// Print move and board after player's turn
			ClearConsole();
			std::cout << game.GetBoard() << '\n';
			std::cout << "Player played: " << moveInfo.ToAlgebraic() << '\n' << std::endl;
		}
		else
		{
			// AI turn
			std::cout << "AI is thinking..." << std::endl;

			Valor::Move bestMove = engine.SearchBestMove(game.GetBoard(), 6);
			Valor::MoveInfo moveInfo = game.GetBoard().ParseMove(bestMove.Source, bestMove.Target);
			game.MakeMove(bestMove);

			// Print AI move and board after AI's turn
			ClearConsole();
			std::cout << game.GetBoard() << '\n';
			std::cout << "AI played: " << moveInfo.ToAlgebraic() << '\n' << std::endl;
		}
	}
}

