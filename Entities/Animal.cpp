#include "Animal.h"
#include "../Config/GameConfig.h"
#include "../Core/Game.h"
#include "../CMUgraphicsLib/error.h"
#include <iostream>
#include <cstdlib>

using namespace std;

static int seekFoodHungerThreshold(Animal* a, int hungerLimit)
{
	// Chick: only path to food when 2 of 5 hunger bars remain
	if (dynamic_cast<Chick*>(a) != nullptr)
		return (3 * hungerLimit) / 5;
	return hungerLimit / 5;
}

static void ensureWanderVelocity(Animal* a)
{
	if (a->curr_vel.x == 0)
		a->curr_vel.x = (rand() % 2) ? 1 : -1;
	if (a->curr_vel.y == 0)
		a->curr_vel.y = (rand() % 2) ? 1 : -1;
}

static bool moveIfHungry(Animal* a, Game* g, int hungerLimit)
{
	if (dynamic_cast<Wolf*>(a) != nullptr)
		return false;
	if (++a->moveCount % config.hungerTickInterval == 0)
		a->hunger++;

	int seekAt = seekFoodHungerThreshold(a, hungerLimit);
	if (a->hunger < seekAt)
	{
		ensureWanderVelocity(a);
		return false;
	}

	Game::GreenArea* target = nullptr;
	int best = 1e9;
	for (int i = 0; i < (int)g->greenAreaList.size(); i++)
	{
		Game::GreenArea* area = g->greenAreaList[i];
		if (!area->active) continue;
		int cx = area->x + 25, cy = area->y + 20;
		int d = abs(a->getPosition().x - cx) + abs(a->getPosition().y - cy);
		if (d < best) { best = d; target = area; }
	}
	if (!target)
	{
		ensureWanderVelocity(a);
		return false;
	}

	int ax = a->getPosition().x;
	int ay = a->getPosition().y;
	if (target->x + 25 > ax)
		a->curr_vel.x = 1;
	else if (target->x + 25 < ax)
		a->curr_vel.x = -1;
	else
		a->curr_vel.x = (rand() % 2) ? 1 : -1;

	if (target->y + 20 > ay)
		a->curr_vel.y = 1;
	else if (target->y + 20 < ay)
		a->curr_vel.y = -1;
	else
		a->curr_vel.y = (rand() % 2) ? 1 : -1;

	return true;
}

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
	auto hungerLimit = [&]() -> int
	{
		if (dynamic_cast<const Chick*>(this) != nullptr) return 45;
		if (dynamic_cast<const Cow*>(this) != nullptr) return 55;
		if (dynamic_cast<const Wolf*>(this) != nullptr) return 35;
		return 50;
	};
	auto drawHungerBar = [&]()
	{
		if (dynamic_cast<const Wolf*>(this) != nullptr) return; // no hunger bar for wolves
		const int barW = width;
		const int barH = 6;
		const int barX = RefPoint.x;
		const int barY = RefPoint.y - 10;
		int maxH = hungerLimit();
		int clamped = hunger;
		if (clamped < 0) clamped = 0;
		if (clamped > maxH) clamped = maxH;
		int barsFull = 5 - (clamped * 5) / maxH; // full at spawn, drops over time
		if (barsFull < 0) barsFull = 0;
		if (barsFull > 5) barsFull = 5;
		int segW = barW / 5;
		pWind->SetPen(BLACK, 1);
		pWind->SetBrush(WHITE);
		pWind->DrawRectangle(barX, barY, barX + barW, barY + barH, FILLED);
		pWind->SetPen(GREEN, 1);
		pWind->SetBrush(GREEN);
		for (int i = 0; i < barsFull; i++)
		{
			int x1 = barX + i * segW + 1;
			int x2 = barX + (i + 1) * segW - 1;
			pWind->DrawRectangle(x1, barY + 1, x2, barY + barH - 1, FILLED);
		}
	};
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

	bool drawn = tryPath(image_path);

	string alt = image_path;
	for (char& c : alt)
	{
		if (c == '\\')
			c = '/';
	}
	if (!drawn && alt != image_path)
		drawn = tryPath(alt);

	if (!drawn)
	{
		pWind->SetPen(RED, 2);
		pWind->SetBrush(LIGHTGRAY);
		pWind->DrawRectangle(RefPoint.x, RefPoint.y, RefPoint.x + width, RefPoint.y + height, FILLED);
	}
	drawHungerBar();
}

