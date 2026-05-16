#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <cstdlib>

#include "./Core/Game.h"

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    {
        
        Game game;
        game.go();
    }

    return 0;
}