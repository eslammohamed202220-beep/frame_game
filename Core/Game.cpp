#include "Game.h"
#include "../Config/GameConfig.h"
#include "../CMUgraphicsLib/error.h"
#include <random>
#include"../UI/BudgetBar.h"
#include <cstdlib>
#include <ctime>

namespace {

	// CMUgraphics loads JPEGs in the image ctor and throws error::FILE_NOT_FOUND on failure.
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
	animalCount = 0;
	budget = 1000;
	wolf_Show = false;

	for (int i = 0; i < 100; i++)
	{
		animalsList[i] = nullptr;
		chickList[i] = nullptr;
		cowList[i] = nullptr;
		ItemList[i] = nullptr;
	}
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

	printBudget("BUDGET = $" + to_string(budget));
	writeStatus();

	pWind->UpdateBuffer();
}

Game::~Game()
{
	// Clean up all allocated memory to prevent leaks
	for (int i = 0; i < animalCount; i++)
	{
		if (animalsList[i]) delete animalsList[i];
	}
	for (int i = 0; i < ItemCount; i++) {
		if (ItemList[i]) delete ItemList[i];
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

	string timelevelmsg = "  TIMER = " + to_string(timer) + " | LEVEL = " + to_string(level) + " | Current animals number = " + to_string(animalCount);

	pWind->DrawString(10, y_pos, timelevelmsg);
}
void Game::warehouseContent() const {
	pWind->SetPen(config.penColor, 50);
	pWind->SetFont(20, BOLD, BY_NAME, "Arial");

	int y_pos = config.windHeight - 325;

	string timelevelmsg = "  Egg: " + to_string(eggInWareHouse) + " | Milk = " + to_string(milkInWareHouse);

	pWind->DrawString(950, y_pos, timelevelmsg);
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
	if (animalCount >= 100)
		return;

	randNum = rand() % 100;
	if (timer > 0 && randNum < 3 * level && !wolf_Show)
	{
		point p;

		std::random_device rd1;
		std::mt19937 gen1(rd1());
		std::uniform_int_distribution<int> dist1(10, config.windWidth - 100);
		p.x = dist1(gen1);

		std::random_device rd2;
		std::mt19937 gen2(rd2());
		std::uniform_int_distribution<int> dist2(2 * config.toolBarHeight + 50, config.windHeight - config.statusBarHeight - 150);
		p.y = dist2(gen2);

		animalsList[animalCount] = new Wolf(this, p, 70, 70, "images/wolff.jpg");
		animalCount++;
		wolf_Show = true;
	}
}

void Game::eggadd()
{
	if (timer > 0 && timer % 10 == 0 && !egg_show)
	{
		for (int i = 0; i < chickCount; i++)
		{
			if (chickList[i] != nullptr)
				chickList[i]->egg();
		}
		egg_show = true;
	}
}

void Game::milkadd()
{
	if (timer > 0 && timer % 15 == 0 && !milk_show)
	{
		for (int i = 0; i < cowCount; i++)
		{
			if (cowList[i] != nullptr)
				cowList[i]->milk();
		}
		milk_show = true;
	}
}
void Game::collectItems(int x,int y) {
	for (int i = 0; i < ItemCount; i++) {
		if (ItemList[i] != nullptr) {
			if ((x >= ItemList[i]->pos.x && x <= ItemList[i]->pos.x + 30) &&
				(y >= ItemList[i]->pos.y && y <= ItemList[i]->pos.y + 30)) {
				if (ItemList[i]->type == "egg") {
					eggInWareHouse++;
					delete ItemList[i];
					ItemList[i] = nullptr;
					break;
				}
				else if (ItemList[i]->type == "milk") {
					milkInWareHouse++;
					delete ItemList[i];
					ItemList[i] = nullptr;
					break;
				}
			}
		}
	}
}

// ==========================
// Drawing
// ==========================

void Game::drawBackground() const
{
	int playY = 2 * config.toolBarHeight;
	int playH = config.windHeight - config.statusBarHeight - playY;

	// Solid fallback if no JPEG can be loaded (wrong working directory, missing file, etc.)
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

	// CMUgraphics DrawRectangle defaults to FILLED; a filled rect would paint over the background image.
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
		"images\\Warehouse.jpg",
		"../images/Warehouse.jpg",
		"..\\images\\Warehouse.jpg",
	};

	for (const char* path : kWarehousePaths)
	{
		if (tryDrawJpeg(pWind, path, wx, wy, warehouseW, warehouseH))
			return;
	}
}

void Game::drawFoodArea() const
{
	int x = 20;
	int y = config.windHeight - config.statusBarHeight - 200;
	int width = 1000;
	int height = 150;

	pWind->SetPen(DARKGREEN, 2);
	pWind->SetBrush(LIGHTGREEN);
	pWind->DrawRectangle(x, y, x + width, y + height, FILLED);

	pWind->DrawRectangle(x, y, x + width, y + height, FRAME);

	pWind->SetPen(BLACK, 1);
	pWind->DrawString(x + 5, y - 18, "Food Area");
}

void Game::redrawScene() const
{
	drawBackground();
	drawWarehouse();
	drawFieldBoundaries();
	drawFoodArea();
	warehouseContent();

	for (int i = 0; i < animalCount; i++)
	{
		if (animalsList[i] != nullptr)
			animalsList[i]->draw();
	}
	for (int i = 0; i < ItemCount; i++)
	{
		if (ItemList[i] != nullptr)
		{
			if (ItemList[i]->type == "egg")
				pWind->DrawImage("images\\egg.jpg", ItemList[i]->pos.x, ItemList[i]->pos.y, 30, 30);
			else if (ItemList[i]->type == "milk")
				pWind->DrawImage("images\\milk.jpg", ItemList[i]->pos.x, ItemList[i]->pos.y, 30, 30);
		}
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
		updateTimer();

		for (int i = 0; i < animalCount; i++)
		{
			if (animalsList[i] != nullptr)
				animalsList[i]->moveStep();
		}

		Wolfadd();
		eggadd();
		milkadd();

		redrawScene();

		clicktype ct = pWind->GetMouseClick(x, y);
		if (ct != NO_CLICK)
		{
			collectItems(x, y);
			if (y >= 0 && y < config.toolBarHeight)
			{
				isExit = gameToolbar->handleClick(x, y);
			}
			else if (y >= config.toolBarHeight && y < 2 * config.toolBarHeight)
			{
				isExit = gameBudgetbar->handleClick(x, y);
			}
		}

		Sleep(60);

	} while (!isExit);
}