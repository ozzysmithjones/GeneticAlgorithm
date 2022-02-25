#include "AIController.h"
#include "Timer.h"
#include "GameState.h"
#include <sstream>
#include <iostream>
#include <Windows.h>

#include "GameController.h"
#include "Random.h"

using namespace std;


AIController::AIController()
	: currentAgentIndex(0)
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
	currentAgent->lastTowerIndex = currentTowerIndex;
	currentAgent->fitness = CalculateFitness(currentAgent);
	currentAgentIndex++;

	if (currentAgentIndex >= agents.size())
	{
		//End of generation.
		generation++;
		currentAgentIndex = 0;

		CrossOver(agents);
		Mutate(agents);
	}

	currentAgent = agents[currentAgentIndex];
	elapsedSeconds = 0;
	currentTowerIndex = 0;

	//update debugging text:
	std::stringstream stream;
	stream << "Generation : " << generation << "\nCurrent AI Index : " << currentAgentIndex << "\nPrevious fitness : " << currentAgent->fitness;
	debugText = stream.str();
}

void AIController::InitPositionsInRangeByTower()
{
	std::array<std::bitset<WIDTH * HEIGHT>, (std::size_t)TowerType::count> bitboardByTower;

	//Init the bitboards with a bit representing that the tower in that position is in range.
	for (std::size_t i = 1; i < (std::size_t)TowerType::count; i++)
	{
		for (std::size_t y = 0; y < HEIGHT; y++)
		{
			for (std::size_t x = 0; x < WIDTH; x++)
			{
				bitboardByTower[i][y * WIDTH + x] = m_gameBoard->inRangeOfPath(x, y, (TowerType)i);
			}
		}
	}

	//Create an array of only the positions in shooting range of the path
	for (std::size_t i = 0; i < (std::size_t)TowerType::count; i++)
	{
		for (std::size_t y = 0; y < HEIGHT; y++)
		{
			for (std::size_t x = 0; x < WIDTH; x++)
			{
				if (bitboardByTower[i][y * WIDTH + x])
				{
					positionsInRangeByTower[i].emplace_back(x, y);
				}
			}
		}
	}
}

void AIController::Initialisation(AgentArray& agents)
{
	/* random init*/
	for (const auto& agent : agents)
	{
		Chromosome& chrom = agent->chromosome;

		for (std::size_t i = 0; i < NUM_GENES; i++)
		{
			const std::size_t tower = Random::UniformInt(1u, (std::size_t)TowerType::count - 1);
			chrom.towers[i] = (TowerType)tower;

			const std::size_t posRoll = Random::UniformInt(0u, positionsInRangeByTower[tower].size() - 1);;
			chrom.positions[i] = positionsInRangeByTower[tower][posRoll];
		}
	}
	
	//best chromosone
	Chromosome& chrom = agents[0]->chromosome;

	/*
enum class TowerType
slammer, swinger, thrower, count,

*/

	/*
	chrom.positions =
	{	
		sf::Vector2i(10, 6),
		sf::Vector2i(6, 5),
		sf::Vector2i(13, 2),
		sf::Vector2i(17, 10),
		sf::Vector2i(11, 9),
		sf::Vector2i(10, 8),
		sf::Vector2i(3, 10),
		sf::Vector2i(20, 6),
		sf::Vector2i(13, 11),
		sf::Vector2i(6, 14),
		sf::Vector2i(13, 4),
		sf::Vector2i(13, 9),
		sf::Vector2i(11, 6),
		sf::Vector2i(2, 6),
		sf::Vector2i(10, 9),
	};



	chrom.towers =
	{
		TowerType::thrower,
		TowerType::thrower,
		TowerType::swinger,
		TowerType::thrower,
		TowerType::thrower,
		TowerType::swinger,
		TowerType::thrower,
		TowerType::swinger,
		TowerType::thrower,
		TowerType::swinger,
		TowerType::thrower,
		TowerType::swinger,
		TowerType::swinger,
		TowerType::swinger,
		TowerType::thrower,
	};
	*/
	

	//39 generations:
	/*
	chrom.positions =
	{
		sf::Vector2i(13, 1),
		sf::Vector2i(16, 2),
		sf::Vector2i(8, 10),
		sf::Vector2i(17, 9),
		sf::Vector2i(19, 5),
		sf::Vector2i(11, 9),
		sf::Vector2i(16, 6),
		sf::Vector2i(15, 12),
		sf::Vector2i(19, 0),
		sf::Vector2i(8, 5),
		sf::Vector2i(14, 9),
		sf::Vector2i(19, 10),
		sf::Vector2i(20, 8),
		sf::Vector2i(19, 14),
		sf::Vector2i(25, 11),
	};

	chrom.towers =
	{
		TowerType::swinger,
		TowerType::swinger,
		TowerType::thrower,
		TowerType::thrower,
		TowerType::thrower,
		TowerType::thrower,
		TowerType::thrower,
		TowerType::thrower,
		TowerType::thrower,
		TowerType::swinger,
		TowerType::thrower,
		TowerType::swinger,
		TowerType::swinger,
		TowerType::thrower,
		TowerType::thrower,
	};
	*/
	
	//105 generations:
	chrom.positions =
	{
		sf::Vector2i(3, 5),
		sf::Vector2i(13, 0),
		sf::Vector2i(15, 11),
		sf::Vector2i(11, 3),
		sf::Vector2i(13, 11),
		sf::Vector2i(9, 11),
		sf::Vector2i(11, 6),
		sf::Vector2i(21, 7),
		sf::Vector2i(16, 6),
		sf::Vector2i(17, 14),
		sf::Vector2i(16, 16),
		sf::Vector2i(7, 13),
		sf::Vector2i(18, 10),
		sf::Vector2i(19, 16),
		sf::Vector2i(23, 6),
	};

	chrom.towers =
	{
		TowerType::swinger,
		TowerType::swinger,
		TowerType::thrower,
		TowerType::thrower,
		TowerType::thrower,
		TowerType::thrower,
		TowerType::swinger,
		TowerType::swinger,
		TowerType::thrower,
		TowerType::swinger,
		TowerType::swinger,
		TowerType::swinger,
		TowerType::swinger,
		TowerType::swinger,
		TowerType::swinger,
	};
}

