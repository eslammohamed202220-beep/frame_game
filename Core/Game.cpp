#include "Game.h"
#include "../Config/GameConfig.h"
#include "../CMUgraphicsLib/error.h"
#include <random>
#include"../UI/BudgetBar.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;
namespace {

	bool tryDrawJpeg(window* pWind, const char* path, int x, int y, int w, int h)
	{
		try
		{
			image img(path);
			pWind->DrawImage(img, x, y, w, h);
			return true;
		}
		catch (error)
		{
			return false;
		}
	}

} // namespace

// ==========================
// Constructors / Destructor
// ==========================

Game::Game()
{
	// Initialization for logic
	srand(time(0));
	budget = 1000;
	wolf_Show = false;
	//1 - Create the main window
	pWind = CreateWind(config.windWidth, config.windHeight, config.wx, config.wy);

	// CRITICAL: Set buffering ONCE in constructor to avoid "BitBlt" Fatal Error
	pWind->SetBuffering(true);

	//2 - create and draw the toolbar
	createToolbar();
	createBudgetbar();

	//3 - create and draw the playing field (background, warehouse, boundary outline)
	drawBackground();
	drawWarehouse();
	drawFieldBoundaries();
	warehouseContent();
	animalCounter();
	//4- Create the Plane
	//TODO: Add code to create and draw the Plane

	//5- Create the Bullet
	//TODO: Add code to create and draw the Bullet

	//6- Create the enemies
	//TODO: Add code to create and draw enemies in random places

	//7- Create and clear the status bar
	clearStatusBar();

	//intilaization for level and timer ints
	level = 1;
	timer = 60 + (level - 1) * 30;
	lasttime = time(0);
	lastWolfSpawnTime = time(0);

	printBudget("BUDGET = $" + to_string(budget));
	writeStatus();

	pWind->UpdateBuffer();
}

Game::~Game()
{
	// Clean up all allocated memory to prevent leaks
	for (int i = 0; i < (int)animalsList.size(); i++)
	{
		if (animalsList[i]) delete animalsList[i];
	}
	for (int i = 0; i < (int)ItemList.size(); i++) {
		if (ItemList[i]) delete ItemList[i];
	}
	for (int i = 0; i < (int)greenAreaList.size(); i++) {
		if (greenAreaList[i]) delete greenAreaList[i];
	}
	delete gameToolbar;
	delete gameBudgetbar;
	delete pWind;
}

// ==========================
// Input
// ==========================

clicktype Game::getMouseClick(int& x, int& y) const
{
	return pWind->WaitMouseClick(x, y);	//Wait for mouse click
}

string Game::getSrting() const
{
	string Label;
	char Key;
	keytype ktype;
	pWind->FlushKeyQueue();
	while (1)
	{
		ktype = pWind->WaitKeyPress(Key);
		if (ktype == ESCAPE)	//ESCAPE key is pressed
			return "";	//returns nothing as user has cancelled label
		if (Key == 13)	//ENTER key is pressed
			return Label;
		if (Key == 8)	//BackSpace is pressed
			if (Label.size() > 0)
				Label.resize(Label.size() - 1);
			else
				Key = '\0';
		else
			Label += Key;
		printMessage(Label);
	}
}

// ==========================
// Window / UI Creation
// ==========================

window* Game::CreateWind(int w, int h, int x, int y) const
{
	window* pW = new window(w, h, x, y);
	pW->SetBrush(config.bkGrndColor);
	pW->SetPen(config.bkGrndColor, 1);
	pW->DrawRectangle(0, 0, w, h);
	return pW;
}

void Game::createToolbar()
{
	point toolbarUpperleft;
	toolbarUpperleft.x = 0;
	toolbarUpperleft.y = 0;

	gameToolbar = new Toolbar(this, toolbarUpperleft, 0, config.toolBarHeight);
	gameToolbar->draw();
}

