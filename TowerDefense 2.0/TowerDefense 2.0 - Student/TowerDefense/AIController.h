#pragma once
#include <array>
#include <bitset>
#include "Constants.h"
#include "Tower.h"

class GameController;
class GameBoard;
class Timer;
class GameState;


constexpr auto STRIP_LENGTH = 24;

struct Agent
{
	int score = 0;
	std::array<sf::Vector2i, STRIP_LENGTH> positionByInterval;
	std::array<TowerType, STRIP_LENGTH> towerByInterval;

public:
	Agent(int score);
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

	inline const std::string& getDebugText() const { return debugText; }

private:
	GameController* m_gameController;
	GameBoard* m_gameBoard;
	Timer* m_Timer;
	GameState* m_gameState;

	Agent* currentAgent = nullptr;
	int currentAgentIndex;
	std::array<Agent*, 4> agents;
	unsigned int currentTowerInterval;

	double elapsedSeconds = 0;
	double counter = 0;
	std::string debugText = "Generation : 1\nCurrent AI Index : 0\nPrevious score :0";

	void Splice(Agent* primary, const Agent* secondary, double bias);
	void Mutate(Agent* agent, std::size_t numPositionChanges, std::size_t numTowerChanges);
};
