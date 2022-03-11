#pragma once
#include <stdint.h>
#include <array>
#include <bitset>
#include <vector>
#include "Constants.h"
#include "Vector2.h"
#include "ITowerDefence.h"

///.-----------------------------------------------------------------\.
//			Chromosome & Agent
//\.-----------------------------------------------------------------./

constexpr std::size_t NUM_GENES = 15u;
constexpr std::size_t NUM_AGENTS = 20u;

struct Chromosome
{
	std::array<Vector2, NUM_GENES> positions;
	std::array<TowerType, NUM_GENES> towers;
};

using ChromosomeArray = std::array<Chromosome, NUM_AGENTS>;


struct Agent
{
	std::size_t fitness = 0;
	Chromosome chromosome;
};

using AgentArray = std::array<Agent*, NUM_AGENTS>;


///.-----------------------------------------------------------------\.
//		Mutation, CrossOver & Selection strategy pointer types:
//\.-----------------------------------------------------------------./

class AIController;
using MutationStrategy = void(AIController::*)(AgentArray& agents);
using CrossOverStrategy = void(AIController::*)(const Chromosome& parent1, const Chromosome& parent2, Chromosome& child1, Chromosome& child2);
using SelectionStrategy = void (AIController::*)(const AgentArray& parents, AgentArray& children, CrossOverStrategy crossOverStrategy);

///.-----------------------------------------------------------------\.
//			AI Controller
//\.-----------------------------------------------------------------./

class AIController
{
public:
	AIController();
	~AIController();

	void Init();

	int32_t GetGeneration() const;
	int32_t GetHighestScore() const;
	Chromosome GetCurrentChromosome(); 
	void SetTowerDefence(ITowerDefence* _towerDefence) {towerDefence = _towerDefence;};
	void update();
	void gameOver();

	void OutputFitnessToFile(const std::string& fileName);
	void OutputChromosomeToFile(const std::string& fileName);
	void InputChromosomeFromFile(const std::string& fileName);

	//debug text 
	inline const std::string& getDebugText() const { return debugText; }

private:

	void addTower(TowerType type, int32_t gridx, int32_t gridy);
	int32_t recordScore();
	void InitPositionsInRangeByTower();

	//Functions that must be implemented according to the mark scheme:
	void Initialisation(AgentArray& agents);
	std::size_t CalculateFitness(Agent* agent);
	void CrossOver(AgentArray& agents);
	void Mutate(AgentArray& agents);

	void ApportCrossOver(AgentArray& agents);
	void Mutate(Agent* a, std::size_t index);
	void CrossOver(Agent* a, Agent* b, Chromosome& abChromo, Chromosome& baChromo);

private:

	///.-----------------------------------------------------------------\.
	//			Selection strategies
	//\.-----------------------------------------------------------------./

	void RouletteSort(AgentArray& agents);
	void TournamentSort(AgentArray& agents);

	void SteadyStateSelection(const AgentArray& parents, AgentArray& children, CrossOverStrategy crossOverStrategy);
	void RoulleteWheelSelection(const AgentArray& parents, AgentArray& children, CrossOverStrategy crossOverStrategy);
	void StochasticUniversalSampling(const AgentArray& parents, AgentArray& children, CrossOverStrategy crossOverStrategy);
	void TournamentSelection(const AgentArray& parents, AgentArray& children, CrossOverStrategy crossOverStrategy);
	void ApportionSelection(const AgentArray& parents, AgentArray& children, CrossOverStrategy crossOverStrategy);

private:

	///.-----------------------------------------------------------------\.
	//			Crossover strategies
	//\.-----------------------------------------------------------------./

	void OnePointCrossOver(const double weight, const Chromosome& parent1, const Chromosome& parent2, Chromosome& child1, Chromosome& child2);
	void InterleaveCrossOver(const Chromosome& parent1, const Chromosome& parent2, Chromosome& child1, Chromosome& child2);
	void NPointCrossOver(const Chromosome& parent1, const Chromosome& parent2, Chromosome& child1, Chromosome& child2);
	void UniformCrossOver(const Chromosome& parent1, const Chromosome& parent2, Chromosome& child1, Chromosome& child2);

private:

	///.-----------------------------------------------------------------\.
	//			Mutation strategies
	//\.-----------------------------------------------------------------./

	void StandardMutation(AgentArray& agents);
	void IncrementalMutation(AgentArray& agents);
	void ProgressiveMutation(AgentArray& agents);

private:


	///.-------------------------------------------------------------------\.
	//			Mutate techniques (scramble, change tower, change position
	//\.-------------------------------------------------------------------./

	void MutateTower(Chromosome& chromosome, const std::size_t iterations);
	void MutatePosition(Chromosome& chromosome, const std::size_t iterations);
	void MutateScramble(Chromosome& chromosome, const std::size_t iterations);

private:

	ITowerDefence* towerDefence;

	//for mutation (avoid placing towers out of range)
	std::array<std::vector<Vector2>, (std::size_t)TowerType::count> positionsInRangeByTower;

	//Strategies 
	SelectionStrategy selectionStrategy;
	CrossOverStrategy crossOverStrategy;
	MutationStrategy mutationStrategy;

	//Agents (each agent is tested in turn)
	Agent* currentAgent = nullptr;

	AgentArray previousGeneration;
	AgentArray agents;
	std::size_t currentAgentIndex = 0;
	std::size_t currentTowerIndex = 0;

	//generation & fitnesss values
	int32_t generation = 1;
	std::size_t highestFitness = 0;

	//For Output
	double elapsedSeconds;
	std::string debugText = "Generation : 1\nCurrent AI Index : 0\nPrevious score :0";
	std::vector<int> fitnessByGeneration;
};