void Game::createBudgetbar()
{
	point budgetbarUpperleft;
	budgetbarUpperleft.x = 0;
	budgetbarUpperleft.y = config.toolBarHeight;

	gameBudgetbar = new Budgetbar(this, budgetbarUpperleft, 0, config.toolBarHeight);
	gameBudgetbar->draw();
}

window* Game::getWind() const
{
	return pWind;
}

// ==========================
// Status Display
// ==========================

void Game::clearBudget() const
{
	//Clear Budget area by drawing a filled rectangle
	pWind->SetPen(config.bkGrndColor, 1);
	pWind->SetBrush(config.bkGrndColor);
	pWind->DrawRectangle(config.windWidth - 340, config.toolBarHeight, config.windWidth, 2 * config.toolBarHeight);
}

void Game::printBudget(string msg) const
{
	clearBudget();	//First clear the budget area

	pWind->SetPen(config.penColor, 50);
	pWind->SetFont(24, BOLD, BY_NAME, "Arial");
	pWind->DrawString(config.windWidth - 250, config.toolBarHeight + 10, msg);
}

void Game::clearStatusBar() const
{
	//Clear Status bar by drawing a filled rectangle
	pWind->SetPen(config.statusBarColor, 1);
	pWind->SetBrush(config.statusBarColor);
	pWind->DrawRectangle(0, config.windHeight - config.statusBarHeight, config.windWidth, config.windHeight);
}

void Game::printMessage(string msg) const
{
	clearStatusBar();	//First clear the status bar

	pWind->SetPen(config.penColor, 50);
	pWind->SetFont(24, BOLD, BY_NAME, "Arial");
	pWind->DrawString(10, config.windHeight - (int)(0.85 * config.statusBarHeight), msg);
}

void Game::writeStatus() const
{
	clearStatusBar();

	pWind->SetPen(config.penColor, 50);
	pWind->SetFont(20, BOLD, BY_NAME, "Arial");

	int y_pos = config.windHeight - config.statusBarHeight + 10;

	string timelevelmsg = "  TIMER = " + to_string(timer) + " | LEVEL = " + to_string(level) + " | Current animals number = " + to_string(animalsList.size());

	pWind->DrawString(10, y_pos, timelevelmsg);
}
void Game::warehouseContent() const {
	pWind->SetPen(config.penColor, 50);
	pWind->SetFont(16, BOLD, BY_NAME, "Arial");

	int y_pos_Structure = config.windHeight -390;
	string wareHouseStructure = "  Item:  |  Count |  price " ;
	pWind->DrawString(985, (y_pos_Structure), wareHouseStructure);

	int y_pos_egg = config.windHeight -370;
	string egg = "  Egg:   |      " + to_string(eggInWareHouse) + "     |   10 ";
	pWind->DrawString(985, y_pos_egg, egg);

	int y_pos_milk = config.windHeight -350;
	string milk = "  Milk:  |      " + to_string(milkInWareHouse) + "      |   20 ";
	pWind->DrawString(985, y_pos_milk, milk);
}
void Game::animalCounter() const {
	pWind->SetPen(config.penColor, 50);
	pWind->SetFont(15, BOLD, BY_NAME, "Arial");
	int y_pos = config.windHeight-490 ;
	string chickDisplay = "  Chicken: " + to_string(chickList.size());
	string cowDisplay = "  Cow: " + to_string(cowList.size());
	string consumedDisplay = "  Fed: " + to_string(totalFoodConsumed);

	pWind->DrawString(5, y_pos, chickDisplay);
	pWind->DrawString(90, y_pos, cowDisplay);
	pWind->DrawString(160, y_pos, consumedDisplay);
}
// ==========================
// Game Logic
// ==========================

void Game::updateTimer()
{
	time_t now = time(0);

	if (timer > 0 && now > lasttime)
	{
		timer -= (int)(now - lasttime);
		if (timer < 0)
			timer = 0;
	
		lasttime = now;
		wolf_Show = false;
		egg_show = false;
		milk_show = false;
	}
}

