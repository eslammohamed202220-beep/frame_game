#pragma once
#include "../CMUgraphicsLib/CMUgraphics.h"
#include "../UI/Toolbar.h"
#include "../UI/BudgetBar.h"
#include "../Entities/Animal.h"
#include <ctime>
#include <vector>

class Game
{
private:
	// Window and UI
	window* pWind;	//Pointer to the CMU graphics window
	Toolbar* gameToolbar;
	Budgetbar* gameBudgetbar;

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
	// Water area state


public:
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
	struct GreenArea {
		int x, y;
		int counter;
		bool active;
		bool touched;
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
	void openWarehouseWindow() const;

	// Game logic
	void updateTimer();
	void Wolfadd();
	void checkAnimalGrassCollision();
	void eggadd();
	void milkadd();
	void collectItems(int x, int y);
	void restartGame();

	// Main loop
	void go();

	// Window access
	window* getWind() const;
};