std::size_t AIController::CalculateFitness(Agent* agent)
{
	std::size_t result = m_gameState->getScore();
	result += (int32_t)(10 * ((float)m_gameState->GetWaveCounter() / 900.0f));
	return result;
}

void AIController::CrossOver(AgentArray& agents)
{
	///.-----------------------------------------------------------------------------\.
	//			If apportionment based, repopulate the agents according to fitness:
	//\.-----------------------------------------------------------------------------./

#if APPORTIONMENT

	ApportCrossOver(agents);
	return;

#endif

	///.-----------------------------------------------------------------------------\.
	//			Sort the agents according to fitness:
	//\.-----------------------------------------------------------------------------./

#if ROULETTE_SORT

	RouletteSort(agents);

#elif TOURNAMENT_SORT

	TournamentSort(agents);

#endif

	///.-----------------------------------------------------------------------------\.
	//			Decide how many of the top can be selected for the next generation:
	//\.-----------------------------------------------------------------------------./

#if SINGLE_SELECTION_AMOUNT

	constexpr std::size_t selectionAmount = 2;

#elif HALF_SELECTION_AMOUNT

	constexpr std::size_t selectionAmount = NUM_AGENTS / 2;

#elif DISTRIBUTION_SELECTION_AMOUNT

	constexpr std::size_t selectionAmount = NUM_AGENTS / 2;
	//Todo distribution selection amount

#elif PROGRESS_SELECTION_AMOUNT

	const double progress = (double)agents[0]->fitness / 200.0;
	const std::size_t selectionAmount = std::max((std::size_t)((1.0 - progress) * agents.size() / 2), 2u);

#endif

	///.-----------------------------------------------------------------------------\.
	//			keeping the best agent for the next round (index 0),
	//			cross the top selected, overwriting any of the ones not selected
	//\.-----------------------------------------------------------------------------./

	Chromosome ab;
	Chromosome ba;

	for (std::size_t i = 0; i < selectionAmount - 1; i++)
	{
		CrossOver(agents[i], agents[i + 1], ab, ba);

		if (i != 0)
		{
			agents[i]->chromosome = ab;
		}

		if ((i + selectionAmount) < agents.size())
		{
			agents[i + selectionAmount]->chromosome = ba;
		}
	}

	for (std::size_t i = (selectionAmount * 2); i < agents.size(); i++)
	{
		agents[i]->chromosome = agents[i - (selectionAmount * 2)]->chromosome;
	}
}

