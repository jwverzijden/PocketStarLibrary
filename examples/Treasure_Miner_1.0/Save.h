/*
 * The Save.h file provides functions for saving and loading the best score
 * 
 * the top five scores are saved, each as 32 bit integer, and with a name of three characters per score
 * 
 * Have a look at the SdFat documentation for information about it
 */

// we need the SdFat library here
#include <SdFat.h>

// path to save file
#define PATH "games/treasureminer/highscore.sav"
#define FOLDER "games/treasureminer"

// the current best scores and the coresponding names
uint32_t highScores[5] = {0, 0, 0, 0, 0};
char highScoresNames[5][4] = {"---", "---", "---", "---", "---"};

// only set to false if sd initialization failed
boolean sdAviable = true;

SdFat SD;

// function headers
boolean initSD();
void loadScores();
uint8_t addScore(uint32_t score);
void saveScores();

// initializing the SD card. returns false if it failes
boolean initSD() {
    // check if the SD initialization fails for some reason
    if (!SD.begin()) {
        sdAviable = false;
        return false;
    }
    
    // create directory if it doesn't exist
    if (!SD.exists(FOLDER))
        SD.mkdir(FOLDER);

    // load best score from sd
    loadScores();
    return true;
}

// loading the best score from the sd card
void loadScores() {
    // exit if we sd isn't aviable
    if (!sdAviable)
        return;

    // check if the file exists - if not, no score has been saved yet
    if (SD.exists(PATH)) {
        File file = SD.open(PATH, FILE_READ);
        uint8_t dataBuffer[4 + 8];
        for (int i = 0; i < 5; i++) {
            // read score and name into buffer
            file.read(dataBuffer, 4 + 3);
            // build score and store it
            highScores[i] = dataBuffer[0] << 24 | dataBuffer[1] << 16 | dataBuffer[2] << 8 | dataBuffer[3];
            // copy name characters
            for (int j = 0; j < 3; j++) {
                highScoresNames[i][j] = dataBuffer[4 + j];
            }
        }
        file.close();
    } else {
        // no score has been saved yet - save 0 as best score
        saveScores();
    }
}

// check if the given score should go into the high scores.
// if it is a new high score, the placement it got is returned (0 to 4)
// if not, a number > 5 is returned
uint8_t addScore(int score, char *name) {
    if (!sdAviable)
        return 255;

    uint8_t newScore = 255;
    // check if the score is higher than last-placed high score
    if (score > highScores[5 - 1]) {
        highScores[5 - 1] = score;
        strcpy(highScoresNames[5 - 1], name);
        newScore = 5 - 1;
    }
    // now, go up the high scores until the score above is higher
    for (int i = 5 - 2; i >= 0; i--) {
        if (score > highScores[i]) {
            highScores[i + 1] = highScores[i];
            strcpy(highScoresNames[i + 1], highScoresNames[i]);
            highScores[i] = score;
            strcpy(highScoresNames[i], name);
            newScore = i;
        }
    }

    // save to SD
    saveScores();
    return newScore;
}

// saving the high scores to the sd card
void saveScores() {
    if (!sdAviable)
        return;

    // open file and go to beginning
    File file = SD.open(PATH, FILE_WRITE);
    file.seek(0);
    uint8_t dataBuffer[4 + 3];
    
    for (int i = 0; i < 5; i++) {
        // copy score and name to buffer, then write it to the file
        dataBuffer[0] = highScores[i] >> 24;
        dataBuffer[1] = highScores[i] >> 16;
        dataBuffer[2] = highScores[i] >> 8;
        dataBuffer[3] = highScores[i];
        for (int j = 0; j < 3; j++) {
            dataBuffer[4 + j] = highScoresNames[i][j];
        }
        file.write(dataBuffer, 4 + 3);
    }
    
    file.close();
}
