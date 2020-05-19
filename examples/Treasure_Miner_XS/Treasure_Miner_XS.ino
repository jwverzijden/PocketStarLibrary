/*
    TREASURE MINER XS
    This is a simplified version of Treasure Miner, only containing the base game and a very simple menu.
    It should show how the core elements of a PocketStar game are done, like the game loop, updating the screen
    or saving a high score to the SD.
    The following features contained in Treasure Miner are changed in this version:
        - the various menus are replaced by a single one showing only the title, a minecart image and the best score
          the player reached
        - only the very best score is saved to the SD card instead of the best five scores. the filename is also changed,
          so the games do not interfere with each other
        - the star, stone, nugget and heart objects were removed from the game. all other objects have the same
          probability to spawn (20%)
        - this means that the minecart can no longer be slowed or sped up anymore, so these effects were also removed 
          from the source code
        - the hearts in the top-left corner do not blink anymore if a life was lost or earned
    
    If you want to test it, compile and upload it!
    You can also try to change some variables and upload it again to see the result.
    
    (The separate .c/.cpp and .h files always confuse me, so I don't follow that system and put everything
    in the .h files - I hope this does not cause much confusion.)
    
    written by Florian Keller
    last changed 25 Mar 19
 */

/*
 * I usually make several files to keep the code (more or less) clear.
 * This is the core file with the menu
 */

// include PocketStar library and other files
#include <PocketStar.h>
#include "Config.h"
#include "Images.h"
#include "Game.h"
#include "Save.h"

// boolean indicating wether we are in menu or in game
boolean ingame;

// global timer for vibration; if >0, the PS should vibrate, and at <=0 not
float vibrationTimer = 0;

void initialize() {
    // init SD (in Save.h) and menu
    initSD();
    ingame = false;
    initMenu();
}

void update(float tpf) {
    // handle vibration
    if (vibrationTimer > 0) {
        vibrationTimer -= tpf;
        if (vibrationTimer < 0)
            pocketstar.vibrate(false);
    }
    
    if (ingame) {
        // gameOver is located in Game.h
        if (gameOver) {
            // leave the game, saving a possible highscore
            saveScore(score);
            ingame = false;
            initMenu();
        } else {
            // game still in progress - call its update method
            updateGame(tpf);
        }
    } else {
        if (pocketstar.getButtons(ButtonA)) {
            // start game
            initializeGame();
            ingame = true;
        }
        // no need to update the screen in the menu - we drew it at initMenu()
    }
}

void initMenu() {
    // draw title image
    pocketstar.drawImage(0, 0, &title);

    // print best score
    pocketstar.setFont(pocketStar5pt);
    pocketstar.setFontColor(DARKGRAY_8b, BLACK_8b);
    char strbuf[10];
    sprintf(strbuf, "%07.7i", highscore);
    pocketstar.setCursor(48 - (pocketstar.getPrintWidth(strbuf) + pocketstar.getPrintWidth("Best Score: "))/2, 9);
    pocketstar.print("Best Score: ");
    pocketstar.print(strbuf);
}

boolean pause() {
    // always allow pause menu
    return true;
}

void resume() {
    if (!ingame) {
        // draw menu again so pause menu isn't visible anymore
        initMenu();
    } else {
        // in Game.h
        resumeGame();
    }
}
