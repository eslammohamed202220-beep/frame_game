#include "Budgetbar.h"
#include "../Config/GameConfig.h"
#include "../Core/Game.h"
#include "../CMUgraphicsLib/error.h"
#include <iostream>
using namespace std;


BudgetbarIcon::BudgetbarIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Drawable(r_pGame, r_point, r_width, r_height)
{
	image_path = img_path;
}

void BudgetbarIcon::draw() const
{
	window* pWind = pGame->getWind();
	try
	{
		image img(image_path);
		pWind->DrawImage(img, RefPoint.x, RefPoint.y, width, height);
	}
	catch (error)
	{
		pWind->SetPen(BLUE, 2);
		pWind->SetBrush(LIGHTGRAY);
		pWind->DrawRectangle(RefPoint.x, RefPoint.y, RefPoint.x + width, RefPoint.y + height, FRAME);
	}
}

ChickIcon::ChickIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : BudgetbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
}
CowIcon::CowIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path)
	: BudgetbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
}
void ChickIcon::onClick()
{
	//TO DO: add code for cleanup and game exit here
	/*
	//draw image of this object in the field
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	*/
	
	//Chick* new_chick = new Chick(pGame, RefPoint, 30, 30, "images\\Chick.png");
	cout << "Icon Chick Clicked" << endl;
	if (pGame->budget > 100) {
		pGame->budget = pGame->budget - 100;
		pGame->clearBudget();
		string budget_string = "BUDGET = $" + to_string(pGame->budget);
		pGame->printBudget(budget_string);

		point p;
	    p.x = range_min_x + rand() % (range_max_x - range_min_x);
        p.y = range_min_y + rand() % (range_max_y - range_min_y);
		Chick* newChick = new Chick(pGame, p, 50, 50, image_path);
		chickList.push_back(newChick);
		newChick->draw();
		pGame->animalsList.push_back(newChick);
		pGame->chickList.push_back(newChick);
		//window* pWind = pGame->getWind();
		//pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	}
}

void CowIcon::onClick()
{
	cout << "Icon Cow Clicked" << endl;

	if (pGame->budget > 300 )
	{
		pGame->budget = pGame->budget - 300;

		pGame->clearBudget();
		string budget_string = "BUDGET = $" + to_string(pGame->budget);
		pGame->printBudget(budget_string);

		point p;

		p.x = range_min_x + rand() % (range_max_x - range_min_x);
		p.y = range_min_y + rand() % (range_max_y - range_min_y);

		Cow* newCow = new Cow(pGame, p, 60, 60, image_path);
		cowList.push_back(newCow);
		newCow->draw();
		pGame->animalsList.push_back(newCow);
		pGame->cowList.push_back(newCow);
	}
}


Budgetbar::Budgetbar(Game* r_pGame, point r_point, int r_width, int r_height) : Drawable(r_pGame, r_point, r_width, r_height)
{
	//First prepare List of images for each icon
	//To control the order of these images in the menu, reoder them in enum ICONS above	
	iconsImages[ICON_CHICK] = "images/chick.jpg";
	iconsImages[ICON_COW] = "images/cow.jpg";
	iconsImages[ICON_WATER] = "images/water.jpg";

	point p;
	p.x = 0;
	p.y = config.toolBarHeight;

	iconsList = new BudgetbarIcon * [ANIMAL_COUNT];

	//For each icon in the tool bar create an object 
	iconsList[ICON_CHICK] = new ChickIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_CHICK]);
	p.x += config.iconWidth;

	//p.x += config.iconWidth;
	//iconsList[ICON_CHICK] = new ChickIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_CHICK]);
	iconsList[ICON_COW] = new CowIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_COW]);
	p.x += config.iconWidth;

	iconsList[ICON_WATER] = new WaterIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_WATER]);
	p.x += config.iconWidth;
}

Budgetbar::~Budgetbar()
{
	for (int i = 0; i < ICON_COUNT; i++)
		delete iconsList[i];
	delete iconsList;
}

void Budgetbar::draw() const
{
	for (int i = 0; i < ANIMAL_COUNT; i++)
		iconsList[i]->draw();
	window* pWind = pGame->getWind();
	pWind->SetPen(BLACK, 3);
	pWind->DrawLine(0, 2*config.toolBarHeight, pWind->GetWidth(), 2*config.toolBarHeight);
}

bool Budgetbar::handleClick(int x, int y)
{
	if (x > ANIMAL_COUNT * config.iconWidth)	//click outside toolbar boundaries
		return false;


	//Check whick icon was clicked
	//==> This assumes that menu icons are lined up horizontally <==
	//Divide x co-ord of the point clicked by the icon width (int division)
	//if division result is 0 ==> first icon is clicked, if 1 ==> 2nd icon and so on

	int clickedIconIndex = (x / config.iconWidth);
	iconsList[clickedIconIndex]->onClick();	//execute onClick action of clicled icon

	//if (clickedIconIndex == ICON_EXIT) return true;

	return false;

}
WaterIcon::WaterIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path)
	: BudgetbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
}

void WaterIcon::onClick()
{
	cout << "Icon Water Clicked" << endl;

	if (pGame->budget >= 50)
	{
		pGame->budget -= 50;
		pGame->clearBudget();
		string budget_string = "BUDGET = $" + to_string(pGame->budget);
		pGame->printBudget(budget_string);
		pGame->drawGreenArea();
	}
}
