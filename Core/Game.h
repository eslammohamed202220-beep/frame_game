#pragma once
#include "../CMUgraphicsLib/CMUgraphics.h"
#include "../UI/Toolbar.h"
#include "../UI/BudgetBar.h"
#include "../Entities/Animal.h"
#include <ctime>

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

	// Display flags
	bool wolf_Show = false;
	bool egg_show = false;
	bool milk_show = false;

public:
	// Public game data
	int budget = 2000;

	Animal* animalsList[100];
	Chick* chickList[100];
	Cow* cowList[100];

	int animalCount = 0;
	int chickCount = 0;
	int cowCount = 0;

	int randNum;

	struct Item
	{
		point pos;
		string type;
	};

	Item* ItemList[100];
	int ItemCount = 0;

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

	// Drawing helpers
	void drawBackground() const;
	void drawWarehouse() const;
	void drawFieldBoundaries() const;
	void redrawScene() const;
	void drawFoodArea() const;

	// Game logic
	void updateTimer();
	void Wolfadd();
	void eggadd();
	void milkadd();
	void collectItems(int x, int y);

	// Main loop
	void go();

	// Window access
	window* getWind() const;
};