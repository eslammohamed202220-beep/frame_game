#include "Animal.h"
#include "../Config/GameConfig.h"
#include "../Core/Game.h"
#include "../CMUgraphicsLib/error.h"
#include <iostream>
#include <cstdlib>

using namespace std;

Animal::Animal(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Drawable(r_pGame, r_point, r_width, r_height)
{
	image_path = img_path;
	curr_pos = r_point;
	curr_vel.x = 1;
	curr_vel.y = 1;
}

void Animal::draw() const
{
	window* pWind = pGame->getWind();
	auto tryPath = [&](const string& path) -> bool
	{
		try
		{
			image img(path);
			pWind->DrawImage(img, RefPoint.x, RefPoint.y, width, height);
			return true;
		}
		catch (error)
		{
			return false;
		}
	};

	if (tryPath(image_path))
		return;

	string alt = image_path;
	for (char& c : alt)
	{
		if (c == '\\')
			c = '/';
	}
	if (alt != image_path && tryPath(alt))
		return;

	pWind->SetPen(RED, 2);
	pWind->SetBrush(LIGHTGRAY);
	pWind->DrawRectangle(RefPoint.x, RefPoint.y, RefPoint.x + width, RefPoint.y + height, FRAME);
}

Chick::Chick(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
}

void Chick::moveStep()
{
	//TO DO: add code for cleanup and game exit here

	// Frequency: Moves roughly every 4-5 frames
	if ((rand() % 4) != 0) return;

	int dx = (rand() % 3) - 1;
	int dy = (rand() % 3) - 1;

	// Jump size: Lowered to 15 pixels
	int newX = RefPoint.x + dx *30;
	int newY = RefPoint.y + dy * 10;
	int foodX = 0;
	int foodY = config.windHeight - config.statusBarHeight - 180;
	int foodW = 1200;
	int foodH = 170;
	if (newX < foodX) newX = foodX;
	if (newX > foodX + foodW - width) newX = foodX + foodW - width;
	if (newY < foodY) newY = foodY;
	if (newY > foodY + foodH - height) newY = foodY + foodH - height;

	RefPoint.x = newX;
	RefPoint.y = newY;
}

Cow::Cow(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
}

void Cow::moveStep()
{
	//TO DO: add code for cleanup and game exit here

	// Frequency: Moves roughly every 8-10 frames
	if ((rand() % 8) != 0) return;

	int dx = (rand() % 3) - 1;
	int dy = (rand() % 3) - 1;

	int newX = RefPoint.x + dx * 40;
	int newY = RefPoint.y + dy * 12;
	int foodX = 0;
	int foodY = config.windHeight - config.statusBarHeight - 180;
	int foodW = 1200;
	int foodH = 170;
	if (newX < foodX) newX = foodX;
	if (newX > foodX + foodW - width) newX = foodX + foodW - width;
	if (newY < foodY) newY = foodY;
	if (newY > foodY + foodH - height) newY = foodY + foodH - height;

	RefPoint.x = newX;
	RefPoint.y = newY;
}

Wolf::Wolf(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
}

void Wolf::moveStep()
{
	if ((rand() % 3) != 0) return;
	int dx = (rand() % 3) - 1;
	int dy = (rand() % 3) - 1;
	int newX = RefPoint.x + dx * 50;
	int newY = RefPoint.y + dy * 20;
	int foodX = 0;
	int foodY = config.windHeight - config.statusBarHeight - 180;
	int foodW = 1200;
	int foodH = 170;
	if (newX < foodX) newX = foodX;
	if (newX > foodX + foodW - width) newX = foodX + foodW - width;
	if (newY < foodY) newY = foodY;
	if (newY > foodY + foodH - height) newY = foodY + foodH - height;

	RefPoint.x = newX;
	RefPoint.y = newY;
}
void Chick::egg()
{
	Game::Item* item = new Game::Item();
	item->pos.x = RefPoint.x + 20;
	item->pos.y = RefPoint.y + 20;
	item->type = "egg";
	pGame->ItemList.push_back(item);
}
void Cow::milk()
{
	Game::Item* item = new Game::Item();
	item->pos.x = RefPoint.x + 20;
	item->pos.y = RefPoint.y + 20;
	item->type = "milk";
	pGame->ItemList.push_back(item);
}
