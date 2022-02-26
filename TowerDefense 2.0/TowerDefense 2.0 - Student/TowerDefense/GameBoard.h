/*
 * GameController.h
 *
 *  Created on: Nov 6, 2018
 *      Author: Connor
 */
#pragma once
#include <vector>
#include <array>
#include "Constants.h"

class GameState;
class TowerController;

// Game Board generates the map, and handles interactions
// for any entities sitting on the map
class GameBoard
{
private:
	GameState* gameState;
	TowerController* towerController;
	int width;

public:
	GameBoard(GameState* gameState, TowerController* towerController,
	          int widthInPixels, const std::vector<sf::Vector2f>& path, const std::array<sf::Vector2i, 71>& pathTiles);
	~GameBoard();

	bool validatePos(int mouseX, int mouseY, int range);
	void process(sf::Event event, sf::Vector2i mousePos);
	void render(sf::RenderWindow* window); // Render board
	void renderLabels(sf::RenderWindow* window);
	void renderRange(int mouseX, int mouseY, int range, sf::RenderWindow* window);
	void renderShadow(int mouseX, int mouseY, int range, sf::RenderWindow* window);

	bool gridSpaceAvailable(int gridX, int gridY);
	bool towerIsPurchasable(TowerType type);
	bool addTower(TowerType type, int gridX, int gridY);
	bool inRangeOfPath(int x, int y, TowerType type) const;
	int getNumTilesInRange(int x, int y, TowerType type) const;

	int gridStatus[WIDTH][HEIGHT] = {0};

	const std::vector<sf::Vector2f>& path;
	const std::array<sf::Vector2i, 71>& pathTiles;

	sf::Texture* _helpTexture;
	sf::Texture* _menuTexture;
	sf::RectangleShape grassTile;
	sf::RectangleShape groundTile;
	sf::CircleShape hoverOutline;
	sf::RectangleShape shadowTile;
	sf::RectangleShape* _tamsCounter;
	sf::RectangleShape* _livesCounter;
	sf::RectangleShape* _wavesCounter;
	sf::RectangleShape helpScreen;
	sf::Text tamText, waveText, waveWord, healthText, text, scoreText, debugText;
	sf::Font font;
	sf::Event event;
};