void AIController::Mutate(AgentArray& agents)
{
	for (std::size_t i = 0; i < agents.size(); i++)
	{
		Mutate(agents[i], i);
	}
}

void AIController::ApportCrossOver(AgentArray& agents)
{

#if ROULETTE_SORT

	RouletteSort(agents);

#elif TOURNAMENT_SORT

	TournamentSort(agents);

#endif

#if SINGLE_SELECTION_AMOUNT

	constexpr std::size_t selectionAmount = 2;

#elif HALF_SELECTION_AMOUNT

	constexpr std::size_t selectionAmount = NUM_AGENTS / 2;

#elif DISTRIBUTION_SELECTION_AMOUNT

	constexpr std::size_t selectionAmount = NUM_AGENTS / 2;
	//Todo distribution selection amount

#elif PROGRESS_SELECTION_AMOUNT

	const double progress = (double)agents[0]->fitness / 200.0;
	const std::size_t selectionAmount = std::max((std::size_t)((1.0 - progress) * agents.size() / 2), 2u);

#endif

	Chromosome ab;
	Chromosome ba;

	std::array<Chromosome, NUM_GENES> newChromosomes;
	std::size_t totalFitness = 0;
	std::size_t j = 1;
	for (std::size_t i = 0; i < selectionAmount-1; i += 2)
	{
		totalFitness += (agents[i]->fitness + agents[i+1]->fitness) / 2;
	}

	for (std::size_t i = 0; i < selectionAmount - 1; i += 2)
	{
		const std::size_t fitness = (agents[i]->fitness + agents[i + 1]->fitness) / 2;
		const std::size_t portion = std::max(std::round(((double)fitness / totalFitness) * agents.size()), 1.0);
		const std::size_t len = std::min(agents.size(), j + portion);
		bool isAb = true;
		for (; j < len; j++)
		{
			CrossOver(agents[i], agents[i + 1], ab, ba);
			newChromosomes[j] = isAb ? ab : ba;
			isAb = !isAb;
		}

		if (j >= agents.size())
			break;
	}

	for (std::size_t i = 1; i < j; i++)
	{
		agents[i]->chromosome = newChromosomes[i];
	}

	for (; j < agents.size(); j++)
	{
		agents[j]->chromosome = agents[0]->chromosome;
	}
}

void AIController::Mutate(Agent* a, std::size_t index)
{

#if CONSTANT_MUTATION_AMOUNT

	constexpr std::size_t mutationAmount = std::max(NUM_GENES / 3,1u);

#elif INCREMENTAL_MUTATION_AMOUNT

	const std::size_t mutationAmount = ((double)index / agents.size()) * NUM_GENES;

#elif PROGRESS_MUTATION_AMOUNT

	const double progress = ((double)a->fitness / 200.0);
	const std::size_t mutationAmount = std::max(std::round((1.0f - progress) * NUM_GENES),1.0);

#endif

	//spend 3 generations improving position, followed by 3 generations improving tower type
#if INTERVAL_MUTATION

	constexpr auto interval = 3;
	static bool isTowerMutation = false;

	if (generation % interval == 0)
	{
		isTowerMutation = !isTowerMutation;
	}

	if (isTowerMutation)
	{
		MutateTower(a->chromosome, mutationAmount);
	}
	else 
	{
		MutatePosition(a->chromosome, mutationAmount);
	}

	//mutate both position and tower consistently
#elif CONSISTENT_MUTATION

	MutateTower(a->chromosome, mutationAmount);
	MutatePosition(a->chromosome, mutationAmount);

#endif
}

void AIController::CrossOver(Agent* a, Agent* b, Chromosome& abChromo, Chromosome& baChromo)
{
#if ONE_POINT_CROSSOVER

	OnePointCrossOver(double)a->fitness / (a->fitness + b->fitness), a->chromosome, b->chromosome, abChromo, baChromo);

