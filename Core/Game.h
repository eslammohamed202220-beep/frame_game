#pragma once
#include "../CMUgraphicsLib/CMUgraphics.h"
#include "../UI/Toolbar.h"
#include "../UI/BudgetBar.h"
#include "../Entities/Animal.h"
#include <ctime>

class Game
{
private:
	window* pWind;	//Pointer to the CMU graphics window
	Toolbar* gameToolbar;
	Budgetbar* gameBudgetbar;
	int timer;
	int level;
	time_t lasttime = time(0);;
	time_t currenttime;
	bool wolf_Show = false;
	bool egg_show = false;
	bool milk_show = false;

public:
	int budget = 2000;
	Animal* animalsList[100];
	Chick* chickList[100];
	Cow* cowList[100];
	int animalCount = 0;
	int chickCount = 0;
	int cowCount = 0;
	int randNum;
	struct Item {
		point pos;
		string type;
	};
	Item* ItemList[100];
	int ItemCount = 0;
	Game();
	~Game();

	clicktype getMouseClick(int& x, int& y) const; //Get coordinate where user clicks and returns click type (left/right)
	string getSrting() const;	 //Returns a string entered by the user

	window* CreateWind(int, int, int, int) const; //creates the game window
	void createToolbar();
	void createBudgetbar();
	void clearBudget() const;
	void printBudget(string msg) const;
	void clearStatusBar() const;	//Clears the status bar
	void writeStatus() const;			//Draw in stats par
	void updateTimer();
	void Wolfadd();
	void eggadd();
	void milkadd();
	void printMessage(string msg) const;	//Print a message on Status bar

	// Drawing helpers for the playing field
	void drawBackground() const;
	void drawWarehouse() const;
	void drawFieldBoundaries() const;
	void redrawScene() const;
	void drawFoodArea()const;
	void go();

	window* getWind() const;		//returns a pointer to the graphics window
};