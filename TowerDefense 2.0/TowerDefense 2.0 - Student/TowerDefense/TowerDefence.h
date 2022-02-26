#pragma once
#include "ITowerDefence.h"
#include <SFML/Graphics.hpp>
#include "Constants.h"

class Timer;
class GameState;
class GameMenuController;
class TowerController;
class MonsterController;
class GameBoard;
class TowerAndMonsterController;
class AIController;

class TowerDefence : public ITowerDefence
{

public:
    void Run();
   
    // Inherited via ITowerDefence (for AI)
    virtual int GetScore() const override;
    virtual bool IsSpaceForTower(int x, int y) override;
    virtual bool IsMoneyForTower(TowerType towerType) override;
    virtual bool IsTowerInRangeOfPath(int x, int y, TowerType towerType) override;
    virtual bool PlaceTower(int x, int y, TowerType towerType) override;

private:

    void UpdateScore();
    void Clean();
    void Reset();

    sf::RenderWindow* window;
    Timer* clk;
    GameState* gameState;
    GameMenuController* gameMenuController;
    TowerController* towerController;
    MonsterController* monsterController;
    TowerAndMonsterController* attackController;
    GameBoard* gameBoard;

    AIController* aiController;
};

