#include "AIController.h"
#include "Timer.h"
#include "GameState.h"
#include <iostream>
#include <Windows.h>

#include "GameController.h"
#include "Random.h"

using namespace std;


Agent::Agent()
{
	//scramble the indices of the positions. 
	for (std::size_t towerIndex = 0; towerIndex < PositionsByTower.size(); towerIndex++)
	{
		auto& positions = PositionsByTower[towerIndex];
		auto& indicesOfPositions = IndexOfPositionsByTower[towerIndex];

		for (std::size_t i = 0; i < positions.size(); i++)
		{
			positions[i] = i;
			indicesOfPositions[i] = i;
		}

		for (std::size_t i = 0; i < positions.size(); i++)
		{
			const std::size_t other = Random::UniformInt(0u, positions.size() - 1);
			indicesOfPositions[positions[other]] = i;
			indicesOfPositions[positions[i]] = other;
			std::swap(positions[other], positions[i]);
		}
	}

	//
	for (auto& tower : towerByInterval)
	{
		tower = (TowerType)Random::UniformInt<int>(1, (int)TowerType::count - 1);
	}
}

AIController::AIController()
	: currentAgentIndex(0), currentTowerInterval(0)
{
	currentAgent = &agents[0];

	m_gameController = nullptr;
	m_gameBoard = nullptr;
	m_Timer = nullptr;
	m_gameState = nullptr;
}

AIController::~AIController()
{
}

void AIController::gameOver()
{
	elapsedSeconds = 0;
	currentAgent->score = m_gameState->getScore();

	currentAgentIndex++;
	if (currentAgentIndex >= agents.size())
	{
		//End of generation.
		currentAgentIndex = 0;

		//work out who did it the best.
		int minScore = std::numeric_limits<int>::max();
		int bestScore = std::numeric_limits<int>::min();
		int secondBestScore = std::numeric_limits<int>::min();
		Agent* bestAgent = &agents[0];
		Agent* secondBestAgent = &agents[1];

		for (auto& agent : agents)
		{
			if (agent.score < minScore)
			{
				minScore = agent.score;
			}

			if (agent.score > bestScore)
			{
				secondBestAgent = bestAgent;
				secondBestScore = bestScore;
				bestScore = agent.score;
				bestAgent = &agent;
			}
			else if (agent.score > secondBestScore)
			{
				secondBestScore = agent.score;
				secondBestAgent = &agent;
			}
		}

		bestScore -= minScore;
		secondBestScore -= minScore;
		const double bias = (double)(secondBestScore) / std::max((double)(bestScore + secondBestScore), 0.001);
		Splice(bestAgent, secondBestAgent, bias);

		for (std::size_t i = 0; i < agents.size(); i++)
		{
			if (&agents[i] == bestAgent)
				continue;

			agents[i] = *secondBestAgent;
			Mutate(&agents[i], (i + 1) * ((WIDTH * HEIGHT) / 10), 1);
		}
	}

	currentAgent = &agents[currentAgentIndex];
}

void AIController::Mutate(Agent* agent, std::size_t numPositionChanges, std::size_t numTowerChanges)
{
	for (std::size_t i = 0; i < numPositionChanges; i++)
	{
		for (std::size_t towerIndex = 0; towerIndex < (std::size_t)TowerType::count - 1; towerIndex++)
		{
			//Random swap a position.

			auto& positionsByPower = agent->PositionsByTower[towerIndex];
			auto& indicesOfPositionsByTower = agent->IndexOfPositionsByTower[towerIndex];

			const int rollOne = Random::UniformInt<int>(0, (int)positionsByPower.size() - 1);
			const int rollTwo = Random::UniformInt<int>(0, (int)positionsByPower.size() - 1);

			indicesOfPositionsByTower[positionsByPower[rollOne]] = rollTwo;
			indicesOfPositionsByTower[positionsByPower[rollTwo]] = rollOne;
			std::swap(positionsByPower[rollOne], positionsByPower[rollTwo]);
		}
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
	for (std::size_t towerIndex = 0; towerIndex < (int)TowerType::count - 1; towerIndex++)
	{
		auto& primaryPositions = primary->PositionsByTower[towerIndex];
		auto& primaryIndexOfPositions = primary->IndexOfPositionsByTower[towerIndex];
		auto& secondaryPositions = secondary->PositionsByTower[towerIndex];

		for (std::size_t i = 0; i < primaryPositions.size(); i++)
		{
			const double roll = Random::UniformReal(0.0, std::nextafter(1.0, std::numeric_limits<double>::max()));

			if (roll <= bias)
			{
				const unsigned swapIndex = primaryIndexOfPositions[secondaryPositions[i]];
				primaryIndexOfPositions[primaryPositions[i]] = swapIndex;
				primaryIndexOfPositions[primaryPositions[swapIndex]] = i;
				std::swap(primaryPositions[i], primaryPositions[swapIndex]);
			}
		}
	}

	auto& primaryTowerByinterval = primary->towerByInterval;
	auto& secondaryTowerByInterval = secondary->towerByInterval;

	for (std::size_t i = 0; i < primaryTowerByinterval.size(); i++)
	{
		const double roll = Random::UniformReal(0.0, std::nextafter(1.0, std::numeric_limits<double>::max()));
		if (roll <= bias)
		{
			primaryTowerByinterval[i] = secondaryTowerByInterval[i];
		}
	}
}

void AIController::update()
{
	if (m_Timer == nullptr)
		return;

	//Calculation of delta time for timing.
	double seconds = floor(m_Timer->elapsedSeconds());
	double deltaTime = seconds - elapsedSeconds;
	if (seconds > elapsedSeconds)
	{
		elapsedSeconds = seconds;
	}


	counter += deltaTime;

	if (counter > 1.0)
	{
		counter = 0;
		const TowerType& type = currentAgent->towerByInterval[currentTowerInterval];
		if (m_gameBoard->towerIsPurchasable(type))
		{
			//enumerate through all of the positions  (organized by priority) until one is found to place the tower.
			for (const auto& i : currentAgent->PositionsByTower[(int)type - 1])
			{
				if (m_gameBoard->addTower(type, i % WIDTH, i / WIDTH))
				{
					currentTowerInterval++;
					if (currentTowerInterval >= currentAgent->towerByInterval.size())
						currentTowerInterval = 0;
					break;
				}
			}
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
