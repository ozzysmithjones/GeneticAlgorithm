#include "Game.h"
#include <chrono>
#include <iostream>
#include <thread>

constexpr float projectileHitRadius = 20.0f;

const std::array<Vector2, 15> pathCoords =
{
	Vector2(15, 0), Vector2(15, 4), Vector2(20, 4),
	Vector2(22, 4), Vector2(22, 8), Vector2(10, 8),
	Vector2(10, 6), Vector2(4, 6), Vector2(4, 13),
	Vector2(12, 13), Vector2(12, 15),Vector2(17, 15),
	Vector2(17, 12), Vector2(21, 12), Vector2(21, 18)
};

const std::array<Vector2, 15> pathWaypoints =
{
	Vector2(870, -30),  Vector2(870, 210),  Vector2(1170, 210),
	Vector2(1290, 210), Vector2(1290, 450), Vector2(570, 450),
	Vector2(570, 330),  Vector2(210, 330),  Vector2(210, 750),
	Vector2(690, 750),  Vector2(690, 870),  Vector2(990, 870),
	Vector2(990, 690),  Vector2(1230, 690), Vector2(1230, 1050),
};


const std::array<Vector2, 71> pathTiles
{
	Vector2(870, -30),
	Vector2(870, 30),
	Vector2(870, 90),
	Vector2(870, 150),
	Vector2(870, 210),
	Vector2(930, 210),
	Vector2(990, 210),
	Vector2(1050, 210),
	Vector2(1110, 210),
	Vector2(1170, 210),
	Vector2(1230, 210),
	Vector2(1290, 210),
	Vector2(1290, 270),
	Vector2(1290, 330),
	Vector2(1290, 390),
	Vector2(1290, 450),
	Vector2(1230, 450),
	Vector2(1170, 450),
	Vector2(1110, 450),
	Vector2(1050, 450),
	Vector2(990, 450),
	Vector2(930, 450),
	Vector2(870, 450),
	Vector2(810, 450),
	Vector2(750, 450),
	Vector2(690, 450),
	Vector2(630, 450),
	Vector2(570, 450),
	Vector2(570, 390),
	Vector2(570, 330),
	Vector2(510, 330),
	Vector2(450, 330),
	Vector2(390, 330),
	Vector2(330, 330),
	Vector2(270, 330),
	Vector2(210, 330),
	Vector2(210, 390),
	Vector2(210, 450),
	Vector2(210, 510),
	Vector2(210, 570),
	Vector2(210, 630),
	Vector2(210, 690),
	Vector2(210, 750),
	Vector2(270, 750),
	Vector2(330, 750),
	Vector2(390, 750),
	Vector2(450, 750),
	Vector2(510, 750),
	Vector2(570, 750),
	Vector2(630, 750),
	Vector2(690, 750),
	Vector2(690, 810),
	Vector2(690, 870),
	Vector2(750, 870),
	Vector2(810, 870),
	Vector2(870, 870),
	Vector2(930, 870),
	Vector2(990, 870),
	Vector2(990, 810),
	Vector2(990, 750),
	Vector2(990, 690),
	Vector2(1050, 690),
	Vector2(1110, 690),
	Vector2(1170, 690),
	Vector2(1230, 690),
	Vector2(1230, 750),
	Vector2(1230, 810),
	Vector2(1230, 870),
	Vector2(1230, 930),
	Vector2(1230, 990),
	//Vector2(1230, 1050),
};



inline int Clamp(int value, int min, int max)
{
	return std::min(max, std::max(min, value));
}

Projectile::Projectile()
{
}

Projectile::Projectile(float x, float y, int damage) 
	: x(x), y(y), damage(damage)
{
}

Tower::Tower()
{
}

Tower::Tower(int cost, int damage, int range, int cooldown, TowerType type) 
	: cost(cost), damage(damage), range(range), cooldown(cooldown), type(type)
{
}

Monster::Monster()
{
}

Monster::Monster(int value, int health, int damage, int speed, MonsterType type)
	: value(value), health(health), damage(damage), speed(speed), pathIndex(0), position{ 0,0 }, type(type)
{
}

void Game::SpawnMonster(MonsterType monsterType, int number)
{
	monsters.push_back(monsterTemplates[(std::size_t)monsterType]);
	Monster& monster = monsters.back();

	monster.position.x = 875;
	monster.position.y = 0;
	monster.value *= difficulty;
	monster.speed *= difficulty;
	monster.health *= difficulty;
	monster.damage *= difficulty;
}

bool Game::InRange(const Vector2& position, const Vector2& target, float range)
{
	const float diffX = target.x - position.x;
	const float diffY = target.y - position.y;
	const float dist = sqrtf(diffX * diffX + diffY * diffY);
	return range >= dist;
}