void Game::Wolfadd()
{
	time_t now = time(0);
	int spawnCooldown = max(6, 12 - level);
	randNum = rand() % 100;
	int activeWolves = 0;
	for (int i = 0; i < (int)animalsList.size(); i++)
	{
		if (dynamic_cast<Wolf*>(animalsList[i]) != nullptr)
			activeWolves++;
	}

	if (timer > 0 &&
		(now - lastWolfSpawnTime) >= spawnCooldown &&
		randNum < (2 + level) &&
		activeWolves < 2 &&
		!wolf_Show)
	{
		point p;

		p.x = rand() % 1130;
		p.y = 370 + rand() % 100;

		animalsList.push_back(new Wolf(this, p, 70, 70, "images/wolff.jpg"));
		lastWolfSpawnTime = now;
		wolf_Show = true;
	}
}

void Game::eggadd()
{
	if (timer > 0 && timer % 10 == 0 && !egg_show)
	{
		for (int i = 0; i < (int)chickList.size(); i++)
		{
			chickList[i]->egg();
		}
		egg_show = true;
	}
}

void Game::milkadd()
{
	if (timer > 0 && timer % 15 == 0 && !milk_show)
	{
		for (int i = 0; i < (int)cowList.size(); i++)
		{
			cowList[i]->milk();
		}
		milk_show = true;
	}
}
void Game::collectItems(int x, int y) {
	for (int i = 0; i < (int)ItemList.size(); i++) {
		if ((x >= ItemList[i]->pos.x && x <= ItemList[i]->pos.x + 30) &&
			(y >= ItemList[i]->pos.y && y <= ItemList[i]->pos.y + 30)) {
			if (ItemList[i]->type == "egg") {
				eggInWareHouse++;
				delete ItemList[i];
				ItemList.erase(ItemList.begin() + i);
				break;
			}
			else if (ItemList[i]->type == "milk") {
				milkInWareHouse++;
				delete ItemList[i];
				ItemList.erase(ItemList.begin() + i);
				break;
			}
		}
	}
}
void Game::restartGame()
{

	for (int i = 0; i < (int)animalsList.size(); i++)
	{
		delete animalsList[i];
	}
	animalsList.clear();
	chickList.clear();
	cowList.clear();
	budget = 1000;
	totalFoodConsumed = 0;
	timer = 60 + (level - 1) * 30;
	lasttime = time(0);
	lastWolfSpawnTime = time(0);

	for (int i = 0; i < (int)greenAreaList.size(); i++)
	{
		delete greenAreaList[i];
	}
	greenAreaList.clear();

	
	for (int i = 0; i < (int)ItemList.size(); i++)
	{
		delete ItemList[i];
	}
	ItemList.clear();

	redrawScene();
}


void Game::drawBackground() const
{
	int playY = 2 * config.toolBarHeight;
	int playH = config.windHeight - config.statusBarHeight - playY;

	pWind->SetPen(LAVENDER);
	pWind->SetBrush(LAVENDER);
	pWind->DrawRectangle(0, playY, config.windWidth, config.windHeight - config.statusBarHeight);

	static const char* kBgPaths[] = {
		"images/Background.jpg",
		"images\\Background.jpg",
		"../images/Background.jpg",
		"..\\images\\Background.jpg",
	};

	for (const char* path : kBgPaths)
	{
		if (tryDrawJpeg(pWind, path, 0, playY, config.windWidth, playH))
			return;
	}
}

void Game::drawFieldBoundaries() const
{
	int topY = 2 * config.toolBarHeight;
	int bottomY = config.windHeight - config.statusBarHeight;

	pWind->SetPen(DARKGREEN, 4);
	pWind->DrawRectangle(0, topY, config.windWidth, bottomY, FRAME);
}

