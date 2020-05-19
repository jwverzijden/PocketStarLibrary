/*
 * Everything that has to do with the actual game is inside the Game.h
 */

// types of falling objects.
// since lists of variable size are not easy to use on microcontrollers, we use an array instead
// thus, we need to mark the empty spots of the array. this is done with the 'unused' value
const uint8_t RUBY      = 0;
const uint8_t SAPPHIRE  = 1;
const uint8_t EMERALD   = 2;
const uint8_t DIAMOND   = 3;
const uint8_t BOMB      = 4;
const uint8_t UNUSED    = 255;

// structure for falling objects
struct Object {
    float x;
    float y;
    uint8_t type;
};

// variables for horizontal cart position, current score and remaining lives
float cartX;
uint32_t score;
uint8_t lives;

// object array ('list') - 16 should be long enough for busy cases
Object objects[16];

// save total game time and time until next object spawns
float gameTime;
float timeUntilNextSpawn;

// is the game over?
boolean gameOver;

// vibration timer is in the menu file, but we can't access it from this file, thus we need to declare it here as extern
extern float vibrationTimer;

// function headers
void updateObjects(float tpf);
void draw();

// each time the game is started, this function is called
void initializeGame() {
    // set random seed. on a PC, this is usually done automatically based on the current time.
    // the PocketStar clock resets every time a game or app is loaded, thus we cannot set it when the game is
    // loaded - it would be always the same value since it always takes the same time to start the game.
    // the PocketStar library sets the seed based on the current battery level (since this is one of the few more-or-less
    // unpredictable values), but this doesn't give many different results (~140), too, so we set the seed based on the only
    // really random value: the time at which the player starts the game. if the player starts it after between 0 and 1
    // seconds, we have 1 million different random values here - seems enough.
    srand(micros());

    // init the list to be 'empty'
    for (int i = 0; i < 16; i++) {
        objects[i].type = UNUSED;
    }
    // set cart to the middle of the screen. cartX is at the left end of the minecart image, so subtract the half of its width here
    cartX = 48 - cart.width/2;

    // set other variables to init values
    score = 0;
    lives = 3;
    gameTime = 0;
    timeUntilNextSpawn = 0;
    gameOver = false;

    // draw the top of the screen in gray (the score and lives will be shown on top of this rectangle)
    // the other parts are drawn continuously, so we don't have to draw them now
    // if we would redraw the rectangle every frame and print the score and lives on top of it, the screen will flicker
    // because it rapidly changes between the rectangle color and the text color
    pocketstar.drawRect(0, 0, 96, 9, true, GRAY_8b);
}

// update function: the core function. chacks user input, moves everything and draws the current state
void updateGame(float tpf) {
    // don't do anything if the game is already over
    if (gameOver)
        return;

    // increase game time
    gameTime += tpf;

    // check player input and move the minecart accordingly, but don't let it exit the screen
    if (pocketstar.getButtons(ButtonLeft)) {
        cartX -= CART_VELOCITY(gameTime) * tpf;
        if (cartX < 0)
            cartX = 0;
    }
    if (pocketstar.getButtons(ButtonRight)) {
        cartX += CART_VELOCITY(gameTime) * tpf;
        if (cartX + cart.width > 96)
            cartX = 96 - cart.width;
    }

    // call other functions
    updateObjects(tpf);
    draw();
}

// helper function for vibration
void startVibration(float time) {
    if (vibrationTimer < time)
        vibrationTimer = time;
    pocketstar.vibrate(true);
}