void Game::FirstWave()
{
	if (waveCounter == 50 || waveCounter == 150 || waveCounter == 300)
	{
		SpawnMonster(MonsterType::SLOW, 3);
	}
	if (waveCounter == 100 || waveCounter == 250 || waveCounter == 350
		|| waveCounter == 450 || waveCounter == 500)
	{
		SpawnMonster(MonsterType::REGULAR, 3);
	}
	if (waveCounter == 200 || waveCounter == 400 || waveCounter == 550)
	{
		SpawnMonster(MonsterType::FAST, 3);
	}
	if (waveCounter == 1000)
	{
		waveCounter = 0;
		difficulty *= 1.1f;
		currentWave = 2;
	}

	++waveCounter;
}

void Game::RegularWave()
{
	if (waveCounter == 50 || waveCounter == 150 || waveCounter == 300
		|| waveCounter == 175)
	{
		SpawnMonster(MonsterType::SLOW, 3);
	}
	if (waveCounter == 100 || waveCounter == 250 || waveCounter == 350
		|| waveCounter == 475)
	{
		SpawnMonster(MonsterType::REGULAR, 3);
	}
	if (waveCounter == 200 || waveCounter == 400 || waveCounter == 550
		|| waveCounter == 450 || waveCounter == 500 || waveCounter == 75)
	{
		SpawnMonster(MonsterType::FAST, 3);
	}
	if (waveCounter == 900)
	{
		waveCounter = 0;
		difficulty *= 1.2f;
		++currentWave;
	}

	++waveCounter;
}

Game::Game()
{
	Start();
}

int Game::GetScore() const
{
	return score;
}

void Game::Start()
{
	//reset the game state.
	health = 50;
	money = 100;
	waveCounter = 0;
	killCounter = 0;
	currentWave = 1;
	difficulty = 1;
	score = 0;
	collision = { 0 };
	monsters.clear();
	towers.clear();

	//towers cannot be placed along the path

	for(std::size_t i = 0; i < pathCoords.size()-1;i++)
	{
		int xDiff = pathCoords[i + 1].x - pathCoords[i].x;
		int yDiff = pathCoords[i + 1].y - pathCoords[i].y;

		if (xDiff != 0)
		{
			int dir = xDiff > 0 ? 1 : -1;

			for (int j = pathCoords[i].x; j != pathCoords[i + 1].x; j += dir)
			{
				collision[pathCoords[i].y * WIDTH + j] = true;
			}

		}else if(yDiff != 0)
		{
			int dir = yDiff > 0 ? 1 : -1;

			for (int j = pathCoords[i].y; j != pathCoords[i + 1].y; j += dir)
			{
				collision[j * WIDTH + pathCoords[i].x] = true;
			}
		}
	}

	//set up game assets:
	towerTemplates[0] = Tower(30, 15, 200, 45, TowerType::slammer);
	towerTemplates[1] = Tower(50, 50, 125, 90, TowerType::swinger);
	towerTemplates[2] = Tower(40, 10, 300, 37, TowerType::thrower);

	monsterTemplates[0] = Monster(1, 60, 3, 3, MonsterType::SLOW);
	monsterTemplates[1] = Monster(2, 30, 2, 4, MonsterType::REGULAR);
	monsterTemplates[2] = Monster(3, 15, 1, 5, MonsterType::FAST);
}


bool Game::Update()
{
	//Spawn Monsters:
	if (currentWave <= 1)
	{
		FirstWave();
	}
	else
	{
		RegularWave();
	}

	//Spawn projectiles:
	for (auto& tower : towers)
	{
		if (tower.currentCooldown > 0)
		{
			--tower.currentCooldown;
		}

		if (tower.currentCooldown <= 0)
		{
			for (auto& monster : monsters)
			{
				if (InRange(tower.position, monster.position, tower.range))
				{
					tower.currentCooldown = tower.cooldown;
					monster.projectiles.emplace_back(tower.position.x, tower.position.y, tower.damage);
					auto& projectile = monster.projectiles.back();

					if (monster.speed > (projectile.speed - 3.5f))
					{
						projectile.speed = (monster.speed + 3.5f);
					}

					break;
				}
			}
		}
	}

	//Move monsters towards goal:
	for (std::size_t i = 0; i < monsters.size(); ++i)
	{
		Monster& monster = monsters[i];

		//Move monster towards path index:
		const int xDiff = pathWaypoints[monster.pathIndex].x - monster.position.x;
		const int yDiff = pathWaypoints[monster.pathIndex].y - monster.position.y;

		const int xMov = Clamp(xDiff, -1, 1) * std::min(monster.speed, abs(xDiff));
		const int yMov = Clamp(yDiff, -1, 1) * std::min(monster.speed, abs(yDiff));

		monster.position.x += xMov;
		monster.position.y += yMov;

		if (monster.position.x == pathWaypoints[monster.pathIndex].x && monster.position.y == pathWaypoints[monster.pathIndex].y)
		{
			monster.pathIndex++;

			if (monster.pathIndex == pathCoords.size()) // reach end of path
			{
				health -= monster.damage;
				monsters.erase(monsters.begin() + i);
				--i;
				continue;
			}
		}

		//Move projectiles towards monster:

		for (std::size_t j = 0; j < monster.projectiles.size(); ++j)
		{
			auto& projectile = monster.projectiles[j];

			//using pythagoras to find the distance
			const float xDiff = (float)monster.position.x - projectile.x;
			const float yDiff = (float)monster.position.y - projectile.y;
			const float dist = sqrtf(xDiff * xDiff + yDiff * yDiff);

			//hit & deal damage 
			if (dist <= projectileHitRadius)
			{
				monster.health -= projectile.damage;

				if (monster.health <= 0)
				{
					money += monster.value;
					monsters.erase(monsters.begin() + i);
					++killCounter;
					--i;
					break;
				}


				monster.projectiles[j] = monster.projectiles.back();
				monster.projectiles.pop_back();
				--j;
				continue;
			}

			//work out the normal of the direction to the monster
			const float xNorm = xDiff / dist;
			const float yNorm = yDiff / dist;

			//move 
			projectile.x += xNorm * projectile.speed;
			projectile.y += yNorm * projectile.speed;

		}

	}

	score = currentWave * 10 + killCounter;

	//if run out of health, the game is over:
	if (health <= 0)
	{
		return false;
	}

	return true;
}

