/*
 * GameState.h
 *
 *  Created on: Nov 21, 2018
 *      Author: Chris
 */

#ifndef GAMESTATE_H_
#define GAMESTATE_H_
#include "Timer.h"
#include "Tower.h"
#include "Constants.h"

class GameState
{
private:
	// This contains info about tower pricing, damage, etc
	std::map<TowerType, std::map<string, int>> towerProps;
	std::map<MonsterType, std::map<string, int>> MonsterProps;

	// Should the help menu be active
	bool helperState = true;

	int health = 50;
	int tams = 100;
	int currentWave = 1;
	int MonsterRemaining = -1;
	int score = 0;
	int MonsterEliminated = 0;
	int tamsPayed = 0; // total number of drams payed, could be good as a negative fitness.
	int waveCounter = 0; // each time it reaches 900, a new wave starts (good to see how close to the next wave we are)

	// This is used by the upgrade button
	TowerType purchaseTower = TowerType::empty;
	Tower* boardTower = nullptr;

	//for debug rendering.
	std::string debugText = "First wave";

public:
	GameState(Timer* timer);
	virtual ~GameState();

	// This is the pixel length of a cubit
	const int cubit = 60;
	bool dirtyBit = true;
	Timer* timer;
	sf::Vector2i mousePos;


	int getScore() { return score; }
	void setScore(const int scoreIn) { score = scoreIn; }

	const std::string& GetDebugText();

	//Accessors
	int getHealth();
	int getTams();
	int GetTamsPayed() const;

	int getCurrentWave();
	void setCurrentWave(int wave);
	int GetWaveCounter() const;
	void SetWaveCounter(int waveTimer);
	int getMonsterRemaining();
	int getMonsterEliminated();
	bool getHelperState();

	void SetDebugText(const std::string& text);

	// This is used by the GameMenuController to determine opacity,
	// as well as determining which tower the towerController should buy
	TowerType getPurchaseTower();
	void setPurchaseTower(TowerType towerType);
	Tower* getBoardTower();
	void setBoardTower(Tower* tower);

	std::map<string, int> getTowerProps(TowerType type);
	std::map<string, int> getMonsterProps(MonsterType type);

	void startGame(); // Starts game
	void endGame(); // Ends game
	void startWave(); // Starts wave
	void updateHealthBy(int update); // Update player lives
	void updateTamBy(int update); // Update player currency
	void toggleHelpScreen();

	void monsterEliminated() { MonsterEliminated++; }
};

#endif /* GAMESTATE_H_ */
