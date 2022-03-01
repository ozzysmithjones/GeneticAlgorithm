#include "AIController.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <fstream>
#include "Random.h"
#include "json.hpp"

using namespace std;


///.-----------------------------------------------------------------\.
//				AI controller
//\.-----------------------------------------------------------------./

AIController::AIController()
	: currentAgentIndex(0)
{
	for (auto& agent : agents)
	{
		agent = new Agent();
	}

	for (auto& agent : previousGeneration)
	{
		agent = new Agent();
	}

	currentAgent = agents[0];

	//Init strategies:
	mutationStrategy = &AIController::StandardMutation;
	crossOverStrategy = &AIController::NPointCrossOver;
	selectionStrategy = &AIController::ApportionSelection;
}

AIController::~AIController()
{
	for (const auto& agent : agents)
	{
		delete agent;
	}

	for (const auto& agent : previousGeneration)
	{
		delete agent;
	}
}

void AIController::gameOver()
{
	currentTowerIndex = 0;
	currentAgent->fitness = CalculateFitness(currentAgent);
	currentAgentIndex++;

	if (currentAgentIndex >= agents.size())
	{
		//End of generation.
		generation++;
		currentAgentIndex = 0;

		//sort agents by fitness
		std::sort(agents.begin(), agents.end(), [](const Agent* a, const Agent* b)-> bool { return a->fitness > b->fitness; });

		//current generation is now previous generation. (with some elitism to maintain the best across generations)
		constexpr auto elitism = 10;

		for (std::size_t i = 0; i < agents.size(); i++)
		{
			if (i >= elitism || previousGeneration[i]->fitness < agents[i]->fitness)
			{
				*previousGeneration[i] = *agents[i];
			}
			else 
			{
				*agents[i] = *previousGeneration[i];
			}
		}

		highestFitness = 0u;
		for (const auto& agent : previousGeneration)
		{
			highestFitness = std::max(highestFitness, agent->fitness);
		}

		//for logging:
		fitnessByGeneration.push_back(highestFitness);

		std::sort(previousGeneration.begin(), previousGeneration.end(), [](const Agent* a, const Agent* b)-> bool { return a->fitness > b->fitness; });

		//Generate child Generation:
		(this->*selectionStrategy)(previousGeneration, agents, crossOverStrategy);
		(this->*mutationStrategy)(agents);
	}

	currentAgent = agents[currentAgentIndex];
	elapsedSeconds = 0;
	currentTowerIndex = 0;

	//update debugging text:
	std::stringstream stream;
	stream << "Generation : " << generation << "\nCurrent AI Index : " << currentAgentIndex << "\nPrevious fitness : " << currentAgent->fitness;
	debugText = stream.str();
}

void AIController::OutputFitnessToFile(const std::string& fileName)
{
	ofstream file(fileName);

	if (file.is_open())
	{
		for (const auto& fitness : fitnessByGeneration)
		{
			file << fitness << ",\n";
		}

		file.close();
	}
}

void AIController::OutputChromosomeToFile(const std::string& fileName)
{
	nlohmann::json j;

	nlohmann::json towers;
	for (const auto& t : agents[currentAgentIndex]->chromosome.towers)
	{
		towers.push_back((int)t);
	}

	j.push_back(towers);

	nlohmann::json positions;
	for (const auto& p : agents[currentAgentIndex]->chromosome.positions)
	{
		positions.push_back({ p.x,p.y });
	}

	j.push_back(positions);

	//output json to file.
	ofstream file(fileName);

	if (file.is_open())
	{
		file << j;
		file.close();
	}
}

void AIController::InputChromosomeFromFile(const std::string& fileName)
{
	Chromosome& chrom = agents[currentAgentIndex]->chromosome;
	ifstream file(fileName);

	if (file.is_open())
	{
		nlohmann::json j = nlohmann::json::parse(file);

		//towers:
		for (std::size_t i = 0; i < j[0].size(); i++)
		{
			chrom.towers[i] = (TowerType)(j[0][i].get<int>());
		}

		//positions:
		for (std::size_t i = 0; i < j[1].size(); i++)
		{
			chrom.positions[i].x = j[1][i][0].get<int>();
			chrom.positions[i].y = j[1][i][1].get<int>();
		}

		file.close();
	}
}


