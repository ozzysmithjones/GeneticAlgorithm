#include <iostream>
#include <thread>
#define NOMINMAX
#include <Windows.h>
#include <array>
#include "Vector2.h"
#include "Game.h"
#include "AIController.h"


void Play()
{
	Game game;
	AIController controller;
	controller.SetTowerDefence(&game);
	controller.Init();

	//controller.InputChromosomeFromFile("chromosome.json");

	int32_t gen = 0;
	bool rendering = false;

	while (true)
	{
		game.Start();

		while (game.Update())
		{
			controller.update();

			if (GetAsyncKeyState(VK_UP))
			{
				rendering = true;
			}

			if (GetAsyncKeyState(VK_DOWN))
			{
				rendering = false;
			}

			if (rendering)
			{
				game.Render();
			}
		}

		if (game.GetScore() >= 210)
		{
			std::cout << "reached" << game.GetScore() << "\n";
			break;
		}

		controller.gameOver();

		if (controller.GetGeneration() > gen)
		{
			gen = controller.GetGeneration();
			std::cout << "Generation: " << gen << " Highest: " << controller.GetHighestScore() << "\n";
		}
	}

	Chromosome chrom = controller.GetCurrentChromosome();

	std::cout << "Positions:\n";
	for (const auto& pos : chrom.positions)
	{
		std::cout << "Vector2(" << pos.x << ", " << pos.y << "),\n";
	}

	std::cout << "Towers:\n";

	for (const auto& tow : chrom.towers)
	{
		std::cout << (int)tow << "\n";
	}

	controller.OutputChromosomeToFile("chromosome.json");
	controller.OutputFitnessToFile("fitness.csv");
}

int main()
{
	Play();

	/*
	int count = 0;
	for (std::size_t i = 0; i < path.size() - 1; i++)
	{
		int xDiff = path[i + 1].x - path[i].x;
		int yDiff = path[i + 1].y - path[i].y;

		if (xDiff != 0)
		{
			int dir = xDiff > 0 ? 1 : -1;

			for (int j = path[i].x; j != path[i + 1].x; j += dir)
			{
				std::cout << "Vector2(" << j*60-30 << ", " << path[i].y*60-30 << "),\n";
				++count;
			}

		}
		else if (yDiff != 0)
		{
			int dir = yDiff > 0 ? 1 : -1;

			for (int j = path[i].y; j != path[i + 1].y; j += dir)
			{
				std::cout << "Vector2(" << path[i].x*60-30 << ", " << j*60-30 << "),\n";
				++count;
			}
		}
	}

	std::cout << "Vector2(" << path.back().x*60-30 << ", " << path.back().y*60-30 << "),\n";
	++count;
	std::cout << count;
	*/
}