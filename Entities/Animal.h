#pragma once
#include "../Core/Drawable.h"

class Animal : public Drawable
{
private:
	string image_path;
public:
	point curr_pos;
	point curr_vel;
	int hunger = 0;
	int moveCount = 0;
	int health = 0; // FIX: added missing health member used in Game::Wolfadd()

	Animal(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void draw() const override;
	virtual void moveStep() = 0;   //The action that should be taken each time step
	point getPosition() const
	{
		return RefPoint;
	}
};

class Chick : public Animal
{
public:
	Chick(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void moveStep();
	void egg();
};

class Cow : public Animal
{
public:
	Cow(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void moveStep();
	void milk();
};

class Wolf : public Animal
{
public:
	Wolf(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void moveStep();
};