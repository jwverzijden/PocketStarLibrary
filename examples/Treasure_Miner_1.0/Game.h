/*
 * Everything that has to do with the actual game is inside the Game.h
 */

// types of falling objects.
// since lists of variable size are not easy to use on microcontrollers, we use an array instead
// thus, we need to mark the empty spots of the array. this is done with the 'unused' value
const uint8_t NUGGET    = 0;
const uint8_t RUBY      = 1;
const uint8_t SAPPHIRE  = 2;
const uint8_t EMERALD   = 3;
const uint8_t DIAMOND   = 4;
const uint8_t STONE     = 5;
const uint8_t BOMB      = 6;
const uint8_t STAR      = 7;
const uint8_t HEART     = 8;
const uint8_t UNUSED    = 255;

const uint8_t EFFECT_NONE   = 0;
const uint8_t EFFECT_SLOW   = 1;
const uint8_t EFFECT_STAR   = 2;

// structure for falling objects
struct Object {
    float x;
    float y;
    uint8_t type;
};

// variables for horizontal cart position, current score and remaining lives, and current effect
float cartX;
uint32_t score;
uint8_t lives;
uint8_t currentEffect;
float remainingEffectTime;

// object array ('list') - 16 should be long enough for busy cases
Object objects[16];

// save total game time and time until next object spawns
float gameTime;
float timeUntilNextSpawn;

// these variables are used to prevent spawning multiple hearts / stars in quick succession
float lastHeartSpawn;
float lastStarSpawn;

// is the game over?
boolean gameOver;

// vibration timer is in the menu file, but we can't access it from this file, thus we need to declare it here as extern
extern float vibrationTimer;

// stats: count collected gems & nuggets
uint8_t gemsCollected[5];

// timer for blinking hearts (in the top-left corner) when a life was earned or lost
float heartBlinkTime;

// function headers
uint8_t randomObject();
void startVibration(float time);
void updateObjects(float tpf);
void draw(float tpf);

// each time the game is started, this function is called
void initializeGame() {
    // set random seed. on a PC, this is usually done automatically based on the current time.
    // the PocketStar clock resets every time a game or app is loaded, thus we cannot set it when the game is
    // loaded - it would be always the same value since it always takes the same time to start the game.
    // the PocketStar library sets the seed based on the current battery level (since this is one of the few unpredictable
    // values), but this doesn't give many different results (about 140), so we set the seed based on the only
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
    currentEffect = EFFECT_NONE;
    score = 0;
    lives = 3;
    gameTime = 0;
    timeUntilNextSpawn = 0;
    lastHeartSpawn = lastStarSpawn = 0;
    gameOver = false;
    heartBlinkTime = 0;

    // init stats - set all to zero
    for (int i = 0; i < 5; i++) {
        gemsCollected[i] = 0;
    }

    // draw the top of the screen in gray (the score and lives will be shown on top of this rectangle).
    // the other parts are drawn in every frame, so we don't have to draw them now.
    // if we would redraw the rectangle every frame and print the score and lives on top of it, the screen would flicker
    // because it rapidly changes between the rectangle color and the text color
    pocketstar.drawRect(0, 0, 96, 9, true, GRAY_8b);
}

// update function: the core game loop. checks user input, moves everything and redraws the screen
void updateGame(float tpf) {
    // don't do anything if the game is already over
    if (gameOver)
        return;

    // increase game time
    gameTime += tpf;

    // increase time since last heart / star spawn
    lastHeartSpawn += tpf;
    lastStarSpawn += tpf;

    // check for current effect, and if it should end
    if (currentEffect != EFFECT_NONE) {
        remainingEffectTime -= tpf;
        if (remainingEffectTime < 0)
            currentEffect = EFFECT_NONE;
    }

    // get current velocity
    float cartVelocity = CART_VELOCITY(gameTime);
    if (currentEffect == EFFECT_SLOW)
        cartVelocity = CART_VELOCITY_SLOW(gameTime);
    if (currentEffect == EFFECT_STAR)
        cartVelocity = CART_VELOCITY_STAR(gameTime);
    
    // check player input and move the minecart accordingly, but don't let it exit the screen
    if (pocketstar.getButtons(ButtonLeft)) {
        cartX -= cartVelocity * tpf;
        if (cartX < 0)
            cartX = 0;
    }
    if (pocketstar.getButtons(ButtonRight)) {
        cartX += cartVelocity * tpf;
        if (cartX + cart.width > 96)
            cartX = 96 - cart.width;
    }

    // call other functions
    updateObjects(tpf);
    draw(tpf);
}

