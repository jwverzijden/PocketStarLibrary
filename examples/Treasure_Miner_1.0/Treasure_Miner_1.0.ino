/*
    TREASURE MINER
    This is the full source code of the game. It is fairly complicated; if you search for basic things, have a
    look at the 'Treasure Miner XS' example which contains a more basic version of the game, with only the core
    game and many fancy features missing.
    
    If you want to test it, compile and upload it!
    You can also try to change some variables and upload it again to see the result.
    
    (The separate .c/.cpp and .h files always confuse me, so I don't follow that system and put everything
    in the .h files - I hope this does not cause much confusion.)
    
    version 1.0
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

// all different states
enum {STATE_TITLE, STATE_MAIN_MENU, STATE_INGAME, STATE_GAME_OVER, STATE_HIGH_SCORES, STATE_ENTER_NAME, STATE_HELP, STATE_CREDITS};

// current state
uint8_t state;
// information for last button input; if a button was pressed, the menus will ignore any button input for a short time
// afterwards. if we would not do this, the state would change again in the next frame since the player likely still presses
// the button
long lastButtonInput;
// variables for various menus: current option in main menu
uint8_t option;
// data for 'enter name'
uint8_t curName[8] = {0, 0, 0};
uint8_t curNameEdit;
// number of newest high score (for blinking animation)
uint8_t newHighScoreNr;
// help page
uint8_t helpPage;

// global timer for vibration; if >0, the PS should vibrate, and at <=0 not
float vibrationTimer = 0;

// function headers
char toChar(uint8_t c);
boolean isButtonPressed(uint8_t button);
void initState(uint8_t newState);

void initialize() {
    // init SD (in Save.h) and menu
    initSD();
    initState(STATE_TITLE);
}

// helper function for input in the menus
boolean isButtonPressed(uint8_t button) {
    if (lastButtonInput + TIME_BETWEEN_INPUT < millis() && pocketstar.getButtons(button)) {
        lastButtonInput = millis();
        return true;
    }
    return false;
}

void update(float tpf) {
    // handle vibration
    if (vibrationTimer > 0) {
        vibrationTimer -= tpf;
        if (vibrationTimer < 0)
            pocketstar.vibrate(false);
    }

    switch (state) {
        case STATE_TITLE:
            // title: go to main menu if 'A' was pressed
            if (isButtonPressed(ButtonA))
                initState(STATE_MAIN_MENU);
            break;
        case STATE_MAIN_MENU: {
            // main menu: draw option text, draw the currently selected option in a different color than the others
            pocketstar.setFont(pocketStar6pt);
            char *strings[] = {"PLAY", "HIGH SCORES", "HELP", "CREDITS"};
            for (int i = 0; i < 4; i++) {
                if (option == i)
                    pocketstar.setFontColor(ORANGE_8b, BLACK_8b);
                else
                    pocketstar.setFontColor(DARKGRAY_8b, BLACK_8b);
                pocketstar.setCursor(48 - pocketstar.getPrintWidth(strings[i])/2, 24 + 8 * i);
                pocketstar.print(strings[i]);
            }

            // check buttons, change option or state
            if (option > 0 && isButtonPressed(ButtonUp))
                option -= 1;
            if (option < 3 && isButtonPressed(ButtonDown))
                option += 1;
            if (isButtonPressed(ButtonA)) {
                switch (option) {
                    case 0: initState(STATE_INGAME);        break;
                    case 1: initState(STATE_HIGH_SCORES);   break;
                    case 2: initState(STATE_HELP);          break;
                    case 3: initState(STATE_CREDITS);       break;
                }
            }
            break;
        }
        case STATE_INGAME:
            // ingame: pass on to updateGame (in Game.h) and check if game is over
            updateGame(tpf);
            if (gameOver)
                initState(STATE_GAME_OVER);
            break;
        case STATE_GAME_OVER:
            // game over: go to enter name or high scores if A was pressed
            if (isButtonPressed(ButtonA)) {
                if (score > highScores[4])
                    initState(STATE_ENTER_NAME);
                else
                    initState(STATE_HIGH_SCORES);
            }
            break;
        case STATE_HIGH_SCORES: {
            // high scores: draw all high scores
            pocketstar.setFont(pocketStar6pt);
            uint8_t colors[] = {ORANGE_8b, RED_8b, BLUE_8b, GREEN_8b, CYAN_8b};
            char strbuf[16];
            for (int i = 0; i < 5; i++) {
                pocketstar.setFontColor(colors[4 - i], BLACK_8b);
                uint8_t y = 16 + i * 8;
                if (newHighScoreNr == i && millis() % 200 < 100) {
                    pocketstar.drawRect(0, y, 96, 8, true, BLACK_16b);
                } else {
                    sprintf(strbuf, "%i.", i + 1);
                    pocketstar.setCursor(18 - pocketstar.getPrintWidth(strbuf), y);
                    pocketstar.print(strbuf);
                    pocketstar.print(highScoresNames[i]);
                    sprintf(strbuf, "%0.6i", highScores[i]);
                    pocketstar.setCursor(84 - pocketstar.getPrintWidth(strbuf), y);
                    pocketstar.print(strbuf);
                }
            }
            // go back to main menu if B is pressed
            if (isButtonPressed(ButtonB))
                initState(STATE_MAIN_MENU);
            break;
        }
        case STATE_ENTER_NAME: {
            // draw name
            pocketstar.setFont(pocketStar7pt);
            uint8_t colors[] = {ORANGE_8b, RED_8b, BLUE_8b, GREEN_8b, CYAN_8b};
            for (int i = 0; i < 3; i++) {
                uint8_t x = 38 + i * 9;
                uint8_t y = 34;
                pocketstar.setFontColor(colors[curName[i] % 5], BLACK_8b);
                
                // I an W are special cases since they have a different width
                if (toChar(curName[i]) == 'I') {
                    pocketstar.setCursor(x - 3, y);
                    pocketstar.print(" I ");
                } else if (toChar(curName[i]) == 'W') {
                    pocketstar.setCursor(x - 1, y);
                    pocketstar.print("W");
                } else {
                    pocketstar.setCursor(x, y);
                    pocketstar.print(toChar(curName[i]));
                }
                if (i == curNameEdit) {
                    pocketstar.drawImage(x + 1, y - 4, &arrowUp);
                    pocketstar.drawImage(x + 1, y + 10, &arrowDown);
                } else {
                    pocketstar.drawRect(x + 1, y - 4, 5, 3, true, BLACK_8b);
                    pocketstar.drawRect(x + 1, y + 10, 5, 3, true, BLACK_8b);
                }
            }
            // change current edited letter
            if (curNameEdit > 0 && isButtonPressed(ButtonLeft))
                curNameEdit--;
            if (curNameEdit < 2 && isButtonPressed(ButtonRight))
                curNameEdit++;

            // change letter
            if (isButtonPressed(ButtonDown))
                curName[curNameEdit] = (curName[curNameEdit] + 1) % 37;
            if (isButtonPressed(ButtonUp))
                curName[curNameEdit] = (curName[curNameEdit] + 36) % 37;

            if (isButtonPressed(ButtonA)) {
                if (curNameEdit == 2) {
                    // save new high score to SD
                    char buffer[4];
                    buffer[0] = toChar(curName[0]);
                    buffer[1] = toChar(curName[1]);
                    buffer[2] = toChar(curName[2]);
                    buffer[3] = '\0';
                    newHighScoreNr = addScore(score, buffer);
                    initState(STATE_HIGH_SCORES);
                } else {
                    curNameEdit++;
                }
            }
            break;
        }
        case STATE_HELP:
            // help: go back to main menu if B is pressed, change page if L or R is pressed
            if (helpPage > 0 && isButtonPressed(ButtonLeft)) {
                helpPage--;
                initState(STATE_HELP);
            }
            if (helpPage < 2 && isButtonPressed(ButtonRight)) {
                helpPage++;
                initState(STATE_HELP);
            }
            if (isButtonPressed(ButtonB))
                initState(STATE_MAIN_MENU);
            break;
        case STATE_CREDITS:
            // credits: go back to main menu if B is pressed
            if (isButtonPressed(ButtonB))
                initState(STATE_MAIN_MENU);
            break;
    }
}

// initializes the state: sets variables, draws images or text to screen
void initState(uint8_t newState) {
    switch (newState) {
        case STATE_TITLE:
            // draw title image
            pocketstar.drawImage(0, 0, &title);
            break;
        case STATE_MAIN_MENU:
            // draw title text; options text is drawn in every update
            pocketstar.drawRect(0, 0, 96, 64, true, BLACK_8b);
            pocketstar.drawImage(0, 0, &titleText);
            break;
        case STATE_INGAME:
            // init game
            initializeGame();
            break;
        case STATE_GAME_OVER: {
            // draw 'game over' text
            pocketstar.drawRect(0, 0, 96, 64, true, BLACK_8b);
            pocketstar.drawImage(0, 0, &gameOverText);

            // draw gem image, how often they were collected and how many points where scored with each gem
            pocketstar.drawImage(12, 11, &gameOverGems);
            pocketstar.setFont(pocketStar5pt);
            uint8_t colors[] = {ORANGE_8b, RED_8b, BLUE_8b, GREEN_8b, CYAN_8b};
            uint16_t points[] = {NUGGET_POINTS, RUBY_POINTS, SAPPHIRE_POINTS, EMERALD_POINTS, DIAMOND_POINTS};
            char strbuf[32];
            for (int i = 0; i < 5; i++) {
                pocketstar.setFontColor(colors[i], BLACK_8b);
                int y = 11 + 9 * i;
                pocketstar.setCursor(25, y);
                sprintf(strbuf, "x %0.3i = %0.5i", gemsCollected[i], points[i] * gemsCollected[i]);
                pocketstar.print(strbuf);
            }

            // draw line
            pocketstar.drawLine(10, 55, 82, 55, DARKGRAY_8b);
            // total score
            pocketstar.setFontColor(DARKGRAY_8b, BLACK_8b);
            pocketstar.setCursor(17, 57);
            pocketstar.print("TOTAL:");
            pocketstar.setCursor(56, 57);
            sprintf(strbuf, "%0.6i", score);
            pocketstar.print(strbuf);
            break;
        }
        case STATE_HIGH_SCORES:
            // draw 'high scores' text; the scores and names are printed in every update
            pocketstar.drawRect(0, 0, 96, 64, true, BLACK_8b);
            pocketstar.drawImage(0, 0, &highScoresText);
            
            // if we came from main menu or from game over, do not show a blinking score
            if (state == STATE_MAIN_MENU || state == STATE_GAME_OVER)
                newHighScoreNr = 255;
            break;
        case STATE_ENTER_NAME:
            // draw 'enter name' text
            pocketstar.drawRect(0, 0, 96, 64, true, BLACK_8b);
            pocketstar.drawImage(0, 0, &enterNameText);
            break;
        case STATE_HELP:
            // the help pages are realised as full-screen images
            if (helpPage == 0)
                pocketstar.drawImage(0, 0, &help1);
            else if (helpPage == 1)
                pocketstar.drawImage(0, 0, &help2);
            else
                pocketstar.drawImage(0, 0, &help3);
            break;
        case STATE_CREDITS:
            // draw 'credits' text
            pocketstar.drawRect(0, 0, 96, 64, true, BLACK_8b);
            pocketstar.drawImage(0, 0, &creditsText);
            
            pocketstar.setFont(pocketStar6pt);

            uint8_t colors[] = {ORANGE_8b, RED_8b, BLUE_8b, GREEN_8b, CYAN_8b};
            char *strings[] = {"Treasure Miner", VERSION, "by", "Florian Keller"};
            for (int i = 0; i < 4; i++) {
                pocketstar.setFontColor(colors[i], BLACK_8b);
                pocketstar.setCursor(48 - pocketstar.getPrintWidth(strings[i])/2, 14 + 9 * i);
                pocketstar.print(strings[i]);
            }
            break;
    }
    state = newState;
}
// a small helper function for the 'enter name' screen. internally, the name is represented as
// integers for simple editing. this function converts the internal integer to the corresponding
// character, used for displaying the chars and for saving them to high scores
char toChar(uint8_t c) {
    if (c < 26) // 0 - 25 = A - Z
        return 'A' + c;
    else if (c < 36) // 26 - 35 = 0 - 9
        return '0' + c - 26;
    else if (c == 36) // 36 = _
        return '_';
    else
        return '?';
}

boolean pause() {
    // always allow pause menu
    return true;
}

void resume() {
    if (state != STATE_INGAME) {
        // init current state again so pause menu isn't visible anymore
        initState(state);
    } else {
        // in Game.h
        resumeGame();
    }
}
