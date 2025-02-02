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
	// Player vs. player
	Valor::Game game;
	std::string input;

	while (!game.IsGameOver())
	{
		ClearConsole();
		std::cout << game.GetBoard() << std::endl;
		std::cout << "Enter move: ";
		std::getline(std::cin, input);
		if (input == "exit")
			break;
		Valor::Move move = Valor::Move::FromAlgebraic(input);
		if (move.IsValid() && game.GetBoard().IsLegalMove(move))
			game.MakeMove(move);
		else
			std::cout << "Invalid move!" << std::endl;
	}

	std::cout << "Game over!" << std::endl;
	if (game.GetBoard().IsCheckmate())
		std::cout << "Checkmate!" << std::endl;
	else if (game.GetBoard().IsStalemate())
		std::cout << "Stalemate!" << std::endl;
	else
		std::cout << "Draw!" << std::endl;
}
