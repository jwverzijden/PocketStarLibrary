/*
 * The Save.h file provides functions for saving and loading the best score
 * 
 * Have a look at the SdFat documentation for information about it
 */

// we need the SdFat library here
#include <SdFat.h>

// path to save file. this version uses a different file than the full version to prevent overwriting
#define PATH "games/treasureminerXS/highscore.sav"
#define FOLDER "games/treasureminerXS"

// the current best score
uint32_t highscore = 0;

// only set to false if something with the sd card failed, e.g. if none was found
boolean sdAviable = true;

SdFat SD;

// function headers
void loadScore();
void saveScore(uint32_t score);

// initializing the SD card
void initSD() {

    // check if the SD initialization fails for some reason, and if so, exit early
    if (!SD.begin()) {
        sdAviable = false;
        return;
    }
    
    // create directory if it doesn't exist
    if (!SD.exists(FOLDER)) {
        SD.mkdir(FOLDER);
    }

    // load best score from sd
    loadScore();
}

// loading the best score from the sd card
void loadScore() {
    // exit if we cannot talk to the sd card
    if (!sdAviable)
        return;

    // check if the file exists - if not, no score has been saved yet
    if (SD.exists(PATH)) {
        // open file, read bytes & convert them to score, close file
        File file = SD.open(PATH, FILE_READ);
        uint8_t buffer[4];
        file.read(buffer, 4);
        highscore = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
        file.close();
    } else {
        // no score has been saved yet - save 0 as best score
        saveScore(0);
    }
}

// saving the best score to the sd card
void saveScore(uint32_t score) {
    // check if the given score is higher than the previous best score
    if (score > highscore)
        highscore = score;
    
    // exit if we cannot talk to the sd card
    if (!sdAviable)
        return;
    
    // open file, convert score to bytes & write these, close file
    File file = SD.open(PATH, FILE_WRITE);
    file.seek(0);
    file.write(highscore >> 24);
    file.write(highscore >> 16);
    file.write(highscore >> 8);
    file.write(highscore);
    file.close();
}

