#pragma once
#include "Constants.h"

class ITowerDefence
{
public:

	virtual int GetScore() const = 0;
	virtual bool IsSpaceForTower(int x, int y) = 0;
	virtual bool IsMoneyForTower(TowerType towerType) = 0;
	virtual bool IsTowerInRangeOfPath(int x, int y, TowerType towerType) = 0;
	virtual bool PlaceTower(int x, int y, TowerType towerType) = 0;
};