void updateObjects(float tpf) {
    timeUntilNextSpawn -= tpf;

    // check if a new object should spawn
    if (timeUntilNextSpawn < 0) {
        // search for the first free spot in the array
        int firstFree;
        for (firstFree = 0; firstFree < 16; firstFree++) {
            if (objects[firstFree].type == UNUSED)
                break;
        }
        // spawn the object only if the array isn't full. while the array should be long enough for busy cases, you can never know it for sure
        // so better check for it. if it is full, it spawns as soon as a spot gets free
        if (firstFree < 16) {
            // set time until next object spawns, on a random value between minimum and maximum time between spawns
            float random = (rand() % 1000) / 1000.f;
            timeUntilNextSpawn = random * (TIME_BETWEEN_OBJECT_SPAWN_MAX - TIME_BETWEEN_OBJECT_SPAWN_MIN) + TIME_BETWEEN_OBJECT_SPAWN_MIN;

            // set type and horizontal position to random values, and vertical position to just a bit above the screen 
            objects[firstFree].type = rand() % 5;
            objects[firstFree].x = rand() % 88 + 4;
            objects[firstFree].y = -5;
        }
    }

    // update all objects
    for (int i = 0; i < 16; i++) {
        if (objects[i].type != UNUSED) {
            // move it down
            objects[i].y += FALL_SPEED(gameTime) * tpf;

            // did the object fall into the minecart? if so, change score or lives and remove the object from the list
            if (objects[i].y > 64 - cart.height && objects[i].y < 64 - cart.height + 2
                    && cartX < objects[i].x && objects[i].x < cartX + cart.width) {

                // start vibrating
                // if a bomb was collected, the vibration time is set again afterwards to the higher bomb vibration time
                startVibration(OBJECT_VIBRATION);
                
                switch (objects[i].type) {
                    // gem: increase score
                    case RUBY:      score += RUBY_POINTS;       break;
                    case SAPPHIRE:  score += SAPPHIRE_POINTS;   break;
                    case EMERALD:   score += EMERALD_POINTS;    break;
                    case DIAMOND:   score += DIAMOND_POINTS;    break;
                    // bomb: decrease lives and set vibration
                    case BOMB:      lives -= 1;
                                    startVibration(BOMB_VIBRATION);
                                    break;
                }
                
                objects[i].type = UNUSED;
            }
            // did it fall on the ground? remove it from the list
            // this causes the object to disappear suddenly, but it looks better than objects falling through the floor
            if (objects[i].y > 70) {
                objects[i].type = UNUSED;
            }
        }
    }
    // check if the player ran out of lives
    if (lives <= 0)
        gameOver = true;
}

void draw() {
    // update the object sprites
    // this game uses the drawSprites() function provided by the library, have a look at the documentation for information about it
    for (int i = 0; i < 16; i++) {
        // don't draw objects that do not exist
        if (objects[i].type == UNUSED) {
            objectSprites[i].enabled = false;
            continue;
        }
        objectSprites[i].enabled = true;
        
        // set image according to object type
        switch (objects[i].type) {
            case RUBY:      objectSprites[i].image = &ruby;     break;
            case SAPPHIRE:  objectSprites[i].image = &sapphire; break;
            case EMERALD:   objectSprites[i].image = &emerald;  break;
            case DIAMOND:   objectSprites[i].image = &diamond;  break;
            case BOMB:      objectSprites[i].image = &bomb;     break;
        }
        
        // set sprite location
        objectSprites[i].x = objects[i].x - objectSprites[i].image->width/2;
        objectSprites[i].y = objects[i].y - objectSprites[i].image->height/2;
    }
    // update minecart sprite X location
    // image and Y location never change, so no need to update them
    cartSprite.x = cartX;

    // let the library draw the sprites, on a black background
    // the first 9 rows show the score and lives, so skip them here. we will update them separately
    pocketstar.drawSprites(spriteList, 17, BLACK_8b, 9);

    // print score at the top right
    // it is drawn on top of the score-text printed in the last frame, which therefore isn't visible anymore
    char strbuf[10];
    sprintf(strbuf, "%07.7i", score);
    pocketstar.setFontColor(BLACK_8b, GRAY_8b);
    pocketstar.setFont(pocketStar6pt);
    pocketstar.setCursor(95 - pocketstar.getPrintWidth(strbuf), 0);
    pocketstar.print(strbuf);

    // draw remaining lives as red hearts. the 'N' character in the PocketStar symbol font represents a heart
    // have a look at the documentation for further information about the symbol font
    pocketstar.setFontColor(RED_8b, GRAY_8b);
    pocketstar.setFont(pocketStarSymbols);
    pocketstar.setCursor(0, 0);
    for (int i = 0; i < lives; i++) {
        pocketstar.print("N");
    }
    // now let's say we had three lives in the last frame and printed three red hearts, and just lost a life,
    // so we printed two red hearts - the third red heart would still be visible, since this area of the screen
    // didn't get an update.
    // so, we print the lost lives as gray hearts with gray background here, essentially hiding the red hearts
    pocketstar.setFontColor(GRAY_8b, GRAY_8b);
    for (int i = lives; i < 3; i++) {
        pocketstar.print("N");
    }
}

void resumeGame() {
    // we only have to redraw the rectangle behind the score and lives here since it is the only thing which is
    // not drawn every frame
    pocketstar.drawRect(0, 0, 96, 9, true, GRAY_8b);
}