#elif RANDOM_CROSSOVER

	RandomCrossOver((double)a->fitness / (a->fitness + b->fitness), a->chromosome, b->chromosome, abChromo, baChromo);

#elif INTERLEAVE_CROSSOVER

	InterleaveCrossOver(a->chromosome, b->chromosome, abChromo, baChromo);

#endif
};

void AIController::RouletteSort(AgentArray& agents)
{
	int32_t totalFitness = 0ull;
	for (const Agent* const agent : agents)
	{
		totalFitness += agent->fitness;
	}

	int32_t fitnessSoFar = 0ull;
	for (std::size_t i = 0; i < agents.size(); i++)
	{
		int32_t roll = Random::UniformInt(0, totalFitness - fitnessSoFar);
		std::size_t k = i;

		for (std::size_t j = i; j < agents.size(); j++)
		{
			k = j;
			roll -= agents[j]->fitness;

			if (roll <= 0)
			{
				break;
			}
		}

		fitnessSoFar += agents[k]->fitness;
		std::swap(agents[i], agents[k]);
	}
}

void AIController::TournamentSort(AgentArray& agents)
{
	std::sort(agents.begin(), agents.end(), [](const Agent* a, const Agent* b)-> bool { return a->fitness > b->fitness; });
}


void AIController::OnePointCrossOver(const double weight, const Chromosome& a, const Chromosome& b, Chromosome& _out_ab, Chromosome& _out_ba)
{
	//point can be  in the middle, a weighted position, or at a random position:
#define MIDDLE_POINT 0
#define WEIGHTED_POINT 1
#define RANDOM_POINT 0

#if MIDDLE_POINT

	constexpr std::size_t point = NUM_GENES / 2;

#elif WEIGHTED_POINT

	const std::size_t point = (NUM_GENES - 1) * weight;

#elif RANDOM_POINT

		const std::size_t point = Random::UniformInt(0u, NUM_GENES - 1);

#endif

	//perform the cross over around the given point:

	for (std::size_t i = 0; i < point; i++)
	{
		_out_ab.positions[i] = a.positions[i];
		_out_ab.towers[i] = a.towers[i];
		_out_ba.positions[i] = b.positions[i];
		_out_ba.towers[i] = b.towers[i];
	}

	for (std::size_t i = point; i < NUM_GENES; i++)
	{
		_out_ab.positions[i] = b.positions[i];
		_out_ab.towers[i] = b.towers[i];
		_out_ba.positions[i] = a.positions[i];
		_out_ba.towers[i] = a.towers[i];
	}
}

void AIController::RandomCrossOver(const double weight, const Chromosome& a, const Chromosome& b, Chromosome& _out_ab, Chromosome& _out_ba)
{
	for (std::size_t i = 0; i < NUM_GENES; i++)
	{
		const double roll = Random::UniformReal(0.0, 1.0);

		if (roll <= weight)
		{
			_out_ab.positions[i] = a.positions[i];
			_out_ab.towers[i] = a.towers[i];
			_out_ba.positions[i] = b.positions[i];
			_out_ba.towers[i] = b.towers[i];
		}
		else
		{
			_out_ab.positions[i] = b.positions[i];
			_out_ab.towers[i] = b.towers[i];
			_out_ba.positions[i] = a.positions[i];
			_out_ba.towers[i] = a.towers[i];
		}
	}
}

void AIController::InterleaveCrossOver(const Chromosome& a, const Chromosome& b, Chromosome& _out_ab, Chromosome& _out_ba)
{
	constexpr uint32_t sectionLength = 2u;
	bool isA = true;

	for (std::size_t i = 0; i < NUM_GENES; i++)
	{
		if ((i % sectionLength) == 0)
		{
			isA = !isA;
		}

		if (isA)
		{
			_out_ab.positions[i] = a.positions[i];
			_out_ab.towers[i] = a.towers[i];
			_out_ba.positions[i] = b.positions[i];
			_out_ba.towers[i] = b.towers[i];
		}
		else
		{
			_out_ab.positions[i] = b.positions[i];
			_out_ab.towers[i] = b.towers[i];
			_out_ba.positions[i] = a.positions[i];
			_out_ba.towers[i] = a.towers[i];
		}
	}
}