void Game::drawWarehouse() const
{
	int playY = 2 * config.toolBarHeight;
	const int warehouseW = 220;
	const int warehouseH = 180;
	int wx = config.windWidth - warehouseW - 30;
	int wy = playY + 20;

	static const char* kWarehousePaths[] = {
		"images/Warehouse.jpg",
		"../images/Warehouse.jpg"
	};

	for (const char* path : kWarehousePaths)
	{
		if (tryDrawJpeg(pWind, path, wx, wy, warehouseW, warehouseH))
			return;
	}
}

void Game::drawFoodArea()const
{
	int x = 0;
	int y = config.windHeight - config.statusBarHeight - 180;
	int width = 1200;
	int height = 170;

	pWind->SetPen(BLACK, 2);
	pWind->DrawRectangle(x, y, x + width, y + height, FRAME);

	pWind->SetPen(BLACK, 1);
	pWind->DrawString(x + 5, y - 18, "Food Area");
}
void Game::drawGreenArea()
{
	int foodX = 0;
	int foodY = config.windHeight - config.statusBarHeight - 180;
	int foodW = 1200;
	int foodH = 170;
	int areaW = 50;
	int areaH = 40;

	GreenArea* newArea = new GreenArea();
	newArea->x = foodX + rand() % (foodW - areaW);
	newArea->y = foodY + rand() % (foodH - areaH);
	newArea->counter = 29;
	newArea->active = true;
	newArea->touched = false;
	greenAreaList.push_back(newArea);
}
void Game::checkAnimalGrassCollision()
{
	for (int i = 0; i < (int)greenAreaList.size(); i++)
	{
		if (!greenAreaList[i]->active)
			continue;

		bool touchingNow = false;
		Animal* eater = nullptr;

		int grassW = 50;
		int grassH = 40;

		
		for (int j = 0; j < (int)chickList.size(); j++)
		{
			point animalPos = chickList[j]->getPosition();

			int animalX = animalPos.x;
			int animalY = animalPos.y;

			int animalW = 60;
			int animalH = 60;

			if (animalX < greenAreaList[i]->x + grassW &&
				animalX + animalW > greenAreaList[i]->x &&
				animalY < greenAreaList[i]->y + grassH &&
				animalY + animalH > greenAreaList[i]->y)
			{
				touchingNow = true;
				eater = chickList[j];
				break;
			}
		}

		for (int j = 0; j < (int)cowList.size() && !touchingNow; j++)
		{
			point animalPos = cowList[j]->getPosition();

			int animalX = animalPos.x;
			int animalY = animalPos.y;

			int animalW = 60;
			int animalH = 60;

			if (animalX < greenAreaList[i]->x + grassW &&
				animalX + animalW > greenAreaList[i]->x &&
				animalY < greenAreaList[i]->y + grassH &&
				animalY + animalH > greenAreaList[i]->y)
			{
				touchingNow = true;
				eater = cowList[j];
				break;
			}
		}

		if (touchingNow)
		{
			if (!greenAreaList[i]->touched)
			{
				greenAreaList[i]->counter--;
				totalFoodConsumed++;
				if (eater != nullptr) eater->hunger = 0;

				if (greenAreaList[i]->counter <= 0)
				{
					greenAreaList[i]->active = false;
				}
				greenAreaList[i]->touched = true;
			}
		}
		else
		{
			greenAreaList[i]->touched = false;
		}
	}
}
bool Game::isWarehouseClicked(int x, int y) const
{
	int playY = 2 * config.toolBarHeight;
	const int warehouseW = 220;
	const int warehouseH = 180;
	int wx = config.windWidth - warehouseW - 30;
	int wy = playY + 20;
	return x >= wx && x <= wx + warehouseW && y >= wy && y <= wy + warehouseH;
}

