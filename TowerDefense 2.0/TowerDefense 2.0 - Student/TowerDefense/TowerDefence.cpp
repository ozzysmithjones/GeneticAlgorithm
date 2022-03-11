#include "TowerDefence.h"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <ctime>
#include <cmath>
#include <string>		// String object
#include <vector>		// Vector object
#include <iostream>

#include "Constants.h"
#include "Tower.h"
#include "GameBoard.h"
#include "GameMenuController.h"
#include "MonsterController.h"
#include "TowerController.h"
#include "TowerAndMonsterController.h"
#include "Tower.h"
#include "GameState.h"
#include "Timer.h"
#include "AIController.h"


constexpr bool debug = false;

using sf::Vector2f;
using std::cout;
using std::endl;


const std::vector<Vector2f> path = {
	Vector2f(15, 0), Vector2f(15, 4), Vector2f(
		20, 4),
	Vector2f(22, 4), Vector2f(22, 8), Vector2f(10, 8), Vector2f(10,
																6),
	Vector2f(4, 6), Vector2f(4, 13), Vector2f(12, 13), Vector2f(12, 15),
	Vector2f(17, 15), Vector2f(17, 12), Vector2f(21, 12), Vector2f(21, 18)
};

const std::array<sf::Vector2i, 71> pathTiles
{
	sf::Vector2i(870, -30),
	sf::Vector2i(870, 30),
	sf::Vector2i(870, 90),
	sf::Vector2i(870, 150),
	sf::Vector2i(870, 210),
	sf::Vector2i(930, 210),
	sf::Vector2i(990, 210),
	sf::Vector2i(1050, 210),
	sf::Vector2i(1110, 210),
	sf::Vector2i(1170, 210),
	sf::Vector2i(1230, 210),
	sf::Vector2i(1290, 210),
	sf::Vector2i(1290, 270),
	sf::Vector2i(1290, 330),
	sf::Vector2i(1290, 390),
	sf::Vector2i(1290, 450),
	sf::Vector2i(1230, 450),
	sf::Vector2i(1170, 450),
	sf::Vector2i(1110, 450),
	sf::Vector2i(1050, 450),
	sf::Vector2i(990, 450),
	sf::Vector2i(930, 450),
	sf::Vector2i(870, 450),
	sf::Vector2i(810, 450),
	sf::Vector2i(750, 450),
	sf::Vector2i(690, 450),
	sf::Vector2i(630, 450),
	sf::Vector2i(570, 450),
	sf::Vector2i(570, 390),
	sf::Vector2i(570, 330),
	sf::Vector2i(510, 330),
	sf::Vector2i(450, 330),
	sf::Vector2i(390, 330),
	sf::Vector2i(330, 330),
	sf::Vector2i(270, 330),
	sf::Vector2i(210, 330),
	sf::Vector2i(210, 390),
	sf::Vector2i(210, 450),
	sf::Vector2i(210, 510),
	sf::Vector2i(210, 570),
	sf::Vector2i(210, 630),
	sf::Vector2i(210, 690),
	sf::Vector2i(210, 750),
	sf::Vector2i(270, 750),
	sf::Vector2i(330, 750),
	sf::Vector2i(390, 750),
	sf::Vector2i(450, 750),
	sf::Vector2i(510, 750),
	sf::Vector2i(570, 750),
	sf::Vector2i(630, 750),
	sf::Vector2i(690, 750),
	sf::Vector2i(690, 810),
	sf::Vector2i(690, 870),
	sf::Vector2i(750, 870),
	sf::Vector2i(810, 870),
	sf::Vector2i(870, 870),
	sf::Vector2i(930, 870),
	sf::Vector2i(990, 870),
	sf::Vector2i(990, 810),
	sf::Vector2i(990, 750),
	sf::Vector2i(990, 690),
	sf::Vector2i(1050, 690),
	sf::Vector2i(1110, 690),
	sf::Vector2i(1170, 690),
	sf::Vector2i(1230, 690),
	sf::Vector2i(1230, 750),
	sf::Vector2i(1230, 810),
	sf::Vector2i(1230, 870),
	sf::Vector2i(1230, 930),
	sf::Vector2i(1230, 990),
	//sf::Vector2i(1230, 1050),
};