void AIController::InitPositionsInRangeByTower()
{
	std::array<std::bitset<WIDTH* HEIGHT>, (std::size_t)TowerType::count> bitboardByTower;

	//Init the bitboards with a bit representing that the tower in that position is in range.
	for (std::size_t i = 1; i < (std::size_t)TowerType::count; i++)
	{
		for (std::size_t y = 0; y < HEIGHT; y++)
		{
			for (std::size_t x = 0; x < WIDTH; x++)
			{
				bitboardByTower[i][y * WIDTH + x] = towerDefence->IsTowerInRangeOfPath(x, y, (TowerType)i);
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
}

std::size_t AIController::CalculateFitness(Agent* agent)
{
	return towerDefence->GetScore();
}

void AIController::CrossOver(AgentArray& agents)
{
	ApportCrossOver(agents);
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
	TournamentSort(agents);
	constexpr std::size_t selectionAmount = NUM_AGENTS / 2;

	Chromosome ab;
	Chromosome ba;

	std::array<Chromosome, NUM_GENES> newChromosomes;
	std::size_t totalFitness = 0;
	std::size_t j = 1;
	for (std::size_t i = 0; i < selectionAmount - 1; i += 2)
	{
		totalFitness += (agents[i]->fitness + agents[i + 1]->fitness) / 2;
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
	const std::size_t mutationAmount = ((double)index / agents.size()) * NUM_GENES;

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
	UniformCrossOver(a->chromosome, b->chromosome, abChromo, baChromo);
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

void AIController::SteadyStateSelection(const AgentArray& parents, AgentArray& children, CrossOverStrategy crossOverStrategy)
{
	constexpr int numSelected = 3;

	for (std::size_t i = 0; i < numSelected - 1; i++)
	{
		(this->*crossOverStrategy)(parents[i]->chromosome, parents[i + 1]->chromosome, children[children.size() - (i + 1)]->chromosome, children[children.size() - (i + numSelected + 1)]->chromosome);
	}
}

void AIController::RoulleteWheelSelection(const AgentArray& parents, AgentArray& children, CrossOverStrategy crossOverStrategy)
{
	constexpr int numSelected = NUM_AGENTS / 2;
	static_assert(numSelected <= NUM_AGENTS, "num selected must not be more than agents.size()");

	std::size_t totalFitness = 0;
	for (const auto& agent : parents)
	{
		totalFitness += agent->fitness;
	}

	for (std::size_t i = 0; i < numSelected; i++)
	{
		int roll = Random::UniformInt(0, (int)totalFitness);
		std::size_t j;

		for (j = 0; j < parents.size() - 1; j++)
		{
			roll -= parents[j]->fitness;
			if (roll <= 0)
				break;
		}

		const Agent* parent1 = parents[j];
		roll = Random::UniformInt(0, (int)(totalFitness - parent1->fitness));

		for (j = 0; j < parents.size() - 1; j++)
		{
			if (parents[j] != parent1)
			{
				roll -= parents[j]->fitness;
				if (roll <= 0)
					break;
			}
		}

		const Agent* parent2 = parents[j];
		(this->*crossOverStrategy)(parent1->chromosome, parent2->chromosome, children[i]->chromosome, children[i + numSelected]->chromosome);
	}
}

void AIController::StochasticUniversalSampling(const AgentArray& parents, AgentArray& children, CrossOverStrategy crossOverStrategy)
{
	constexpr int numSelected = NUM_AGENTS / 2;
	static_assert(numSelected <= NUM_AGENTS, "num selected must not be more than agents.size()");

	std::size_t totalFitness = 0;
	for (const auto& agent : parents)
	{
		totalFitness += agent->fitness;
	}

	//same as Roullete sort except the second parent is directly opposite on the wheel
	int mean = totalFitness / parents.size();

	for (std::size_t i = 0; i < numSelected; i++)
	{
		const int rollinit = Random::UniformInt(0, (int)totalFitness);
		int roll = rollinit;
		std::size_t j;

		for (j = 0; j < parents.size() - 1; j++)
		{
			roll -= parents[j]->fitness;
			if (roll <= 0)
				break;
		}

		const Agent* parent1 = parents[j];
		roll = (rollinit + mean) % totalFitness;

		for (j = 0; j < parents.size() - 1; j++)
		{
			if (parents[j] != parent1)
			{
				roll -= parents[j]->fitness;
				if (roll <= 0)
					break;
			}
		}

		const Agent* parent2 = parents[j];
		(this->*crossOverStrategy)(parent1->chromosome, parent2->chromosome, children[i]->chromosome, children[i + numSelected]->chromosome);
	}
}

const Agent* Tournament(const AgentArray& agents, const Agent* exclude = nullptr)
{
	constexpr int tournamentSize = 2;
	std::array<Agent*, tournamentSize> arena;

	for (std::size_t i = 0; i < arena.size(); i++)
	{
		std::size_t roll = Random::UniformInt(0u, agents.size() - 1);

		if (agents[roll] == exclude)
		{
			++roll;
			if (roll >= agents.size())
				roll = 0;
		}

		arena[i] = agents[roll];
	}

	std::size_t highestFitness = 0u;
	Agent* winner = arena[0];

	for (std::size_t i = 0; i < arena.size(); i++)
	{
		if (arena[i]->fitness > highestFitness || i == 0)
		{
			highestFitness = arena[i]->fitness;
			winner = arena[i];
		}
	}

	return winner;
}

void AIController::TournamentSelection(const AgentArray& parents, AgentArray& children, CrossOverStrategy crossOverStrategy)
{
	constexpr int numSelected = NUM_AGENTS / 2;
	static_assert(numSelected <= NUM_AGENTS, "num selected must not be more than agents.size()");

	for (std::size_t i = 0; i < numSelected; i++)
	{
		const Agent* parent1 = Tournament(parents);
		const Agent* parent2 = Tournament(parents, parent1);

		(this->*crossOverStrategy)(parent1->chromosome, parent2->chromosome, children[i]->chromosome, children[i + numSelected]->chromosome);
	}
}

void AIController::ApportionSelection(const AgentArray& parents, AgentArray& children, CrossOverStrategy crossOverStrategy)
{
	constexpr std::size_t selectionAmount = NUM_AGENTS / 2;

	Chromosome ab;
	Chromosome ba;

	std::size_t totalFitness = 0;
	std::size_t j = 0;

	for (std::size_t i = 0; i < selectionAmount - 1; i += 2)
	{
		totalFitness += (agents[i]->fitness + agents[i + 1]->fitness) / 2;
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
			children[j]->chromosome = isAb ? ab : ba;
			isAb = !isAb;
		}

		if (j >= agents.size())
			break;
	}

	for (; j < agents.size(); j++)
	{
		children[j]->chromosome = agents[0]->chromosome;
	}
}


void AIController::OnePointCrossOver(const double weight, const Chromosome& parent1, const Chromosome& parent2, Chromosome& child1, Chromosome& child2)
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
		child1.positions[i] = parent1.positions[i];
		child1.towers[i] = parent1.towers[i];
		child2.positions[i] = parent2.positions[i];
		child2.towers[i] = parent2.towers[i];
	}

	for (std::size_t i = point; i < NUM_GENES; i++)
	{
		child1.positions[i] = parent2.positions[i];
		child1.towers[i] = parent2.towers[i];
		child2.positions[i] = parent1.positions[i];
		child2.towers[i] = parent1.towers[i];
	}
}

void AIController::InterleaveCrossOver(const Chromosome& parent1, const Chromosome& parent2, Chromosome& child1, Chromosome& child2)
{
	uint32_t sectionLength = Random::UniformInt(2u, 4u);
	bool isA = (bool)Random::UniformInt(0u,1u);

	for (std::size_t i = 0; i < NUM_GENES; i++)
	{
		if ((i % sectionLength) == 0)
		{
			isA = !isA;
		}

		if (isA)
		{
			child1.positions[i] = parent1.positions[i];
			child1.towers[i] = parent1.towers[i];
			child2.positions[i] = parent2.positions[i];
			child2.towers[i] = parent2.towers[i];
		}
		else
		{
			child1.positions[i] = parent2.positions[i];
			child1.towers[i] = parent2.towers[i];
			child2.positions[i] = parent1.positions[i];
			child2.towers[i] = parent1.towers[i];
		}
	}
}

void AIController::NPointCrossOver(const Chromosome& parent1, const Chromosome& parent2, Chromosome& child1, Chromosome& child2)
{
	const std::size_t numPoints = Random::UniformInt(0u, NUM_GENES / 2);
	const double probability = (double)numPoints / NUM_GENES;
	std::size_t p = 0;

	bool isParent1 = true;

	for (std::size_t i = 0; i < NUM_GENES; i++)
	{
		if (isParent1)
		{
			child1.positions[i] = parent1.positions[i];
			child1.towers[i] = parent1.towers[i];
			child2.positions[i] = parent2.positions[i];
			child2.towers[i] = parent2.towers[i];
		}
		else
		{
			child1.positions[i] = parent2.positions[i];
			child1.towers[i] = parent2.towers[i];
			child2.positions[i] = parent1.positions[i];
			child2.towers[i] = parent1.towers[i];
		}

		double roll = Random::UniformReal(0.0, 1.0);
		if (p < numPoints && (roll <= probability || ((NUM_GENES - 1) - i)) <= (numPoints - p))
		{
			isParent1 = !isParent1;
			p++;
		}
	}
}

void AIController::UniformCrossOver(const Chromosome& parent1, const Chromosome& parent2, Chromosome& child1, Chromosome& child2)
{
	for (std::size_t i = 0; i < NUM_GENES; i++)
	{
		double roll = Random::UniformReal(0.0, 1.0);
		if (roll <= 0.5f)
		{
			child1.positions[i] = parent1.positions[i];
			child1.towers[i] = parent1.towers[i];
			child2.positions[i] = parent2.positions[i];
			child2.towers[i] = parent2.towers[i];
		}
		else
		{
			child1.positions[i] = parent2.positions[i];
			child1.towers[i] = parent2.towers[i];
			child2.positions[i] = parent1.positions[i];
			child2.towers[i] = parent1.towers[i];
		}
	}
}

/// <summary>
/// All agents mutated by a constant amount
/// </summary>
void AIController::StandardMutation(AgentArray& agents)
{
	for (auto& agent : agents)
	{
		MutateScramble(agent->chromosome, Random::UniformInt(0u, NUM_GENES-1) / 3);
		MutateTower(agent->chromosome, Random::UniformInt(0u, NUM_GENES - 1) / 3);
		MutatePosition(agent->chromosome, Random::UniformInt(0u, NUM_GENES - 1) / 3);
	}
}

/// <summary>
/// The higher the index of the agent, the more that the agent is mutated
/// </summary>
void AIController::IncrementalMutation(AgentArray& agents)
{
	for (std::size_t i = 0; i < agents.size(); i++)
	{
		const std::size_t amount = std::max((std::size_t)((double(i) / agents.size()) * NUM_GENES),2u);
		//MutateScramble(agents[i]->chromosome, amount);
		MutateTower(agents[i]->chromosome, amount);
		MutatePosition(agents[i]->chromosome, amount);
	}
}

/// <summary>
/// The closer to the goal, the more that the agent is mutated
/// </summary>
void AIController::ProgressiveMutation(AgentArray& agents)
{
	for (std::size_t i = 0; i < agents.size(); i++)
	{
		const double progress = ((double)agents[i]->fitness) / 200.0f;
		const std::size_t amount = std::max((std::size_t)(progress * NUM_GENES), 2u);

		MutateTower(agents[i]->chromosome, amount);
		MutatePosition(agents[i]->chromosome, amount);
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

void AIController::MutateScramble(Chromosome& chromosome, const std::size_t iterations)
{
	for (std::size_t i = 0; i < iterations; i++)
	{
		const std::size_t roll1 = Random::UniformInt(0u, chromosome.towers.size() - 1);
		const std::size_t roll2 = Random::UniformInt(0u, chromosome.towers.size() - 1);

		std::swap(chromosome.positions[roll1], chromosome.positions[roll2]);
		std::swap(chromosome.towers[roll1], chromosome.towers[roll2]);
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
		if (!towerDefence->IsTowerInRangeOfPath(chromosome.positions[roll].x, chromosome.positions[roll].y, (TowerType)tower))
		{
			//set it to a position in range of the path.
			const std::size_t posRoll = Random::UniformInt(0u, positionsInRangeByTower[tower].size() - 1);
			chromosome.positions[roll] = positionsInRangeByTower[tower][posRoll];
		}
	}
}

void AIController::update()
{
	//Try to place a tower:

	const auto& type = currentAgent->chromosome.towers[currentTowerIndex];
	const auto& pos = currentAgent->chromosome.positions[currentTowerIndex];

	if (towerDefence->IsMoneyForTower(type))
	{
		addTower(type, pos.x, pos.y);
	}
}

void AIController::addTower(TowerType type, int32_t gridx, int32_t gridy)
{
	towerDefence->PlaceTower(gridx, gridy, type);

	//Increment to the next tower by default.
	const std::size_t prev = currentTowerIndex;
	currentTowerIndex++;
	if (currentTowerIndex >= NUM_GENES)
		currentTowerIndex = 0;

	//Find the next tower with available space (if the default isn't already)
	Vector2 pos = currentAgent->chromosome.positions[currentTowerIndex];
	while (currentTowerIndex != prev && !towerDefence->IsSpaceForTower(pos.x, pos.y))
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

int32_t AIController::GetGeneration() const
{
	return generation;
}

int32_t AIController::GetHighestScore() const
{
	return highestFitness;
}

Chromosome AIController::GetCurrentChromosome()
{
	return agents[currentAgentIndex]->chromosome;
}

int32_t AIController::recordScore()
{
	/* MOVED to tower defence/game  class
	* 
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
	*/

	return 0;
}