void Game::openWarehouseWindow() const
{
	window infoWin(420, 240, config.wx + 80, config.wy + 80);
	infoWin.SetPen(BLACK, 2);
	infoWin.SetBrush(WHITE);
	infoWin.DrawRectangle(0, 0, 420, 240, FILLED);
	infoWin.SetFont(22, BOLD, BY_NAME, "Arial");
	infoWin.DrawString(95, 20, "Warehouse Inventory");
	infoWin.SetFont(18, BOLD, BY_NAME, "Arial");
	infoWin.DrawString(40, 80, "Eggs  : " + to_string(eggInWareHouse));
	infoWin.DrawString(40, 120, "Milk  : " + to_string(milkInWareHouse));
	infoWin.DrawString(40, 160, "Fed Animals: " + to_string(totalFoodConsumed));
	infoWin.SetFont(14, BOLD, BY_NAME, "Arial");
	infoWin.DrawString(40, 205, "Click anywhere in this window to close");
	int x, y;
	infoWin.WaitMouseClick(x, y);
}
void Game::redrawScene() const
{
	drawBackground();
	drawWarehouse();
	drawFieldBoundaries();
	drawFoodArea();
	warehouseContent();
	animalCounter();
	for (int i = 0; i < (int)animalsList.size(); i++)
	{
		animalsList[i]->draw();
	}
	for (int i = 0; i < (int)ItemList.size(); i++)
	{
		if (ItemList[i]->type == "egg")
			pWind->DrawImage("images\\egg.jpg", ItemList[i]->pos.x, ItemList[i]->pos.y, 30, 30);
		else if (ItemList[i]->type == "milk")
			pWind->DrawImage("images\\milk.jpg", ItemList[i]->pos.x, ItemList[i]->pos.y, 30, 30);
	}
	for (int i = 0; i < (int)greenAreaList.size(); i++)
	{
		if (!greenAreaList[i]->active) continue;

		pWind->SetPen(DARKGREEN, 2);
		pWind->SetBrush(GREEN);
		pWind->DrawRectangle(greenAreaList[i]->x, greenAreaList[i]->y,
			greenAreaList[i]->x + 50, greenAreaList[i]->y + 40, FILLED);

		pWind->SetPen(BLACK, 1);
		pWind->SetFont(16, BOLD, BY_NAME, "Arial");
		pWind->DrawString(greenAreaList[i]->x + 15, greenAreaList[i]->y - 18,
			to_string(greenAreaList[i]->counter));
	}

	gameToolbar->draw();
	gameBudgetbar->draw();
	printBudget("BUDGET = $" + to_string(budget));
	writeStatus();
	pWind->UpdateBuffer();
}

// ==========================
// Main Loop
// ==========================

void Game::go()
{
	printMessage("Entered go()");
	int x, y;
	bool isExit = false;

	pWind->ChangeTitle("- - - - - - - - - - Farm Frenzy (CIE101-project) - - - - - - - - - -");
	do
	{
		if (!isPaused)
		{
			updateTimer();

			for (int i = 0; i < (int)animalsList.size(); i++)
			{
				animalsList[i]->moveStep();
			}

			Wolfadd();
			eggadd();
			milkadd();
			checkAnimalGrassCollision();
		}
		else
		{
			lasttime = time(0);
		}

		redrawScene();

		clicktype ct = pWind->GetMouseClick(x, y);
		if (ct != NO_CLICK)
		{
			if (isPaused)
			{
				if (y >= 0 && y < config.toolBarHeight)
				{
					isExit = gameToolbar->handleClick(x, y);
				}
			}
			else
			{
				collectItems(x, y);
				if (isWarehouseClicked(x, y))
				{
					openWarehouseWindow();
				}

				if (y >= 0 && y < config.toolBarHeight)
				{
					isExit = gameToolbar->handleClick(x, y);
				}
				else if (y >= config.toolBarHeight && y < 2 * config.toolBarHeight)
				{
					isExit = gameBudgetbar->handleClick(x, y);
				}
			}
		}

		Sleep(60);

	} while (!isExit);
}