// Must be defined in exactly ONE .cpp file before including Game.h
#define MINIAUDIO_IMPLEMENTATION
#include "Game.h"
#include "../CMUgraphicsLib/error.h"
#include "../Config/GameConfig.h"
#include "../UI/Toolbar.h"    // full type needed — forward-declared in Game.h
#include "../UI/BudgetBar.h"  // full type needed — forward-declared in Game.h
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <random>


using namespace std;
namespace {

    bool tryDrawJpeg(window* pWind, const char* path, int x, int y, int w, int h) {
        try {
            image img(path);
            pWind->DrawImage(img, x, y, w, h);
            return true;
        }
        catch (error) {
            return false;
        }
    }

    bool tryDrawJpeg(window& w, const char* path, int x, int y, int iw, int ih) {
        try {
            image img(path);
            w.DrawImage(img, x, y, iw, ih);
            return true;
        }
        catch (error) {
            return false;
        }
    }

    bool drawFirstMatch(window& w, const char* const* paths, size_t nPaths, int x,
        int y, int iw, int ih) {
        for (size_t i = 0; i < nPaths; ++i) {
            if (tryDrawJpeg(w, paths[i], x, y, iw, ih))
                return true;
        }
        return false;
    }

    const char* LEADERBOARD_FILE = "leaderboard.txt";
    const int LEADERBOARD_TOP_N = 10;

    string sanitizeUsername(string name) {
        string out;
        for (char c : name) {
            if (isalnum((unsigned char)c) || c == '_')
                out += c;
        }
        if (out.empty())
            return "Player";
        if (out.size() > 15)
            out.resize(15);
        return out;
    }

    int animalHungerLimit(const Animal* a) {
        if (dynamic_cast<const Cow*>(a) != nullptr)
            return 55;
        if (dynamic_cast<const Chick*>(a) != nullptr)
            return 45;
        return 50;
    }

    bool animalIsHungryEnoughToEat(const Animal* a) {
        const int limit = animalHungerLimit(a);
        if (dynamic_cast<const Chick*>(a) != nullptr)
            return a->hunger >= (3 * limit) / 5; // 2 of 5 bars remaining
        return a->hunger >= limit / 5;
    }

} // namespace

// ==========================
// Constructors / Destructor
// ==========================

Game::Game() {
    // Initialization for logic
    srand(time(0));
    budget = 1000;
    wolf_Show = false;
    // 1 - Create the main window
    pWind = CreateWind(config.windWidth, config.windHeight, config.wx, config.wy);

    // CRITICAL: Set buffering ONCE in constructor to avoid "BitBlt" Fatal Error
    pWind->SetBuffering(true);

    // 2 - create and draw the toolbar
    createToolbar();
    createBudgetbar();

    // 3 - create and draw the playing field (background, warehouse, boundary
    // outline)
    drawBackground();
    drawWarehouse();
    drawFieldBoundaries();
    warehouseContent();
    animalCounter();

    // 4- Create the Plane
    // TODO: Add code to create and draw the Plane

    // 5- Create the Bullet
    // TODO: Add code to create and draw the Bullet

    // 6- Create the enemies
    // TODO: Add code to create and draw enemies in random places

    // 7- Create and clear the status bar
    clearStatusBar();

    // intilaization for level and timer ints
    level = 1;
    timer = 60 + (level - 1) * 30;
    lasttime = time(0);
    lastWolfSpawnTime = time(0);
    isGameOver = false;
    gameStarted = false;
    playerName = "Player";

    printBudget("BUDGET = $" + to_string(budget));
    writeStatus();

    pWind->UpdateBuffer();

    // -------------------------------------------------------
    // Background Music
    // -------------------------------------------------------
    const char* MUSIC_PATH = "sounds/background.mp3";

    if (ma_engine_init(NULL, &audioEngine) == MA_SUCCESS) {
        if (ma_sound_init_from_file(&audioEngine, MUSIC_PATH, MA_SOUND_FLAG_STREAM,
            NULL, NULL, &bgMusic) == MA_SUCCESS) {
            ma_sound_set_looping(&bgMusic, MA_TRUE); // loop forever
            ma_sound_set_volume(&bgMusic, 0.4f);     // 40% volume (0.0 to 1.0)
            ma_sound_start(&bgMusic);
            audioReady = true;
        }
        else {
            // File not found or unsupported — engine still alive, no music
            ma_engine_uninit(&audioEngine);
        }
    }
}

Game::~Game() {
    for (int i = 0; i < animalsList.size(); i++)
        delete animalsList[i];

    for (int i = 0; i < ItemList.size(); i++)
        delete ItemList[i];

    for (int i = 0; i < greenAreaList.size(); i++)
        delete greenAreaList[i];

    animalsList.clear();
    chickList.clear();
    cowList.clear();
    ItemList.clear();
    greenAreaList.clear();

    delete gameToolbar;
    delete gameBudgetbar;

    // Clean up audio resources
    if (audioReady) {
        ma_sound_uninit(&bgMusic);
        ma_engine_uninit(&audioEngine);
    }

    delete pWind;
}

