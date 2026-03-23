#pragma once
#include "../CMUgraphicsLib/CMUgraphics.h"

__declspec(selectany) //This line to prevent "redefinition error"

struct  //contains all game configurations (MAY be loaded from a configuration file later)
{
	int	windWidth = 1200, windHeight = 600,	//Window width and height
		wx = 5, wy = 5,			//Window starting coordinates

		//The window is divded into 3 areas: toolbar, playing area, statusbar
		toolBarHeight = 50,		//Tool Bar Height (distance from top of window to bottom line of toolbar)
		toolbarItemWidth = 40,			//Width of each icon in toolbar
		statusBarHeight = 50;	//Status Bar Height

	// Remaining Height 
	int playingAreaHeight = windHeight - toolBarHeight - statusBarHeight;

	color	penColor = BLUE,			//color used to draw lines/write messages
		bkGrndColor =WHITE,		//Background color
		statusBarColor = RED;	//background color of the status
	int penWidth = 3;			//width of the pen that draws shapes

	int	iconWidth = 70;			//Width of each icon in toolbar 
} config;





