#include "Valor/Engine/ValorEngine.h"

#include <iostream>
#include <memory>
#include <string>

#ifdef _WIN32
#include <windows.h>
void ClearConsole() {
	system("cls");
}
#else
#include <cstdlib>
void ClearConsole() {
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
		// Clear console
		ClearConsole();

		// Find and play the best move
		Move bestMove = engine.FindBestMove(5);
		engine.MakeMove(bestMove);

		// Display the updated board state
		std::cout << "Board:\n" << std::endl;
		std::cout << engine.GetGame()->GetBoard() << std::endl;

		// Evaluate the board
		float score = engine.Evaluate();
		std::cout << "Score: " << score << std::endl;

		// Display the move played
		std::cout << "\nMove played: " << bestMove.Source << " -> " << bestMove.Target << std::endl;

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
	if (engine.GetGame()->IsCheckmate())
		std::cout << "Checkmate!" << (engine.GetGame()->GetTurn() == PieceColor::White ? " Black wins!" : " White wins!");
	else if (engine.GetGame()->IsStalemate())
		std::cout << "Stalemate!";
	else if (engine.GetGame()->IsDraw())
	{
		if (engine.GetGame()->IsThreefoldRepetition())
			std::cout << "Draw by Threefold Repetition!";
		else if (engine.GetGame()->IsFiftyMoveRule())
			std::cout << "Draw by Fifty Move Rule!";
		else if (engine.GetGame()->IsInsufficientMaterial())
			std::cout << "Draw by Insufficient Material!";
	}

	std::cout << std::endl;
#else
	using namespace Valor;

	bool playerTurn = true;

	while (!engine.GetGame()->IsGameOver()) {
		// Clear console and display the board
		ClearConsole();
		std::cout << engine.GetGame()->GetBoard() << std::endl;

		if (playerTurn) {
			// Player's move
			std::string inputMove;
			std::cout << "Your move (e.g., e2e4): ";
			std::cin >> inputMove;

			// Parse input and make the move
			try {
				Move playerMove = Move::FromString(inputMove);
				engine.MakeMove(playerMove);
			}
			catch (const std::exception& e) {
				std::cout << "Invalid move: " << e.what() << "\n";
				std::cin.get();  // Wait for Enter
				continue;        // Retry move
			}
		}
		else {
			// Bot's move
			std::cout << "Bot is thinking..." << std::endl;
			Move bestMove = engine.FindBestMove(5);  // Adjust depth as needed
			engine.MakeMove(bestMove);

			std::cout << "Bot played: " << (std::string)bestMove << std::endl;
		}

		// Switch turns
		playerTurn = !playerTurn;
	}

	// Game over - print result
	ClearConsole();
	std::cout << "Game over!\n";
	std::cout << engine.GetGame()->GetBoard() << std::endl;

	if (engine.GetGame()->IsCheckmate()) {
		std::cout << (playerTurn ? "Bot wins!" : "You win!") << std::endl;
	}
	else if (engine.GetGame()->IsStalemate()) {
		std::cout << "Stalemate!" << std::endl;
	}
	else {
		std::cout << "Draw!" << std::endl;
	}
#endif
}
