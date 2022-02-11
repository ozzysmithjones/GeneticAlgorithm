#pragma once
#include <array>
#include <bitset>
#include "Constants.h"
#include "Tower.h"

class GameController;
class GameBoard;
class Timer;
class GameState;


struct Agent
{
	int score = 0;
	std::array<std::array<unsigned, WIDTH * HEIGHT>, (int)TowerType::count - 1> PositionsByTower;
	std::array<std::array<unsigned, WIDTH * HEIGHT>, (int)TowerType::count - 1> IndexOfPositionsByTower;
	std::array<TowerType, 24> towerByInterval;

public:
	Agent();
};

class AIController
{
public:
	AIController();
	~AIController();

	void setupBoard();
	int recordScore();

	void setGameController(GameController* gameController) { m_gameController = gameController; }
	void setGameBoard(GameBoard* gameBoard) { m_gameBoard = gameBoard; }
	void setTimer(Timer* timer) { m_Timer = timer; }
	void setGameState(GameState* gameState) { m_gameState = gameState; }
	void update();
	void addTower(TowerType type, int gridx, int gridy);
	void gameOver();

private:
	GameController* m_gameController;
	GameBoard* m_gameBoard;
	Timer* m_Timer;
	GameState* m_gameState;

	Agent* currentAgent = nullptr;
	int currentAgentIndex;
	std::array<Agent, 10> agents;
	unsigned int currentTowerInterval;

	double elapsedSeconds = 0;
	double counter = 0;


	void Splice(Agent* primary, const Agent* secondary, double bias);
	void Mutate(Agent* agent, std::size_t numPositionChanges, std::size_t numTowerChanges);
};