// helper function for vibration
void startVibration(float time) {
    if (vibrationTimer < time)
        vibrationTimer = time;
    pocketstar.vibrate(true);
}

// returns a random object
uint8_t randomObject() {
    int sumOfAll = NUGGET_PROBABILITY + RUBY_PROBABILITY + SAPPHIRE_PROBABILITY + EMERALD_PROBABILITY + 
            DIAMOND_PROBABILITY + STONE_PROBABILITY + BOMB_PROBABILITY;
    
    // stars and hearts can only spawn under certain conditions
    boolean canStarSpawn = lastStarSpawn > 3;
    boolean canHeartSpawn = lives < 5 && lastHeartSpawn > 3;
    if (canStarSpawn)
        sumOfAll += STAR_PROBABILITY;
    if (canHeartSpawn)
        sumOfAll += HEART_PROBABILITY;
    
    float type = rand() % sumOfAll;
    
    if (type < NUGGET_PROBABILITY)
        return NUGGET;
    type -= NUGGET_PROBABILITY;
    
    if (type < RUBY_PROBABILITY)
        return RUBY;
    type -= RUBY_PROBABILITY;
    
    if (type < SAPPHIRE_PROBABILITY)
        return SAPPHIRE;
    type -= SAPPHIRE_PROBABILITY;
    
    if (type < EMERALD_PROBABILITY)
        return EMERALD;
    type -= EMERALD_PROBABILITY;
    
    if (type < DIAMOND_PROBABILITY)
        return DIAMOND;
    type -= DIAMOND_PROBABILITY;
    
    if (canStarSpawn) {
        if (type < STAR_PROBABILITY)
            return STAR;
        type -= STAR_PROBABILITY;
    }

    if (canHeartSpawn) {
        if (type < HEART_PROBABILITY)
            return HEART;
        type -= HEART_PROBABILITY;
    }
    
    if (type < STONE_PROBABILITY)
        return STONE;
    type -= STONE_PROBABILITY;
    
    if (type < BOMB_PROBABILITY)
        return BOMB;

    // getting here should be impossible
    return NUGGET;
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
        // spawn the object only if the array isn't full. while the array should be long enough for busy cases, we can never know it for sure
        // so better check for it. if the array is full, the new object spawns as soon as a spot is free
        if (firstFree < 16) {
            // set time until next object spawns, on a random value between minimum and maximum time between spawns
            float random = (rand() % 1000) / 1000.f;
            timeUntilNextSpawn = random * (TIME_BETWEEN_OBJECT_SPAWN_MAX - TIME_BETWEEN_OBJECT_SPAWN_MIN) + TIME_BETWEEN_OBJECT_SPAWN_MIN;

            // set type and horizontal position to random values, and vertical position to just a bit above the screen
            uint8_t type = randomObject();
            if (type == HEART)
                lastHeartSpawn = 0;
            if (type == STAR)
                lastStarSpawn = 0;
            objects[firstFree].type = type;
            objects[firstFree].x = rand() % 88 + 4;
            objects[firstFree].y = -5;
        }
    }

    // update all objects
    for (int i = 0; i < 16; i++) {
        if (objects[i].type != UNUSED) {
            // move it down
            objects[i].y += FALL_SPEED(gameTime) * tpf;

            // did the object fall into the minecart? if so, change score or lives or activate their effect and remove the object from the list
            if (objects[i].y > 64 - cart.height && objects[i].y < 64 - cart.height + 2
                    && cartX < objects[i].x && objects[i].x < cartX + cart.width) {
                startVibration(OBJECT_VIBRATION);
                switch (objects[i].type) {
                    case NUGGET:    score += NUGGET_POINTS;     break;
                    case RUBY:      score += RUBY_POINTS;       break;
                    case SAPPHIRE:  score += SAPPHIRE_POINTS;   break;
                    case EMERALD:   score += EMERALD_POINTS;    break;
                    case DIAMOND:   score += DIAMOND_POINTS;    break;
                    case HEART:     if (lives < 5) {
                                        lives += 1;
                                        heartBlinkTime = 0.9f;
                                    }
                                    break;
                    case BOMB:      lives -= 1;
                                    heartBlinkTime = 0.9f;
                                    startVibration(BOMB_VIBRATION);
                                    break;
                    case STONE:     if (currentEffect == EFFECT_STAR) {
                                        currentEffect = EFFECT_NONE;
                                    } else {
                                        currentEffect = EFFECT_SLOW;
                                        remainingEffectTime = EFFECT_SLOW_TIME;
                                    }
                                    break;
                    case STAR:      if (currentEffect == EFFECT_SLOW) {
                                        currentEffect = EFFECT_NONE;
                                    } else {
                                        currentEffect = EFFECT_STAR;
                                        remainingEffectTime = EFFECT_SLOW_TIME;
                                    }
                                    break;
                }
                // increase object count if a gem or nugget was collected
                if (objects[i].type < 5)
                    gemsCollected[objects[i].type] += 1;

                // mark as unused
                objects[i].type = UNUSED;
                continue;
            }
            // did it fall on the ground? remove it from the list
            // this causes the object to disappear suddenly, but it looks better than objects falling through the floor
            if (objects[i].y > 70)
                objects[i].type = UNUSED;
        }
    }
    // check if the player ran out of lives
    if (lives <= 0)
        gameOver = true;
}

