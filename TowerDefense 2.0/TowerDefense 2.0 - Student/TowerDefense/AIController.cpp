#include "AIController.h"
#include "Timer.h"
#include "GameState.h"
#include <sstream>
#include <iostream>
#include <Windows.h>

#include "GameController.h"
#include "Random.h"

using namespace std;


Agent::Agent(int score) : Agent()
{
	this->score = score;
}

Agent::Agent()
{
	for (auto& position : positionByInterval)
	{
		position.x = Random::UniformInt(0, WIDTH - 1);
		position.y = Random::UniformInt(0, HEIGHT - 1);
	}

	for (auto& tower : towerByInterval)
	{
		tower = (TowerType)Random::UniformInt<int>(1, (int)TowerType::count - 1);
	}
}

AIController::AIController()
	: currentAgentIndex(0), currentTowerInterval(0)
{
	for (auto& agent : agents)
	{
		agent = new Agent();
	}

	currentAgent = agents[0];

	m_gameController = nullptr;
	m_gameBoard = nullptr;
	m_Timer = nullptr;
	m_gameState = nullptr;
}

AIController::~AIController()
{
	for (const auto& agent : agents)
	{
		delete agent;
	}
}

void AIController::gameOver()
{
	static int generation = 1;

	elapsedSeconds = 0;
	counter = 0;
	currentTowerInterval = 0;
	currentAgent->score = m_gameState->getScore();
	currentAgentIndex++;

	if (currentAgentIndex >= agents.size())
	{
		generation++;
		//End of generation.
		currentAgentIndex = 0;

		//work out who did it the best.

		std::sort(agents.begin(), agents.end(), [](const Agent* first, const Agent* second) -> bool { return first->score > second->score; });
		const Agent* bestAgent = agents[0];
		const double bestScore = bestAgent->score;

		for (std::size_t i = 1; i < agents.size(); i++)
		{
			if (i < agents.size() / 2)
			{
				const double bias = bestScore / ((double)agents[i]->score + bestScore);
				Splice(agents[i], bestAgent, bias);
			}
			else
			{
				*agents[i] = *agents[i - agents.size() / 2];
				agents[i]->score = 0;
			}
		}


		for (std::size_t i = 1; i < agents.size(); i++)
		{
			Mutate(agents[i], (STRIP_LENGTH / (agents.size() + 1)) * i, 8);
		}
	}

	currentAgent = agents[currentAgentIndex];

	std::stringstream stream;
	stream << "Generation : " << generation << "\nCurrent AI Index : " << currentAgentIndex << "\nPrevious score : " << currentAgent->score;
	debugText = stream.str();
}

void AIController::Mutate(Agent* agent, std::size_t numPositionChanges, std::size_t numTowerChanges)
{
	for (std::size_t i = 0; i < numPositionChanges; i++)
	{
		const int roll = Random::UniformInt<int>(0, (int)agent->positionByInterval.size() - 1);
		do
		{
			agent->positionByInterval[roll].x = Random::UniformInt(0, WIDTH - 1);
			agent->positionByInterval[roll].y = Random::UniformInt(0, HEIGHT - 1);
		}
		while (!m_gameBoard->inRangeOfPath(agent->positionByInterval[roll].x, agent->positionByInterval[roll].y, agent->towerByInterval[roll]));
	}

	for (std::size_t i = 0; i < numTowerChanges; i++)
	{
		//Random change a tower.
		const int roll = Random::UniformInt<int>(0, (int)agent->towerByInterval.size() - 1);
		agent->towerByInterval[roll] = (TowerType)Random::UniformInt<int>(1, (int)TowerType::count - 1);
	}
}

void AIController::Splice(Agent* primary, const Agent* secondary, double bias)
{
	auto& primaryPositionByInterval = primary->positionByInterval;
	auto& secondaryPositionByInterval = secondary->positionByInterval;

	for (std::size_t i = 0; i < (int)TowerType::count - 1; i++)
	{
		const double roll = Random::UniformReal(0.0, std::nextafter(1.0, std::numeric_limits<double>::max()));
		if (roll <= bias)
		{
			primaryPositionByInterval[i] = secondaryPositionByInterval[i];
		}
	}

	auto& primaryTowerByInterval = primary->towerByInterval;
	auto& secondaryTowerByInterval = secondary->towerByInterval;

	for (std::size_t i = 0; i < primaryTowerByInterval.size(); i++)
	{
		const double roll = Random::UniformReal(0.0, std::nextafter(1.0, std::numeric_limits<double>::max()));
		if (roll <= bias)
		{
			primaryTowerByInterval[i] = secondaryTowerByInterval[i];
		}
	}
}

void AIController::update()
{
	if (m_Timer == nullptr)
		return;

	//Calculation of delta time for timing.
	const double seconds = floor(m_Timer->elapsedSeconds());
	const double deltaTime = seconds - elapsedSeconds;
	if (seconds > elapsedSeconds)
	{
		elapsedSeconds = seconds;
	}


	counter += deltaTime;

	if (counter > 0.2)
	{
		counter = 0;
		const TowerType& type = currentAgent->towerByInterval[currentTowerInterval];
		const sf::Vector2i& position = currentAgent->positionByInterval[currentTowerInterval];
		const bool spaceAvailable = m_gameBoard->gridSpaceAvailable(position.x, position.y);
		const bool purchasable = m_gameBoard->towerIsPurchasable(type);

		if (spaceAvailable && purchasable)
		{
			m_gameBoard->addTower(type, position.x, position.y);
		}

		if (!spaceAvailable || purchasable)
		{
			currentTowerInterval++;
			if (currentTowerInterval >= currentAgent->towerByInterval.size())
				currentTowerInterval = 0;
		}
	}

	//GAManager::Instance()->Update(m_Timer->elapsedSeconds());

	/*
	// this might be useful? Monsters killed
	static int monstersKilled = 0;

	if (m_gameState->getMonsterEliminated() > monstersKilled)
	{
		monstersKilled = m_gameState->getMonsterEliminated();
	}
	*/

	recordScore();
}

void AIController::addTower(TowerType type, int gridx, int gridy)
{
	// grid position can be from 0,0 to 25,17
	/*
	enum class TowerType {
	empty, slammer, swinger, thrower };
	*/

	bool towerAdded = m_gameBoard->addTower(type, gridx, gridy);

	// NOTE towerAdded might be false if the tower can't be placed in that position, is there isn't enough funds
}

void AIController::setupBoard()
{
	m_Timer->start();
}

int AIController::recordScore()
{
	int currentWave = m_gameState->getCurrentWave();
	const int killCount = m_gameState->getMonsterEliminated();
	currentWave *= 10; // living longer is good
	const int score = currentWave + killCount;

	static int iteration = 0;

	if (iteration == 0)
		cout << "iteration" << "," << "wave" << "," << "kills" << "," << "score" << endl;

	cout << iteration << "," << m_gameState->getCurrentWave() << "," << m_gameState->getMonsterEliminated() << "," << score << endl;
	iteration++;

	m_gameState->setScore(score);

	return score;
}
