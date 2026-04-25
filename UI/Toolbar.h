#pragma once

#include "../Core/Drawable.h"

//Base class for all toolbar icons
class ToolbarIcon :public Drawable
{
private:
	string image_path;
public:
	ToolbarIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void draw() const override;
	virtual void onClick() = 0;   //The action that should be taken when this icon is clicked
};

class RestartIcon : public ToolbarIcon
{
public:
	RestartIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void onClick();
};

class ExitIcon : public ToolbarIcon
{
public:
	ExitIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
	virtual void onClick();
};
class PauseIcon : public ToolbarIcon
{
public:
    PauseIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
    virtual void onClick();
};

class ResumeIcon : public ToolbarIcon
{
public:
    ResumeIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
    virtual void onClick();
};

class SaveIcon : public ToolbarIcon
{
public:
    SaveIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
    virtual void onClick();
};

class LoadIconn : public ToolbarIcon
{
public:
    LoadIconn(Game* r_pGame, point r_point, int r_width, int r_height, string img_path);
    virtual void onClick();
};


// TO DO: The rest of icons in the toolbar

enum ICONS //The icons of the toolbar (you should add more icons)
{
	ICON_RESTART,
	ICON_PAUSE,
	ICON_RESUME,
	ICON_SAVE,
	ICON_LOAD,
	ICON_EXIT,
	ICON_COUNT
};

class Toolbar : public Drawable
{
private:
	ToolbarIcon** iconsList; //an array of toolbarIcon pointers
	string iconsImages[ICON_COUNT];

public:
	Toolbar(Game* r_pGame, point r_point, int r_width, int r_height);
	~Toolbar();
	void draw() const override;
	bool handleClick(int x, int y);	//handles clicks on toolbar icons, returns true if exit is clicked

};