Chick::Chick(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
}

void Chick::moveStep()
{
	int foodX = 0;
	int foodY = config.windHeight - config.statusBarHeight - 180;
	int foodW = 1200;
	int foodH = 170;

	int dx = curr_vel.x;
	int dy = curr_vel.y;

	if (moveIfHungry(this, pGame, 45))
	{
		dx = curr_vel.x;
		dy = curr_vel.y;
	}

	int newX = RefPoint.x + dx * config.animalMoveStepPxX;
	int newY = RefPoint.y + dy * config.animalMoveStepPxY;

	if (newX < foodX)
	{
		newX = foodX;
		curr_vel.x = -curr_vel.x;
	}
	else if (newX > foodX + foodW - width)
	{
		newX = foodX + foodW - width;
		curr_vel.x = -curr_vel.x;
	}

	if (newY < foodY)
	{
		newY = foodY;
		curr_vel.y = -curr_vel.y;
	}
	else if (newY > foodY + foodH - height)
	{
		newY = foodY + foodH - height;
		curr_vel.y = -curr_vel.y;
	}

	RefPoint.x = newX;
	RefPoint.y = newY;
}

Cow::Cow(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
}

void Cow::moveStep()
{
	int foodX = 0;
	int foodY = config.windHeight - config.statusBarHeight - 180;
	int foodW = 1200;
	int foodH = 170;

	int dx = curr_vel.x;
	int dy = curr_vel.y;

	if (moveIfHungry(this, pGame, 55))
	{
		dx = curr_vel.x;
		dy = curr_vel.y;
	}

	int newX = RefPoint.x + dx * config.animalMoveStepPxX;
	int newY = RefPoint.y + dy * config.animalMoveStepPxY;

	if (newX < foodX)
	{
		newX = foodX;
		curr_vel.x = -curr_vel.x;
	}
	else if (newX > foodX + foodW - width)
	{
		newX = foodX + foodW - width;
		curr_vel.x = -curr_vel.x;
	}

	if (newY < foodY)
	{
		newY = foodY;
		curr_vel.y = -curr_vel.y;
	}
	else if (newY > foodY + foodH - height)
	{
		newY = foodY + foodH - height;
		curr_vel.y = -curr_vel.y;
	}

	RefPoint.x = newX;
	RefPoint.y = newY;
}

Wolf::Wolf(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{
}
void Wolf::moveStep()
{
	int foodX = 0;
	int foodY = config.windHeight - config.statusBarHeight - 180;
	int foodW = 1200;
	int foodH = 170;

	int dx = curr_vel.x;
	int dy = curr_vel.y;

	if (moveIfHungry(this, pGame, 35))
	{
		dx = curr_vel.x;
		dy = curr_vel.y;
	}

	int newX = RefPoint.x + dx * config.wolfMoveStepPx;
	int newY = RefPoint.y + dy * (config.wolfMoveStepPx - 1);

	if (newX < foodX)
	{
		newX = foodX;
		curr_vel.x = -curr_vel.x;
	}
	else if (newX > foodX + foodW - width)
	{
		newX = foodX + foodW - width;
		curr_vel.x = -curr_vel.x;
	}

	if (newY < foodY)
	{
		newY = foodY;
		curr_vel.y = -curr_vel.y;
	}
	else if (newY > foodY + foodH - height)
	{
		newY = foodY + foodH - height;
		curr_vel.y = -curr_vel.y;
	}

	RefPoint.x = newX;
	RefPoint.y = newY;}
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
