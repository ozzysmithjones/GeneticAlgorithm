#pragma once
#include <stdint.h>
#include <array>
#include <bitset>
#include <vector>
#include <SFML/Graphics.hpp>
//#include "Constants.h"
#include "ITowerDefence.h"
#include "Random.h"


struct Agent;

constexpr std::size_t NUM_GENES = 15u;
constexpr std::size_t NUM_AGENTS = 10u;
using AgentArray = std::array<Agent*, NUM_AGENTS>;

///.-----------------------------------------------------------------\.
//			Genetic algorithm method can be changed here.
//\.-----------------------------------------------------------------./

//Selection Process:
#define ROULETTE_SORT 0
#define TOURNAMENT_SORT 1
#define APPORTIONMENT 1

#define SINGLE_SELECTION_AMOUNT 0
#define HALF_SELECTION_AMOUNT 1
#define DISTRIBUTION_SELECTION_AMOUNT 0
#define PROGRESS_SELECTION_AMOUNT 0

//Crossover process:
#define ONE_POINT_CROSSOVER 0
#define RANDOM_CROSSOVER 1
#define INTERLEAVE_CROSSOVER 0

//Mutation process:
#define CONSTANT_MUTATION_AMOUNT 0
#define INCREMENTAL_MUTATION_AMOUNT 1
#define PROGRESS_MUTATION_AMOUNT 0

#define CONSISTENT_MUTATION 0
#define INTERVAL_MUTATION 1

///.-----------------------------------------------------------------\.
//			Chromosome & Agent
//\.-----------------------------------------------------------------./


struct Chromosome
{
	std::array<sf::Vector2i, NUM_GENES> positions;
	std::array<TowerType, NUM_GENES> towers;
};

struct Agent
{
	std::size_t fitness = 0;
	Chromosome chromosome;
};

///.-----------------------------------------------------------------\.
//			AI Controller
//\.-----------------------------------------------------------------./

class AIController
{
public:
	AIController();
	~AIController();

	void Init();
	const Chromosome& GetCurrentChromosome();
	void SetTowerDefence(ITowerDefence* _towerDefence) { towerDefence = _towerDefence; };
	void update();
	void addTower(TowerType type, int32_t gridx, int32_t gridy);
	void gameOver();

	//debug text 
	inline const std::string& getDebugText() const { return debugText; }

private:


	void InitPositionsInRangeByTower();
	//Functions that must be implemented according to the mark scheme:
	void Initialisation(AgentArray& agents);
	std::size_t CalculateFitness(Agent* agent);
	void CrossOver(AgentArray& agents);
	void Mutate(AgentArray& agents);


	void ApportCrossOver(AgentArray& agents);

	ITowerDefence* towerDefence;

private:

	void Mutate(Agent* a, std::size_t index);
	void CrossOver(Agent* a, Agent* b, Chromosome& abChromo, Chromosome& baChromo);

	void RouletteSort(AgentArray& agents);
	void TournamentSort(AgentArray& agents);

	void OnePointCrossOver(const double weight, const Chromosome& a, const Chromosome& b, Chromosome& _out_ab, Chromosome& _out_ba);
	void RandomCrossOver(const double weight, const Chromosome& a, const Chromosome& b, Chromosome& _out_ab, Chromosome& _out_ba);
	void InterleaveCrossOver(const Chromosome& a, const Chromosome& b, Chromosome& _out_ab, Chromosome& _out_ba);

	void MutateTower(Chromosome& chromosome, const std::size_t iterations);
	void MutatePosition(Chromosome& chromosome, const std::size_t iterations);

	//for mutation
	std::array<std::vector<sf::Vector2i>, (std::size_t)TowerType::count> positionsInRangeByTower;

	//Agents (each agent is tested in turn)
	Agent* currentAgent = nullptr;
	AgentArray agents;
	std::size_t currentAgentIndex = 0;
	std::size_t currentTowerIndex = 0;

	int32_t generation = 1;
	std::size_t highestFitness = 0;
	std::size_t outHighestFitness = 0;

	//For Output
	double elapsedSeconds;
	std::string debugText = "Generation : 1\nCurrent AI Index : 0\nPrevious score :0";
};