void Game::Render()
{
	std::string buffer;
	buffer.resize(WIDTH * HEIGHT + HEIGHT, ' ');

	for (std::size_t i = 0; i < HEIGHT; i++)
	{
		buffer[(i * (WIDTH+1)) + WIDTH] = '\n';
	}

	for (const auto& p : pathTiles)
	{
		int x = (p.x + 30) / 60;
		int y = (p.y + 30) / 60;
		buffer[(y) * (WIDTH + 1) + (x)] = '#';
	}

	for (const auto& tower : towers)
	{
		int x = (tower.position.x / 60);
		int y = (tower.position.y / 60);
		char apperance = L"SWT"[(int)tower.type];

		buffer[(y)*(WIDTH+1) + (x)] = apperance;
		buffer[(y)*(WIDTH+1) + (x+1)] = apperance;
		buffer[(y+1)*(WIDTH+1) + (x)] = apperance;
		buffer[(y+1)*(WIDTH+1) + (x+1)] = apperance;
	}


	for (const auto& monster : monsters)
	{
		int x = (monster.position.x + 30 ) / 60;
		int y = (monster.position.y + 30) / 60;


		for (const auto& projectile : monster.projectiles)
		{
			int px = (projectile.x / 60);
			int py = (projectile.y / 60);
			buffer[(py) * (WIDTH + 1) + (px)] = '.';
		}

		buffer[(y)*(WIDTH+1) + (x)] = L"smf"[(int)monster.type];
	}

	system("cls");
	std::cout << buffer << "\n";
	std::cout << "Money = " << money << "\n";
	std::cout << "Health = " << health << "\n";
	std::cout << "Wave = " << currentWave << "\n";
	std::cout << "Score = " << score << "\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(16));
}

bool Game::IsSpaceForTower(int x, int y)
{
	//Boundary checking
	if ((x + 1) >= WIDTH || (y + 1) >= HEIGHT)
		return false;
	if ((x) < 0 || y < 0)
		return false;

	//check mask
	if (collision[y * WIDTH + x] ||
		collision[y * WIDTH + (x + 1)] ||
		collision[(y + 1) * WIDTH + x] ||
		collision[(y + 1) * WIDTH + (x + 1)])
	{
		return false;
	}

	return true;
}

bool Game::IsMoneyForTower(TowerType towerType)
{
	return money >= towerTemplates[(int)towerType].cost;
}


bool Game::IsTowerInRangeOfPath(int x, int y, TowerType towerType)
{
	const float range = towerTemplates[(std::size_t)towerType].range;
	const Vector2 position(x * 60, y * 60);
	return std::any_of(pathTiles.begin(), pathTiles.end(),
		[this, &position, range](const Vector2& p) -> bool
		{
			return InRange(position,p , range);
		}
	);
}

bool Game::PlaceTower(int x, int y, TowerType towerType)
{
	if (IsSpaceForTower(x, y) && IsMoneyForTower(towerType))
	{
		money -= towerTemplates[(std::size_t)towerType].cost;

		//update mask
		collision[y * WIDTH + x] = true;
		collision[y * WIDTH + (x + 1)] = true;
		collision[(y + 1) * WIDTH + x] = true;
		collision[(y + 1) * WIDTH + (x + 1)] = true;

		//add tower to vector
		towers.push_back(towerTemplates[(std::size_t)towerType]);
		Tower& tower = towers.back();
		tower.position.x = x * 60;
		tower.position.y = y * 60;
		return true;
	}

	return false;
}
