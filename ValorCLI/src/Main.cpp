#include "Valor/Engine/ValorEngine.h"

#include <iostream>
#include <memory>
#include <string>

#ifdef _WIN32
#include <windows.h>
void ClearConsole()
{
	system("cls");
}
#else
#include <cstdlib>
void ClearConsole()
{
	system("clear");
}
#endif

int main()
{
	using namespace Valor;

	// Initialize engine and set game and evaluator
	Engine::ValorEngine engine;
	engine.SetGame(std::make_unique<Game>());
	engine.SetEvaluator(std::make_unique<PieceValueEvaluator>());

	engine.Reset();
#if 0
	// Initial board state
	ClearConsole();
	std::cout << "Initial Board:\n" << std::endl;
	std::cout << engine.GetGame()->GetBoard() << std::endl;

	// Wait for user input before starting
	std::cout << "Press Enter to begin...";
	std::cin.get();

	while (!engine.GetGame()->IsGameOver()) {
		// Find and play the best move
		Move bestMove = engine.FindBestMove(6);
		bestMove = engine.GetGame()->GetBoard().ParseMove(bestMove.GetSource(), bestMove.GetTarget());
		engine.MakeMove(bestMove);

		// Clear console
		ClearConsole();

		// Display the updated board state
		std::cout << "Board:\n" << std::endl;
		std::cout << engine.GetGame()->GetBoard() << std::endl;

		// Evaluate the board
		float score = engine.Evaluate();
		std::cout << "Score: " << score << std::endl;

		// Display the move played
		std::cout << "\nMove played: " << bestMove.ToAlgebraic() << std::endl;

		// Wait for user input
		std::cout << "Press Enter to continue...";
		std::cin.get();
	}

	// Game over
	ClearConsole();
	std::cout << "Game over!" << std::endl;
	std::cout << "Final Board:\n" << std::endl;
	std::cout << engine.GetGame()->GetBoard() << std::endl;

	std::cout << "Result: ";
	if (engine.GetGame()->GetBoard().IsCheckmate(!engine.GetGame()->GetBoard().IsWhiteTurn()))
		std::cout << "Checkmate!" << (engine.GetGame()->GetBoard().IsWhiteTurn() ? " Black wins!" : " White wins!");
	else if (engine.GetGame()->GetBoard().IsStalemate(!engine.GetGame()->GetBoard().IsWhiteTurn()))
		std::cout << "Stalemate!";
	else if (engine.GetGame()->IsDraw())
	{
		if (engine.GetGame()->IsThreefoldRepetition())
			std::cout << "Draw by Threefold Repetition!";
		else if (engine.GetGame()->GetBoard().IsFiftyMoveRule())
			std::cout << "Draw by Fifty Move Rule!";
		else if (engine.GetGame()->IsInsufficientMaterial())
			std::cout << "Draw by Insufficient Material!";
	}

	std::cout << std::endl;
#elif 1
	using namespace Valor;

	bool playerTurn = true;

	// Print the initial board
	ClearConsole();
	std::cout << engine.GetGame()->GetBoard() << std::endl;

	while (!engine.GetGame()->IsGameOver())
	{
		if (playerTurn)
		{
			// Player's move
			std::string inputMove;
			std::cout << "Your move (e.g., e2e4): ";
			std::cin >> inputMove;

			MoveInfo playerMove = Move::FromAlgebraic(inputMove);
			playerMove = engine.GetGame()->GetBoard().ParseMove(playerMove.Source, playerMove.Target);
			if (!engine.GetGame()->GetBoard().IsLegalMove(playerMove))
			{
				std::cout << "Invalid move: " << inputMove << '\n';
				std::cin.get();  // Wait for Enter
				continue;        // Retry move
			}
			engine.MakeMove(playerMove);

			// Clear console and display the board
			ClearConsole();
			std::cout << engine.GetGame()->GetBoard() << std::endl;

			// Display the move played by the bot
			std::cout << "You played: " << playerMove.ToAlgebraic() << '\n';
			std::cout << "Board evaluation: " << engine.Evaluate() << '\n';

			// Switch to bot's turn
			playerTurn = false;
		}
		else
		{
			// Bot's move
			std::cout << "Bot is thinking...\n";
			MoveInfo bestMove = engine.FindBestMove(6);  // Adjust depth as needed
			bestMove = engine.GetGame()->GetBoard().ParseMove(bestMove.Source, bestMove.Target);
			engine.MakeMove(bestMove);

			// Clear console and display the board
			ClearConsole();
			std::cout << engine.GetGame()->GetBoard() << std::endl;

			// Display the move played by the bot
			std::cout << "Bot played: " << bestMove.ToAlgebraic() << '\n';
			std::cout << "Board evaluation: " << engine.Evaluate() << '\n';

			// Switch to player's turn
			playerTurn = true;
		}
	}

	// Game over - print result
	ClearConsole();
	std::cout << "Game over!\n";
	std::cout << engine.GetGame()->GetBoard() << std::endl;

	if (engine.GetGame()->GetBoard().IsCheckmate(engine.GetGame()->GetBoard().IsWhiteTurn())) {
		std::cout << (playerTurn ? "Bot wins!" : "You win!") << std::endl;
	}
	else if (engine.GetGame()->GetBoard().IsStalemate(engine.GetGame()->GetBoard().IsWhiteTurn())) {
		std::cout << "Stalemate!" << std::endl;
	}
	else {
		std::cout << "Draw!" << std::endl;
	}
#else
	int moves = 0;

	while (!engine.GetGame()->IsGameOver() && moves < 25)
	{
		// bot plays against itself endlessly
		Move bestMove = engine.FindBestMove(4);
		engine.MakeMove(bestMove);

		++moves;
	}

	// Game over - print result
	ClearConsole();
	std::cout << "Game over!\n";
	std::cout << engine.GetGame()->GetBoard() << std::endl;

	if (engine.GetGame()->GetBoard().IsCheckmate(SWAP_COLOR(engine.GetGame()->GetBoard().GetTurn()))) {
		std::cout << "Bot wins!" << std::endl;
	}
	else if (engine.GetGame()->GetBoard().IsStalemate(SWAP_COLOR(engine.GetGame()->GetBoard().GetTurn()))) {
		std::cout << "Stalemate!" << std::endl;
	}
	else {
		std::cout << "Draw!" << std::endl;
	}
#endif
}