void TowerDefence::Run()
{
    window = new sf::RenderWindow(sf::VideoMode(1920, 1080), "Monster Defence", sf::Style::Close);
    window->setFramerateLimit(60);
    Reset();

	aiController = new AIController();
	aiController->SetTowerDefence(this);
    aiController->Init();
	aiController->InputChromosomeFromFile("chromosome.json");
    gameMenuController->setDebug(debug);

	clk->start();

    while (window->isOpen())
    {
		sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
		gameState->mousePos = mousePos;

		// Display the help menu
		if (gameState->getHelperState())
		{
			window->clear();
			window->draw(gameBoard->helpScreen);
			window->display();
			while (gameState->getHelperState())
			{
				/*
				while (window->pollEvent(gameBoard->event))
				{
					if (gameBoard->event.type == sf::Event::EventType::Closed
						|| (gameBoard->event.type == sf::Event::KeyPressed
							&& gameBoard->event.key.code == sf::Keyboard::Escape))
					{
						delete gameBoard;
						gameBoard = nullptr;
						window->close();
						return 0;
					}
					else if ((gameBoard->event.type == sf::Event::MouseButtonReleased)
						&& (gameBoard->event.mouseButton.button == sf::Mouse::Left))
					{
						gameState->toggleHelpScreen();
					}
				}
				*/
			}
		}

		// Process
		while (window->pollEvent(gameBoard->event))
		{
			if (gameBoard->event.type == sf::Event::EventType::Closed
				|| (gameBoard->event.type == sf::Event::KeyPressed
					&& gameBoard->event.key.code == sf::Keyboard::Escape))
			{
				window->setActive(false);

				Clean();
				window->close();
				exit(0);
				return;
			}
			else
			{
				gameMenuController->process(gameBoard->event, mousePos);
				gameBoard->process(gameBoard->event, mousePos);
				towerController->process(gameBoard->event, mousePos);
			}
		}

		gameMenuController->update();

		if (clk->newTick())
		{
			//update
			monsterController->update();
			attackController->update();
		}

		if (gameState->dirtyBit)
		{
			gameBoard->debugText.setString(aiController->getDebugText());
			gameBoard->scoreText.setString(std::to_string(gameState->getScore()));
			gameBoard->waveText.setString(std::to_string(gameState->getCurrentWave()));
			gameBoard->healthText.setString(std::to_string(gameState->getHealth()));
			gameBoard->tamText.setString(std::to_string(gameState->getTams()));
			gameState->dirtyBit = false;
		}

		// Render
		window->clear();
		gameBoard->render(window);
		monsterController->render();
		towerController->render();
		gameMenuController->render();
		attackController->render();

		aiController->update();
		UpdateScore();

		gameBoard->renderLabels(window);
		if (debug)
		{
			gameBoard->text.setString(std::to_string(clk->elapsedTicks()));
			gameBoard->text.setFont(gameBoard->font);
			gameBoard->text.setPosition(float(mousePos.x), float(mousePos.y));
			window->draw(gameBoard->text);
		}

		if (gameState->getHealth() <= 0)
		{
			clk->stop();
			aiController->gameOver();
		
			Clean();
			Reset();
			clk->start();
		}

		window->display();
    }


	delete aiController;
	aiController = nullptr;
	delete gameBoard;
	gameBoard = nullptr;
	window->close();
}

int TowerDefence::GetScore() const
{
    return gameState->getScore();
}

bool TowerDefence::IsSpaceForTower(int x, int y)
{
    return gameBoard->gridSpaceAvailable(x,y);
}

bool TowerDefence::IsMoneyForTower(TowerType towerType)
{
    return gameBoard->towerIsPurchasable(towerType);
}

bool TowerDefence::IsTowerInRangeOfPath(int x, int y, TowerType towerType)
{
    return gameBoard->inRangeOfPath(x,y,towerType);
}

bool TowerDefence::PlaceTower(int x, int y, TowerType towerType)
{
    return gameBoard->addTower(towerType,x,y);
}

void TowerDefence::UpdateScore()
{
	int32_t currentWave = gameState->getCurrentWave();
	const int32_t killCount = gameState->getMonsterEliminated();
	currentWave *= 10; // living longer is good
	const int32_t score = currentWave + killCount;

	static int32_t iteration = 0;

	if (iteration == 0)
		cout << "iteration" << "," << "wave" << "," << "kills" << "," << "score" << endl;

	cout << iteration << "," << gameState->getCurrentWave() << "," << gameState->getMonsterEliminated() << "," << score << endl;
	iteration++;

	gameState->setScore(score);
}

void TowerDefence::Clean()
{
	delete clk;
	clk = nullptr;

	delete gameState;
	gameState = nullptr;

	delete gameMenuController;
	gameMenuController = nullptr;

	delete towerController;
	towerController = nullptr;

	delete monsterController;
	monsterController = nullptr;

	delete gameBoard;
	gameBoard = nullptr;

	delete attackController;
	attackController = nullptr;
}

void TowerDefence::Reset()
{
	clk = new Timer();
	gameState = new GameState(clk);
	gameMenuController = new GameMenuController(window, gameState);
	towerController = new TowerController(window, gameState);
	monsterController = new MonsterController(window, gameState, path);
	gameBoard = new GameBoard(gameState, towerController, (int)(gameMenuController)->getMenuPos().x, path, pathTiles);
	attackController = new TowerAndMonsterController( window, gameState, monsterController, 
		(towerController)->getTowerVec(), (monsterController)->getMonsterVec());
}
