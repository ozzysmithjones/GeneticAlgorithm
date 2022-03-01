#pragma once
#include <bitset>
#include <array>
#include <algorithm>
#include <vector>
#include "Vector2.h"
#include "ITowerDefence.h"

/*
enum class TowerType
{
	SLAMMER, SWINGER, THROWER, COUNT,
};
*/

enum class MonsterType
{
	SLOW, REGULAR, FAST, COUNT,
};

struct Projectile
{
	float x = 0, y = 0;
	float speed = 8;
	int damage = 1;

	Projectile();
	Projectile(float x, float y, int damage);
};

struct Tower
{
	TowerType type;
	int cost = 1;
	int damage = 1;
	int range = 1;
	int cooldown = 1;
	int currentCooldown = 0;
	Vector2 position{ 0,0 };

	Tower();
	Tower(int cost, int damage, int range, int cooldown, TowerType type);
};

struct Monster
{
	MonsterType type;
	int value = 1;
	int health = 1;
	int damage = 1;
	int speed = 1;

	int pathIndex = 0;
	Vector2 position{ 0,0 };
	std::vector<Projectile> projectiles;

	Monster();
	Monster(int value, int health, int damage, int speed, MonsterType type);
};


class Game : public ITowerDefence
{
private:

	int score = 0;
	int health = 50;
	int money = 100;
	int waveCounter = 0;
	int killCounter = 0;
	int currentWave = 1;
	float difficulty = 1.0f;

	std::bitset<WIDTH* HEIGHT> collision{ 0 };
	std::vector<Monster> monsters;
	std::vector<Tower> towers;
	std::array<Monster, (std::size_t)MonsterType::COUNT> monsterTemplates;
	std::array<Tower, (std::size_t)TowerType::count> towerTemplates;

private:

	void SpawnMonster(MonsterType monsterType, int number);
	bool InRange(const Vector2& position, const Vector2& target, float range);
	void FirstWave();
	void RegularWave();

public:

	Game();

	virtual int GetScore() const override;
	void Start();
	bool Update();
	void Render();
	virtual bool IsSpaceForTower(int x, int y) override;
	virtual bool IsMoneyForTower(TowerType towerType) override;
	virtual bool IsTowerInRangeOfPath(int x, int y, TowerType towerType) override;
	virtual bool PlaceTower(int x, int y, TowerType towerType) override;
};