void draw(float tpf) {
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
            case NUGGET:    objectSprites[i].image = &nugget;   break;
            case RUBY:      objectSprites[i].image = &ruby;     break;
            case SAPPHIRE:  objectSprites[i].image = &sapphire; break;
            case EMERALD:   objectSprites[i].image = &emerald;  break;
            case DIAMOND:   objectSprites[i].image = &diamond;  break;
            case STONE:     objectSprites[i].image = &stone;    break;
            case BOMB:      objectSprites[i].image = &bomb;     break;
            case STAR:      if ((millis() % 300) < 150)
                                objectSprites[i].image = &star;
                            else
                                objectSprites[i].image = &star2;
                            break;
            case HEART:     if ((millis() % 300) < 150)
                                objectSprites[i].image = &heart;
                            else
                                objectSprites[i].image = &heart2;
                            break;
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
    pocketstar.drawSprites(spriteList, 17, BLACK_8b, 11);

    // draw the bar which indicates how long the current effect will last
    if (currentEffect == EFFECT_SLOW) {
        int x = (remainingEffectTime / EFFECT_SLOW_TIME) * 95;
        pocketstar.drawRect(0,     9, x,      2, true, DARKGRAY_8b);
        pocketstar.drawRect(x + 1, 9, 96 - x, 2, true, GRAY_8b);
    } else if (currentEffect == EFFECT_STAR) {
        int x = (remainingEffectTime / EFFECT_STAR_TIME) * 95;
        pocketstar.drawRect(0,     9, x,      2, true, ORANGE_8b);
        pocketstar.drawRect(x + 1, 9, 96 - x, 2, true, GRAY_8b);
    } else {
        pocketstar.drawRect(0,     9, 96,     2, true, GRAY_8b);
    }
    
    // print score at top right
    // it is drawn on top of the score-text printed in the last frame, which therefore isn't visible anymore
    char strbuf[10];
    sprintf(strbuf, "%07.7i", score);
    pocketstar.setFontColor(BLACK_8b, GRAY_8b);
    pocketstar.setFont(pocketStar6pt);
    pocketstar.setCursor(95 - pocketstar.getPrintWidth(strbuf), 0);
    pocketstar.print(strbuf);

    heartBlinkTime -= tpf;
    // blinking hearts: test if they are blinking, and if so, check if they are visible
    // here, they blink for one second (heartBlinkTime is set to 1.0 when losing / earning a life)
    // the second test checks if the hearts are visible in the current blink animation
    if (heartBlinkTime > 0 && (((int) (heartBlinkTime * 1000)) % 300) > 150) {
        // draw five gray hearts on gray background. the 'N' character in the PocketStar symbol font represents a heart
        // have a look at the documentation for further information about the symbol font
        pocketstar.setFontColor(GRAY_8b, GRAY_8b);
        pocketstar.setFont(pocketStarSymbols);
        pocketstar.setCursor(0, 0);
        for (int i = 0; i < 5; i++) {
            pocketstar.print("N"); // heart
        }
    } else {
        // draw remaining lives as red hearts
        pocketstar.setFontColor(RED_8b, GRAY_8b);
        pocketstar.setFont(pocketStarSymbols);
        pocketstar.setCursor(0, 0);
        for (int i = 0; i < lives; i++) {
            pocketstar.print("N"); // heart
        }
        // print the other possible lives as gray hearts with gray background here, essentially hiding the red hearts (which
        // may were drawn last frame, and would still be visible if we don't overdraw them
        pocketstar.setFontColor(GRAY_8b, GRAY_8b);
        for (int i = lives; i < 5; i++) {
            pocketstar.print("N"); // heart
        }
    }
}

void resumeGame() {
    // we only have to redraw the rectangle behind the score and lives here since it is the only thing which is
    // not drawn every frame
    pocketstar.drawRect(0, 0, 96, 9, true, GRAY_8b);
}

