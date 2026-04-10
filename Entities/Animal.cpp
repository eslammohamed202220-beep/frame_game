#include "Animal.h"
#include "../Config/GameConfig.h"
#include "../Core/Game.h"
#include <iostream>
#include <cstdlib>

using namespace std;

Animal::Animal(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Drawable(r_pGame, r_point, r_width, r_height)
{
	image_path = img_path;
	curr_pos = r_point;
	curr_vel.x=1;
	curr_vel.y=1;

}

void Animal::draw() const
{
	//draw image of this object
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
}

Chick::Chick(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{}

void Chick::moveStep()
{
	//TO DO: add code for cleanup and game exit here
	/*
	//draw image of this object in the field
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
	*/
	window* pWind = pGame->getWind();
	pWind->SetPen(WHITE, 1);
	pWind->SetBrush(WHITE);
	pWind->DrawRectangle(RefPoint.x, RefPoint.y, RefPoint.x + width, RefPoint.y + height);

	int dx = (rand() % 3) - 1;
	int dy = (rand() % 3) - 1;

	int newX = RefPoint.x + dx * 20;
	int newY = RefPoint.y + dy * 20;

	if (newX < 0) newX = 0;
	if (newY < 2 * config.toolBarHeight) newY = 2 * config.toolBarHeight;
	if (newX > config.windWidth - width) newX = config.windWidth - width;
	if (newY > config.windHeight - config.statusBarHeight - height) newY = config.windHeight - config.statusBarHeight - height;

	RefPoint.x = newX;
	RefPoint.y = newY;

	draw();
}

Cow::Cow(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Animal(r_pGame, r_point, r_width, r_height, img_path)
{}

void Cow::moveStep()
{
	//TO DO: add code for cleanup and game exit here
	window* pWind = pGame->getWind();
	pWind->SetPen(WHITE, 1);
	pWind->SetBrush(WHITE);
	pWind->DrawRectangle(RefPoint.x, RefPoint.y, RefPoint.x + width, RefPoint.y + height);

	int dx = (rand() % 3) - 1;
	int dy = (rand() % 3) - 1;

	int newX = RefPoint.x + dx * 20;
	int newY = RefPoint.y + dy * 20;

	if (newX < 0) newX = 0;
	if (newY < 2 * config.toolBarHeight) newY = 2 * config.toolBarHeight;
	if (newX > config.windWidth - width) newX = config.windWidth - width;
	if (newY > config.windHeight - config.statusBarHeight - height) newY = config.windHeight - config.statusBarHeight - height;

	RefPoint.x = newX;
	RefPoint.y = newY;

	draw();

}