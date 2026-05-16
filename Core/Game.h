#pragma once

// miniaudio: single-header audio library (already in project root)
// NOTE: MINIAUDIO_IMPLEMENTATION is defined only in Game.cpp
#include "../miniaudio.h"

#include "../CMUgraphicsLib/CMUgraphics.h"
#include "../UI/Toolbar.h"
#include "../UI/BudgetBar.h"
#include "../Entities/Animal.h"
#include <ctime>
#include <string>
#include <utility>
#include <vector>


class Game
{
private:
	// Window and UI
	window* pWind;	//Pointer to the CMU graphics window
	Toolbar* gameToolbar;
	Budgetbar* gameBudgetbar;

	// Background music (miniaudio)
	ma_engine audioEngine;
	ma_sound  bgMusic;
	bool      audioReady = false;

	// Game state
	int timer;
	int level;

	// Time handling
	time_t lasttime = time(0);
	time_t currenttime;
	time_t lastWolfSpawnTime = 0;

	// Display flags
	bool wolf_Show = false;
	bool egg_show = false;
	bool milk_show = false;
	bool isGameOver = false;
	bool gameStarted = false;
	string playerName;
	vector<pair<string, int>> leaderboardEntries;
	// Water area state
	
public:
	int numchick = 0;
	int numcow = 0;

	// Public game data
	int budget = 2000;

	vector<Animal*> animalsList;
	vector<Chick*> chickList;
	vector<Cow*> cowList;

	int eggInWareHouse = 0;
	int milkInWareHouse = 0;
	int totalFoodConsumed = 0;
	int randNum;
	// add green area
	struct GreenArea
	{
		int x;
		int y;
		int counter;
		bool active;
		int eatTimer;
	};
	vector<GreenArea*> greenAreaList;

	// add egg and milk
	struct Item
	{
		point pos;
		string type;
	};

	vector<Item*> ItemList;
	//for pause and resume 
	bool isPaused = false;
	// Constructors / Destructor
	Game();
	~Game();

	// Input
	clicktype getMouseClick(int& x, int& y) const;
	string getSrting() const;

	// Window and UI creation
	window* CreateWind(int, int, int, int) const;
	void createToolbar();
	void createBudgetbar();

	// Status / Budget
	void clearBudget() const;
	void printBudget(string msg) const;
	void clearStatusBar() const;
	void writeStatus() const;
	void printMessage(string msg) const;
	void warehouseContent() const;
	void animalCounter() const;

	// Drawing helpers
	void drawBackground() const;
	void drawWarehouse() const;
	void drawFieldBoundaries() const;
	void redrawScene() const;
	void drawFoodArea()const;
	void drawGreenArea();
	bool isWarehouseClicked(int x, int y) const;
	void openWarehouseWindow();
	void drawWarehouseUI(window& infoWin);

	// Game logic
	void updateTimer();
	void checkLevelUp();
	void gameOver();
	void promptUsername();
	void loadLeaderboard();
	void saveLeaderboard() const;
	void updatePlayerHighScore(int score);
	void Wolfadd();
	void checkAnimalGrassCollision();
	void eggadd();
	void milkadd();
	void collectItems(int x, int y);
	void restartGame();
	void sellegg();
	void sellmilk();
	void saveGame() const;
	void loadGame();
	// Game logic
	
	// Main loop
	void go();

	// Window access
	window* getWind() const;

	// Audio control
	void pauseMusic();
	void resumeMusic();
	void playSfx(const char* path);
};