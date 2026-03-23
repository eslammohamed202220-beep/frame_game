#include "Drawable.h"

Drawable::Drawable(Game* r_pGame, point r_point, int r_width, int r_height)
{
	pGame = r_pGame;
	RefPoint = r_point;
	width = r_width;
	height = r_height;
}