void AIController::MutatePosition(Chromosome& chromosome, const std::size_t iterations)
{
	for (std::size_t i = 0; i < iterations; i++)
	{
		const std::size_t roll = Random::UniformInt(0u, chromosome.positions.size() - 1);
		const std::size_t tower = (std::size_t)chromosome.towers[roll];
		const std::size_t posRoll = Random::UniformInt(0u, positionsInRangeByTower[tower].size() - 1);;

		chromosome.positions[roll] = positionsInRangeByTower[tower][posRoll];
	}
}

void AIController::MutateTower(Chromosome& chromosome, const std::size_t iterations)
{
	for (std::size_t i = 0; i < iterations; i++)
	{
		const std::size_t roll = Random::UniformInt(0u, chromosome.towers.size() - 1);
		const std::size_t tower = Random::UniformInt(1u, (std::size_t)TowerType::count - 1);
		chromosome.towers[roll] = (TowerType)tower;

		//if the tower is no longer in range of the path
		if (!m_gameBoard->inRangeOfPath(chromosome.positions[roll].x, chromosome.positions[roll].y, (TowerType)tower))
		{
			//set it to a position in range of the path.
			const std::size_t posRoll = Random::UniformInt(0u, positionsInRangeByTower[tower].size() - 1);
			chromosome.positions[roll] = positionsInRangeByTower[tower][posRoll];
		}
	}
}


/*
std::array<std::string, (std::size_t)TowerType::count> towerTypeStrings
{
	"empty",
	"slammer",
	"swinger",
	"thrower"
};
*/

void AIController::update()
{
	if (m_Timer == nullptr)
		return;

	//Calculation of delta time for timing.
	//const double seconds = floor(m_Timer->elapsedSeconds());
	//const double deltaTime = seconds - elapsedSeconds;
	//if (seconds > elapsedSeconds)
	//{
	//	elapsedSeconds = seconds;
	//}

	//Try to place a tower:

	const auto& type = currentAgent->chromosome.towers[currentTowerIndex];
	const auto& pos = currentAgent->chromosome.positions[currentTowerIndex];

	
	std::cout << "CHROMO:\n";
	std::cout << "Positions:\n";

	for (const auto& pos : currentAgent->chromosome.positions)
	{
		std::cout << "Vector2f(" << pos.x << "," << pos.y << "),\n";
	}

	std::cout << "Towers:\n";

	for (const auto& tower : currentAgent->chromosome.towers)
	{
		std::cout << (std::size_t)tower << ",\n";
	}
	

	if (m_gameBoard->towerIsPurchasable(type))
	{
		addTower(type, pos.x, pos.y);
	}

	recordScore();
}

void AIController::addTower(TowerType type, int32_t gridx, int32_t gridy)
{
	m_gameBoard->addTower(type, gridx, gridy);

	//Increment to the next tower by default.
	const std::size_t prev = currentTowerIndex;
	currentTowerIndex++;
	if (currentTowerIndex >= NUM_GENES)
		currentTowerIndex = 0;

	//Find the next tower with available space (if the default isn't already)
	sf::Vector2i pos = currentAgent->chromosome.positions[currentTowerIndex];
	while (currentTowerIndex != prev && !m_gameBoard->gridSpaceAvailable(pos.x, pos.y))
	{
		currentTowerIndex++;
		if (currentTowerIndex >= NUM_GENES)
			currentTowerIndex = 0;

		pos = currentAgent->chromosome.positions[currentTowerIndex];
	}
}

void AIController::Init()
{
	InitPositionsInRangeByTower();
	Initialisation(agents);
}

void AIController::setupBoard()
{
	m_Timer->start();
}

int32_t AIController::recordScore()
{
	int32_t currentWave = m_gameState->getCurrentWave();
	const int32_t killCount = m_gameState->getMonsterEliminated();
	currentWave *= 10; // living longer is good
	const int32_t score = currentWave + killCount;

	static int32_t iteration = 0;

	if (iteration == 0)
		cout << "iteration" << "," << "wave" << "," << "kills" << "," << "score" << endl;

	cout << iteration << "," << m_gameState->getCurrentWave() << "," << m_gameState->getMonsterEliminated() << "," << score << endl;
	iteration++;

	m_gameState->setScore(score);

	return score;
}
