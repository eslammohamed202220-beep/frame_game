# Farm Frenzy

A Windows desktop farm game built in C++ with CMUgraphics. Manage chickens and cows, collect eggs and milk, sell produce, and protect the farm from wolves while working toward each level's budget goal.

## Gameplay

- Buy animals and manage a limited budget.
- Collect produce and sell it from the warehouse.
- Keep animals fed and click wolves to defend the farm.
- Reach the budget goal before the timer runs out to advance.
- Save and load a game, and track scores on a local leaderboard.

## Run locally

1. On Windows, open `graphics_prj.sln` in Visual Studio with the C++ desktop workload and Windows SDK installed.
2. If Visual Studio asks to retarget the project, select a toolset installed on your machine. The checked-in project currently targets `v145`.
3. Build and run the application with the repository root as the working directory so it can find `images/` and `sounds/`.

The project uses the included CMUgraphics sources and `miniaudio.h`. It has no network or account requirement. Save data and leaderboard scores are written to local text files.

## Project layout

| Path | Purpose |
| --- | --- |
| `Core/` | Game loop and shared game objects |
| `Entities/` | Animals and their behavior |
| `UI/` | Toolbar and budget controls |
| `Config/` | Window and game configuration |
| `images/`, `sounds/` | Local game assets |
| `CMUgraphicsLib/` | Bundled graphics library |

`Readme.txt` and `Manual.txt` document the bundled CMUgraphics package.