// ==========================
// Input
// ==========================

clicktype Game::getMouseClick(int& x, int& y) const {
    return pWind->WaitMouseClick(x, y); // Wait for mouse click
}

string Game::getSrting() const {
    string Label;
    char Key;
    keytype ktype;
    pWind->FlushKeyQueue();
    while (1) {
        ktype = pWind->WaitKeyPress(Key);
        if (ktype == ESCAPE) // ESCAPE key is pressed
            return "";         // returns nothing as user has cancelled label
        if (Key == 13)       // ENTER key is pressed
            return Label;
        if (Key == 8) // BackSpace is pressed
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

window* Game::CreateWind(int w, int h, int x, int y) const {
    window* pW = new window(w, h, x, y);
    pW->SetBrush(config.bkGrndColor);
    pW->SetPen(config.bkGrndColor, 1);
    pW->DrawRectangle(0, 0, w, h);
    return pW;
}

void Game::createToolbar() {
    point toolbarUpperleft;
    toolbarUpperleft.x = 0;
    toolbarUpperleft.y = 0;

    gameToolbar = new Toolbar(this, toolbarUpperleft, 0, config.toolBarHeight);
    gameToolbar->draw();
}

void Game::createBudgetbar() {
    point budgetbarUpperleft;
    budgetbarUpperleft.x = 0;
    budgetbarUpperleft.y = config.toolBarHeight;

    gameBudgetbar =
        new Budgetbar(this, budgetbarUpperleft, 0, config.toolBarHeight);
    gameBudgetbar->draw();
}

window* Game::getWind() const { return pWind; }

// ==========================
// Status Display
// ==========================

void Game::clearBudget() const {
    // Clear Budget area by drawing a filled rectangle
    pWind->SetPen(config.bkGrndColor, 1);
    pWind->SetBrush(config.bkGrndColor);
    pWind->DrawRectangle(config.windWidth - 340, config.toolBarHeight,
        config.windWidth, 2 * config.toolBarHeight);
}

void Game::printBudget(string msg) const {
    clearBudget(); // First clear the budget area

    pWind->SetPen(config.penColor, 50);
    pWind->SetFont(24, BOLD, BY_NAME, "Arial");
    pWind->DrawString(config.windWidth - 250, config.toolBarHeight + 10, msg);
}

void Game::clearStatusBar() const {
    // Clear Status bar by drawing a filled rectangle
    pWind->SetPen(config.statusBarColor, 1);
    pWind->SetBrush(config.statusBarColor);
    pWind->DrawRectangle(0, config.windHeight - config.statusBarHeight,
        config.windWidth, config.windHeight);
}

void Game::printMessage(string msg) const {
    clearStatusBar(); // First clear the status bar

    pWind->SetPen(config.penColor, 50);
    pWind->SetFont(24, BOLD, BY_NAME, "Arial");
    pWind->DrawString(
        10, config.windHeight - (int)(0.85 * config.statusBarHeight), msg);
}

void Game::writeStatus() const {
    clearStatusBar();
    pWind->SetPen(config.penColor, 50);
    pWind->SetFont(20, BOLD, BY_NAME, "Arial");
    int y_pos = config.windHeight - config.statusBarHeight + 10;

    int goal = 2000 + (level - 1) * 1000;
    string timelevelmsg = "  PLAYER: " + playerName + " | TIMER = " +
        to_string(timer) + " | LEVEL = " + to_string(level) +
        " | Animals = " + to_string(animalsList.size()) +
        " | GOAL: $" + to_string(goal);
    pWind->DrawString(10, y_pos, timelevelmsg);
}

void Game::warehouseContent() const {
    pWind->SetPen(config.penColor, 50);
    pWind->SetFont(16, BOLD, BY_NAME, "Arial");

    int y_pos_Structure = config.windHeight - 390;
    string wareHouseStructure = "  Item:  |  Count |  price ";
    pWind->DrawString(985, (y_pos_Structure), wareHouseStructure);

    int y_pos_egg = config.windHeight - 370;
    string egg = "  Egg:   |      " + to_string(eggInWareHouse) + "     |   100 ";
    pWind->DrawString(985, y_pos_egg, egg);

    int y_pos_milk = config.windHeight - 350;
    string milk =
        "  Milk:  |      " + to_string(milkInWareHouse) + "      |   200 ";
    pWind->DrawString(985, y_pos_milk, milk);
}
void Game::animalCounter() const {
    for (int i = 0; i < (int)animalsList.size(); i++) {
        Animal* animal = animalsList[i];
        point p = animal->getPosition();
        animalsList[i]->draw();

        pWind->SetPen(RED, 2);
        pWind->SetFont(14, BOLD, BY_NAME, "Arial");
        pWind->DrawString(p.x + 25, p.y - 25, to_string(i + 1));
    }
}
// ==========================
// Game Logic
// ==========================

void Game::updateTimer() {
    time_t now = time(0);

    if (timer > 0 && now > lasttime) {
        timer -= (int)(now - lasttime);
        if (timer < 0)
            timer = 0;

        lasttime = now;
        wolf_Show = false;
        egg_show = false;
        milk_show = false;
    }

    checkLevelUp();

    if (timer == 0 && gameStarted && !isGameOver) {
        gameOver();
    }
}

void Game::checkLevelUp() {
    int goal = 2000 + (level - 1) * 1000;
    if (budget >= goal && timer > 0) {
        level++;
        timer = 60 + (level - 1) * 30;
        lasttime = time(0);
    }
}

void Game::loadLeaderboard() {
    leaderboardEntries.clear();
    ifstream file(LEADERBOARD_FILE);
    if (!file)
        return;

    string name;
    int score = 0;
    while (file >> name >> score) {
        bool found = false;
        for (auto& entry : leaderboardEntries) {
            if (entry.first == name) {
                entry.second = max(entry.second, score);
                found = true;
                break;
            }
        }
        if (!found)
            leaderboardEntries.push_back({ name, score });
    }

    sort(leaderboardEntries.begin(), leaderboardEntries.end(),
        [](const pair<string, int>& a, const pair<string, int>& b) {
            return a.second > b.second;
        });
}

void Game::saveLeaderboard() const {
    ofstream file(LEADERBOARD_FILE);
    if (!file)
        return;

    for (const auto& entry : leaderboardEntries)
        file << entry.first << " " << entry.second << "\n";
}

void Game::updatePlayerHighScore(int score) {
    bool found = false;
    for (auto& entry : leaderboardEntries) {
        if (entry.first == playerName) {
            entry.second = max(entry.second, score);
            found = true;
            break;
        }
    }
    if (!found)
        leaderboardEntries.push_back({ playerName, score });

    sort(leaderboardEntries.begin(), leaderboardEntries.end(),
        [](const pair<string, int>& a, const pair<string, int>& b) {
            return a.second > b.second;
        });
    saveLeaderboard();
}

void Game::promptUsername() {
    static const char* kUsernameUi[] = {
        "images/Username interface.jpg",
        "images\\Username interface.jpg",
        "../images/Username interface.jpg",
        "..\\images\\Username interface.jpg",
    };

    const int playY = 2 * config.toolBarHeight;
    const int playBottom = config.windHeight - config.statusBarHeight;
    const int playH = playBottom - playY;
    const int inputY = playY + playH / 2 + 18;

    auto drawDialog = [&](const string& typed) {
        drawTopBars();
        clearStatusBar();

        if (!drawFirstMatch(*pWind, kUsernameUi,
            sizeof(kUsernameUi) / sizeof(kUsernameUi[0]), 0, playY,
            config.windWidth, playH)) {
            const int cx = config.windWidth / 2;
            const int cy = playY + playH / 2;
            pWind->SetBrush(LIGHTGRAY);
            pWind->SetPen(BLACK, 2);
            pWind->DrawRectangle(cx - 300, cy - 100, cx + 300, cy + 100, FILLED);
            pWind->SetFont(20, BOLD, BY_NAME, "Arial");
            pWind->DrawString(cx - 150, cy - 10, "Username UI image not found");
        }

        if (!typed.empty()) {
            pWind->SetPen(BLACK, 1);
            pWind->SetFont(26, BOLD, BY_NAME, "Arial");
            int textW = 0, textH = 0;
            pWind->GetStringSize(textW, textH, typed);
            pWind->DrawString(config.windWidth / 2 - textW / 2, inputY, typed);
        }

        pWind->UpdateBuffer();
        };

    string label;
    char key = '\0';
    keytype ktype = NO_KEYPRESS;
    pWind->FlushKeyQueue();
    drawDialog(label);

    while (true) {
        ktype = pWind->WaitKeyPress(key);
        if (ktype == ESCAPE)
            break;
        if (key == 13)
            break;
        if (key == 8) {
            if (!label.empty())
                label.pop_back();
        }
        else {
            label += key;
        }
        drawDialog(label);
    }

    playerName = sanitizeUsername(label);
    redrawScene();
    printMessage("Welcome, " + playerName + "!");
    pWind->UpdateBuffer();
}

void Game::gameOver() {
    isGameOver = true;
    isPaused = true;

    const int score = budget;
    int previousHigh = 0;
    for (const auto& entry : leaderboardEntries) {
        if (entry.first == playerName) {
            previousHigh = entry.second;
            break;
        }
    }

    updatePlayerHighScore(score);
    const bool newRecord = score > previousHigh;

    redrawScene();
    int centerX = config.windWidth / 2;
    int centerY = config.windHeight / 2;
    int topN = min((int)leaderboardEntries.size(), LEADERBOARD_TOP_N);
    int panelTop = centerY - 95 - topN * 24;
    int panelBottom = centerY + 115;

    pWind->SetPen(BLACK, 2);
    pWind->SetBrush(BLACK);
    pWind->DrawRectangle(centerX - 290, panelTop, centerX + 290, panelBottom,
        FILLED);
    pWind->SetPen(RED, 3);
    pWind->DrawRectangle(centerX - 290, panelTop, centerX + 290, panelBottom,
        FRAME);

    pWind->SetPen(RED, 1);
    pWind->SetFont(40, BOLD, BY_NAME, "Arial");
    pWind->DrawString(centerX - 130, panelTop + 15, "GAME OVER!");

    pWind->SetPen(WHITE, 1);
    pWind->SetFont(20, BOLD, BY_NAME, "Arial");
    string finalMsg =
        playerName + " - Final Score: $" + to_string(score);
    pWind->DrawString(centerX - 170, panelTop + 60, finalMsg);

    if (newRecord) {
        pWind->SetPen(YELLOW, 1);
        pWind->SetFont(18, BOLD, BY_NAME, "Arial");
        pWind->DrawString(centerX - 95, panelTop + 88, "NEW HIGH SCORE!");
    }

    pWind->SetPen(LIGHTGRAY, 1);
    pWind->SetFont(18, BOLD, BY_NAME, "Arial");
    pWind->DrawString(centerX - 70, centerY - 15, "TOP SCORES");

    int y = centerY + 10;
    for (int i = 0; i < topN; i++) {
        const string& name = leaderboardEntries[i].first;
        const int high = leaderboardEntries[i].second;
        string line =
            to_string(i + 1) + ". " + name + "  $" + to_string(high);
        if (name == playerName)
            pWind->SetPen(YELLOW, 1);
        else
            pWind->SetPen(WHITE, 1);
        pWind->SetFont(16, BOLD, BY_NAME, "Arial");
        pWind->DrawString(centerX - 160, y, line);
        y += 24;
    }

    pWind->UpdateBuffer();
    Sleep(6000);
    isGameOver = false;
    isPaused = false;
    gameStarted = true;
    level = 1;
    eggInWareHouse = 0;
    milkInWareHouse = 0;
    numchick = 0;
    numcow = 0;
    restartGame();
}

void Game::Wolfadd() {
    time_t now = time(0);
    int spawnCooldown = max(6, 12 - level);
    randNum = rand() % 100;
    int activeWolves = 0;
    for (int i = 0; i < (int)animalsList.size(); i++) {
        if (dynamic_cast<Wolf*>(animalsList[i]) != nullptr)
            activeWolves++;
    }

    if (timer > 0 && (now - lastWolfSpawnTime) >= spawnCooldown &&
        randNum < (2 + level) && activeWolves < 2 && !wolf_Show) {
        point p;

        p.x = rand() % 1130;
        p.y = 370 + rand() % 100;

        Wolf* newWolf = new Wolf(this, p, 70, 70, "images/wolff.jpg");
        newWolf->health = 5;
        animalsList.push_back(newWolf);
        playSfx("sounds/Wolf drop effect.mp3");
        lastWolfSpawnTime = now;
        wolf_Show = true;
    }
}

void Game::eggadd() {
    if (timer > 0 && timer % 10 == 0 && !egg_show) {
        for (int i = 0; i < (int)chickList.size(); i++) {
            chickList[i]->egg();
        }
        egg_show = true;
    }
}

void Game::milkadd() {
    if (timer > 0 && timer % 15 == 0 && !milk_show) {
        for (int i = 0; i < (int)cowList.size(); i++) {
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
void Game::sellegg() {
    eggInWareHouse--;
    budget += 100;
}
void Game::sellmilk() {
    milkInWareHouse--;
    budget += 200;
}
void Game::restartGame() {

    for (int i = 0; i < (int)animalsList.size(); i++) {
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

    for (int i = 0; i < (int)greenAreaList.size(); i++) {
        delete greenAreaList[i];
    }
    greenAreaList.clear();

    for (int i = 0; i < (int)ItemList.size(); i++) {
        delete ItemList[i];
    }
    ItemList.clear();

    redrawScene();
}

void Game::drawTopBars() const {
    pWind->SetPen(config.bkGrndColor, 1);
    pWind->SetBrush(config.bkGrndColor);
    pWind->DrawRectangle(0, 0, config.windWidth, 2 * config.toolBarHeight, FILLED);
    gameToolbar->draw();
    gameBudgetbar->draw();
}

void Game::drawBackground() const {
    int playY = 2 * config.toolBarHeight;
    int playH = config.windHeight - config.statusBarHeight - playY;

    pWind->SetPen(LAVENDER);
    pWind->SetBrush(LAVENDER);
    pWind->DrawRectangle(0, playY, config.windWidth,
        config.windHeight - config.statusBarHeight);

    static const char* kBgPaths[] = {
        "images/Background.jpg",
        "images\\Background.jpg",
        "../images/Background.jpg",
        "..\\images\\Background.jpg",
    };

    for (const char* path : kBgPaths) {
        if (tryDrawJpeg(pWind, path, 0, playY, config.windWidth, playH))
            return;
    }
}

void Game::drawFieldBoundaries() const {
    int topY = 2 * config.toolBarHeight;
    int bottomY = config.windHeight - config.statusBarHeight;

    pWind->SetPen(DARKGREEN, 4);
    pWind->DrawRectangle(0, topY, config.windWidth, bottomY, FRAME);
}

void Game::drawWarehouse() const {
    int playY = 2 * config.toolBarHeight;
    const int warehouseW = 220;
    const int warehouseH = 180;
    int wx = config.windWidth - warehouseW - 30;
    int wy = playY + 20;

    static const char* kWarehousePaths[] = { "images/Warehouse.jpg",
                                            "../images/Warehouse.jpg" };

    for (const char* path : kWarehousePaths) {
        if (tryDrawJpeg(pWind, path, wx, wy, warehouseW, warehouseH))
            return;
    }
}

void Game::drawFoodArea() const {
    int x = 0;
    int y = config.windHeight - config.statusBarHeight - 180;
    int width = 1200;
    int height = 170;

    pWind->SetPen(BLACK, 2);
    pWind->DrawRectangle(x, y, x + width, y + height, FRAME);

    pWind->SetPen(BLACK, 1);
    pWind->DrawString(x + 5, y - 18, "Food Area");
}
void Game::drawGreenArea() {
    int foodX = 0;
    int foodY = config.windHeight - config.statusBarHeight - 180;
    int foodW = 1200;
    int foodH = 170;
    int areaW = 50;
    int areaH = 40;
    GreenArea* newArea = new GreenArea();
    newArea->x = foodX + rand() % (foodW - areaW);
    newArea->y = foodY + rand() % (foodH - areaH);
    newArea->counter = 2;
    newArea->active = true;
    newArea->eatTimer = 0;

    greenAreaList.push_back(newArea);
}
void Game::checkAnimalGrassCollision() {
    const int grassW = 50;
    const int grassH = 40;

    const int chickW = 60;
    const int chickH = 60;

    const int cowW = 80;
    const int cowH = 80;

    for (int i = 0; i < (int)greenAreaList.size(); i++) {
        GreenArea* grass = greenAreaList[i];

        if (!grass->active)
            continue;

        bool eating = false;
        bool onGrass = false;

        for (int j = 0; j < (int)chickList.size(); j++) {
            point chickPos = chickList[j]->getPosition();

            int animalX = chickPos.x;
            int animalY = chickPos.y;

            if (animalX < grass->x + grassW && animalX + chickW > grass->x &&
                animalY < grass->y + grassH && animalY + chickH > grass->y) {
                onGrass = true;
                if (animalIsHungryEnoughToEat(chickList[j])) {
                    eating = true;
                    chickList[j]->hunger = 0;
                }
            }
        }

        for (int j = 0; j < (int)cowList.size(); j++) {
            point cowPos = cowList[j]->getPosition();

            int animalX = cowPos.x;
            int animalY = cowPos.y;

            if (animalX < grass->x + grassW && animalX + cowW > grass->x &&
                animalY < grass->y + grassH && animalY + cowH > grass->y) {
                onGrass = true;
                if (animalIsHungryEnoughToEat(cowList[j])) {
                    eating = true;
                    cowList[j]->hunger = 0;
                }
            }
        }

        // Keep eating until the patch is consumed if a hungry animal started a meal
        if (grass->eatTimer > 0 && onGrass)
            eating = true;

        if (eating) {
            grass->eatTimer++;
            if (grass->eatTimer >= config.grassEatTicksRequired) {
                grass->counter--;
                grass->eatTimer = 0;

                if (grass->counter <= 0) {
                    delete grass;
                    greenAreaList.erase(greenAreaList.begin() + i);
                    i--;
                    break;
                }
            }
        }
        else {
            grass->eatTimer = 0;
        }
    }
}
bool Game::isWarehouseClicked(int x, int y) const {
    int playY = 2 * config.toolBarHeight;
    const int warehouseW = 220;
    const int warehouseH = 180;
    int wx = config.windWidth - warehouseW - 30;
    int wy = playY + 20;
    return x >= wx && x <= wx + warehouseW && y >= wy && y <= wy + warehouseH;
}

void Game::drawWarehouseUI(window& infoWin) {
    image bg("../images/best-shop.jpg");
    infoWin.DrawImage(bg, 0, 0, 960, 540);

    image egg("../images/egg.jpg");
    infoWin.DrawImage(egg, 205, 160, 90, 90);

    image milk("../images/milk.jpg");
    infoWin.DrawImage(milk, 320, 157, 90, 90);

    infoWin.SetPen(BLACK, 3);
    infoWin.SetFont(18, BOLD, BY_NAME, "Arial");

    string eggText = "Egg: " + to_string(eggInWareHouse);
    infoWin.DrawString(200, 260, eggText.c_str());

    string milkText = "Milk: " + to_string(milkInWareHouse);
    infoWin.DrawString(330, 260, milkText.c_str());
}

void Game::openWarehouseWindow() {
    static const char* kShopBg[] = {
        "images/best-shop.jpg",
        "../images/best-shop.jpg",
        "images\\best-shop.jpg",
        "..\\images\\best-shop.jpg",
    };
    static const char* kEggImg[] = {
        "images/egg.jpg",
        "../images/egg.jpg",
        "images\\egg.jpg",
        "..\\images\\egg.jpg",
    };
    static const char* kMilkImg[] = {
        "images/milk.jpg",
        "../images/milk.jpg",
        "images\\milk.jpg",
        "..\\images\\milk.jpg",
    };

    const int winW = 960;
    const int winH = 540;

    int winX = config.wx + (config.windWidth - winW) / 2;
    int winY = config.wy + (config.windHeight - winH) / 2;

    window infoWin(winW, winH, winX, winY);

    // Do not use SetBuffering on a second CMUgraphics window: tearing it down
    // while a bitmap is still selected into the off-screen DC can destabilize GDI
    // / the process.

    const int BOX_W = 180;
    const int ICON_W = 90;
    const int ICON_H = 90;

    const int EGG_BOX_X = 160;
    const int EGG_BOX_Y = 160;
    const int EGG_X = EGG_BOX_X + (BOX_W - ICON_W) / 2;
    const int EGG_Y = EGG_BOX_Y;

    const int MILK_BOX_X = 300;
    const int MILK_BOX_Y = 160;
    const int MILK_X = MILK_BOX_X + (BOX_W - ICON_W) / 2;
    const int MILK_Y = MILK_BOX_Y;

    const int EXIT_X = 720;
    const int EXIT_Y = 40;
    const int EXIT_W = 60;
    const int EXIT_H = 60;

    int x, y;

    while (true) {
        if (!drawFirstMatch(infoWin, kShopBg, sizeof(kShopBg) / sizeof(kShopBg[0]),
            0, 0, winW, winH)) {
            infoWin.SetBrush(WHITE);
            infoWin.SetPen(BLACK, 1);
            infoWin.DrawRectangle(0, 0, winW, winH, FILLED);
        }

        infoWin.SetPen(BLACK, 2);
        infoWin.SetBrush(LIGHTGRAY);
        infoWin.DrawRectangle(EXIT_X, EXIT_Y, EXIT_X + EXIT_W, EXIT_Y + EXIT_H,
            FILLED);
        infoWin.SetPen(config.penColor, 2);
        infoWin.SetFont(14, BOLD, BY_NAME, "Arial");
        infoWin.DrawString(EXIT_X + 8, EXIT_Y + 20, "EXIT");

        infoWin.SetPen(config.penColor, 50);
        infoWin.SetFont(15, BOLD, BY_NAME, "Arial");

        if (eggInWareHouse > 0) {
            drawFirstMatch(infoWin, kEggImg, sizeof(kEggImg) / sizeof(kEggImg[0]),
                EGG_X, EGG_Y, ICON_W, ICON_H);
            string egg = "Egg: " + to_string(eggInWareHouse);
            infoWin.DrawString(160, 260, egg);
            string eggprice = "  $" + to_string(100);
            infoWin.DrawString(260, 260, eggprice);
        }
        if (milkInWareHouse > 0) {
            drawFirstMatch(infoWin, kMilkImg, sizeof(kMilkImg) / sizeof(kMilkImg[0]),
                MILK_X, MILK_Y, ICON_W, ICON_H);
            string milk = "Milk: " + to_string(milkInWareHouse);
            infoWin.DrawString(300, 260, milk);
            string milkprice = "  $" + to_string(200);
            infoWin.DrawString(400, 260, milkprice);
        }

        infoWin.WaitMouseClick(x, y);

        if (x >= EXIT_X && x <= EXIT_X + EXIT_W && y >= EXIT_Y &&
            y <= EXIT_Y + EXIT_H)
            break;

        if (x >= EGG_X && x <= EGG_X + ICON_W && y >= EGG_Y &&
            y <= EGG_Y + ICON_H && eggInWareHouse > 0)
            sellegg();

        if (x >= MILK_X && x <= MILK_X + ICON_W && y >= MILK_Y &&
            y <= MILK_Y + ICON_H && milkInWareHouse > 0)
            sellmilk();
    }
}
void Game::redrawScene() const {
    drawTopBars();
    drawBackground();
    drawWarehouse();
    drawFieldBoundaries();
    drawFoodArea();
    warehouseContent();
    animalCounter();

    for (int i = 0; i < (int)ItemList.size(); i++) {
        if (ItemList[i]->type == "egg")
            pWind->DrawImage("images\\egg.jpg", ItemList[i]->pos.x,
                ItemList[i]->pos.y, 30, 30);
        else if (ItemList[i]->type == "milk")
            pWind->DrawImage("images\\milk.jpg", ItemList[i]->pos.x,
                ItemList[i]->pos.y, 30, 30);
    }
    for (int i = 0; i < (int)greenAreaList.size(); i++) {
        if (!greenAreaList[i]->active)
            continue;

        pWind->SetPen(DARKGREEN, 2);
        pWind->SetBrush(GREEN);
        pWind->DrawRectangle(greenAreaList[i]->x, greenAreaList[i]->y,
            greenAreaList[i]->x + 50, greenAreaList[i]->y + 40,
            FILLED);

        pWind->SetPen(BLACK, 1);
        pWind->SetFont(16, BOLD, BY_NAME, "Arial");
        pWind->DrawString(greenAreaList[i]->x + 15, greenAreaList[i]->y - 18,
            to_string(greenAreaList[i]->counter));
    }

    printBudget("BUDGET = $" + to_string(budget));
    writeStatus();
    pWind->UpdateBuffer();
}
// ==========================
// load and save
// ==========================

void Game::saveGame() const {
    ofstream file("savegame.txt");
    if (!file.is_open())
        return;
    file << "LEVEL " << level << "\n";
    file << "BUDGET " << budget << "\n";
    file << "TIMER " << timer << "\n";
    int animalCount = 0;
    for (int i = 0; i < (int)animalsList.size(); i++)
        if (!dynamic_cast<Wolf*>(animalsList[i]))
            animalCount++;

    file << "ANIMALS " << animalCount << "\n";
    for (int i = 0; i < (int)animalsList.size(); i++) {
        point p = animalsList[i]->getPosition();
        if (dynamic_cast<Chick*>(animalsList[i]))
            file << "CHICK " << p.x << " " << p.y << "\n";
        else if (dynamic_cast<Cow*>(animalsList[i]))
            file << "COW " << p.x << " " << p.y << "\n";
    }

    int wolfCount = 0;
    for (int i = 0; i < (int)animalsList.size(); i++)
        if (dynamic_cast<Wolf*>(animalsList[i]))
            wolfCount++;

    file << "WOLVES " << wolfCount << "\n";
    for (int i = 0; i < (int)animalsList.size(); i++) {
        if (dynamic_cast<Wolf*>(animalsList[i])) {
            point p = animalsList[i]->getPosition();
            file << "WOLF " << p.x << " " << p.y << "\n";
        }
    }

    int activeFood = 0;
    for (int i = 0; i < (int)greenAreaList.size(); i++)
        if (greenAreaList[i]->active)
            activeFood++;

    file << "FOODAREAS " << activeFood << "\n";
    for (int i = 0; i < (int)greenAreaList.size(); i++) {
        if (greenAreaList[i]->active)
            file << "FOOD " << greenAreaList[i]->x << " " << greenAreaList[i]->y
            << " " << greenAreaList[i]->counter << "\n";
    }

    file << "WAREHOUSE\n";
    file << "EGGS " << eggInWareHouse << "\n";
    file << "MILK " << milkInWareHouse << "\n";

    file.close();
    printMessage("Game saved successfully!");
}

void Game::loadGame() {
    ifstream file("savegame.txt");
    if (!file.is_open()) {
        printMessage("No save file found!");
        return;
    }

    for (int i = 0; i < (int)animalsList.size(); i++)
        delete animalsList[i];
    animalsList.clear();
    chickList.clear();
    cowList.clear();

    for (int i = 0; i < (int)greenAreaList.size(); i++)
        delete greenAreaList[i];
    greenAreaList.clear();

    for (int i = 0; i < (int)ItemList.size(); i++)
        delete ItemList[i];
    ItemList.clear();

    string token;

    file >> token >> level;
    file >> token >> budget;
    file >> token >> timer;

    int animalCount;
    file >> token >> animalCount;
    for (int i = 0; i < animalCount; i++) {
        string type;
        int x, y;
        file >> type >> x >> y;
        point p;
        p.x = x;
        p.y = y;
        if (type == "CHICK") {
            Chick* c = new Chick(this, p, 50, 50, "images/chick.jpg");
            animalsList.push_back(c);
            chickList.push_back(c);
        }
        else if (type == "COW") {
            Cow* c = new Cow(this, p, 60, 60, "images/cow.jpg");
            animalsList.push_back(c);
            cowList.push_back(c);
        }
    }

    int wolfCount;
    file >> token >> wolfCount;
    for (int i = 0; i < wolfCount; i++) {
        string type;
        int x, y;
        file >> type >> x >> y;
        point p;
        p.x = x;
        p.y = y;
        Wolf* w = new Wolf(this, p, 70, 70, "images/wolff.jpg");
        animalsList.push_back(w);
    }

    int foodCount;
    file >> token >> foodCount;
    for (int i = 0; i < foodCount; i++) {
        string type;
        int x, y, counter;
        file >> type >> x >> y >> counter;
        GreenArea* g = new GreenArea();
        g->x = x;
        g->y = y;
        g->counter = counter;
        g->active = true;
        g->eatTimer = 0;
        greenAreaList.push_back(g);
    }

    file >> token;
    file >> token >> eggInWareHouse;
    file >> token >> milkInWareHouse;

    file.close();

    lasttime = time(0);
    lastWolfSpawnTime = time(0);

    redrawScene();
    printMessage("Game loaded successfully!");
}
void Game::checkWolfAnimalCollision()
{
    // Collect prey to delete AFTER iterating, to avoid iterator invalidation.
    vector<Animal*> toRemove;

    for (int i = 0; i < (int)animalsList.size(); i++)
    {
        Wolf* wolf = dynamic_cast<Wolf*>(animalsList[i]);
        if (!wolf) continue;

        point wolfPos = wolf->getPosition();

        // Check against every chick
        for (int j = 0; j < (int)chickList.size(); j++)
        {
            // Skip prey already marked for removal this frame
            if (find(toRemove.begin(), toRemove.end(), chickList[j]) != toRemove.end())
                continue;

            point preyPos = chickList[j]->getPosition();
            // Overlap test: bounding boxes of wolf (70x70) and chick (50x50)
            if (abs(wolfPos.x - preyPos.x) < 60 &&
                abs(wolfPos.y - preyPos.y) < 60)
            {
                toRemove.push_back(chickList[j]);
                printMessage("A wolf ate your chicken!");
            }
        }

        // Check against every cow
        for (int j = 0; j < (int)cowList.size(); j++)
        {
            if (find(toRemove.begin(), toRemove.end(), cowList[j]) != toRemove.end())
                continue;

            point preyPos = cowList[j]->getPosition();
            // Overlap test: bounding boxes of wolf (70x70) and cow (60x60)
            if (abs(wolfPos.x - preyPos.x) < 65 &&
                abs(wolfPos.y - preyPos.y) < 65)
            {
                toRemove.push_back(cowList[j]);
                printMessage("A wolf ate your cow!");
            }
        }
    }

    // Now safely remove all marked prey from every list and free memory
    for (int i = 0; i < (int)toRemove.size(); i++)
    {
        Animal* dead = toRemove[i];

        // Remove from chickList and update counter
        size_t chickBefore = chickList.size();
        chickList.erase(
            remove(chickList.begin(), chickList.end(), dead), chickList.end());
        if (chickList.size() < chickBefore) numchick--;

        // Remove from cowList and update counter
        size_t cowBefore = cowList.size();
        cowList.erase(
            remove(cowList.begin(), cowList.end(), dead), cowList.end());
        if (cowList.size() < cowBefore) numcow--;

        // Remove from master animalsList
        animalsList.erase(
            remove(animalsList.begin(), animalsList.end(), dead), animalsList.end());

        delete dead;
    }

    if (!toRemove.empty())
    {
        redrawScene();
        Sleep(500);
    }
}
// ==========================
// Audio Control
// ==========================

void Game::pauseMusic() {
    if (audioReady)
        ma_sound_stop(&bgMusic);
}

void Game::resumeMusic() {
    if (audioReady)
        ma_sound_start(&bgMusic);
}

void Game::playSfx(const char* path) {
    if (audioReady)
        ma_engine_play_sound(&audioEngine, path, NULL);
}

// ==========================
// Main Loop
// ==========================

void Game::go() {
    int x, y;
    bool isExit = false;

    pWind->ChangeTitle(
        "- - - - - - - - - - Farm Frenzy (CIE101-project) - - - - - - - - - -");

    loadLeaderboard();
    promptUsername();
    gameStarted = true;
    lasttime = time(0);

    do {
        if (!isPaused) {
            updateTimer();

            for (int i = 0; i < (int)animalsList.size(); i++) {
                animalsList[i]->moveStep();
            }

            Wolfadd();
            eggadd();
            milkadd();
            checkAnimalGrassCollision();
            checkWolfAnimalCollision();
        }
        else {
            lasttime = time(0);
        }

        redrawScene();

        clicktype ct = pWind->GetMouseClick(x, y);
        if (ct != NO_CLICK) {
            if (isPaused) {
                if (y >= 0 && y < config.toolBarHeight) {
                    isExit = gameToolbar->handleClick(x, y);
                }
            }
            else {
                collectItems(x, y);

                // Wolf click: decrement health, remove when health reaches 0
                for (int i = 0; i < (int)animalsList.size(); i++)
                {
                    Wolf* wolf = dynamic_cast<Wolf*>(animalsList[i]);
                    if (!wolf) continue;
                    point wp = wolf->getPosition();
                    if (x >= wp.x && x <= wp.x + 70 && y >= wp.y && y <= wp.y + 70)
                    {
                        wolf->health--;
                        if (wolf->health <= 0)
                        {
                            animalsList.erase(animalsList.begin() + i);
                            delete wolf;
                            printMessage("Wolf defeated!");
                            redrawScene();
                        }
                        break; // one wolf hit per click
                    }
                }

                if (isWarehouseClicked(x, y)) {
                    openWarehouseWindow();
                }

                if (y >= 0 && y < config.toolBarHeight) {
                    isExit = gameToolbar->handleClick(x, y);
                }
                else if (y >= config.toolBarHeight && y < 2 * config.toolBarHeight) {
                    isExit = gameBudgetbar->handleClick(x, y);
                }
            }
        }

        Sleep(config.gameLoopDelayMs);

    } while (!isExit);
}