/*
PocketStar.h
Last modified 09 Jan 2019

This file is part of the PocketStar Library.

Copyright (C) 2018  Florian Keller

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PocketStar_h
#define PocketStar_h

#include <SPI.h>
#include <SdFat.h>
#include "PS_Colors.h"
#include "PS_Commands.h"
#include "PS_FontDefinitions.h"
#include "PS_Fonts.h"
#include "PS_Images.h"
#include "PS_PauseMenuImages.h"
#include "PS_Sprites.h"

#define _SETTINGS_FILE "pocketstar.settings"

const uint8_t ColorModeRGB = 0;
const uint8_t ColorModeBGR = 1;

const uint8_t ButtonUp      = 1 << 0;
const uint8_t ButtonDown    = 1 << 1;
const uint8_t ButtonLeft    = 1 << 2;
const uint8_t ButtonRight   = 1 << 3;
const uint8_t ButtonA       = 1 << 4;
const uint8_t ButtonB       = 1 << 5;

const uint8_t BacklightBlack    = (0 << 2) | (0 << 1) | (0 << 0);
const uint8_t BacklightRed      = (1 << 2) | (0 << 1) | (0 << 0);
const uint8_t BacklightGreen    = (0 << 2) | (1 << 1) | (0 << 0);
const uint8_t BacklightBlue     = (0 << 2) | (0 << 1) | (1 << 0);
const uint8_t BacklightYellow   = (1 << 2) | (1 << 1) | (0 << 0);
const uint8_t BacklightMagenta  = (1 << 2) | (0 << 1) | (1 << 0);
const uint8_t BacklightCyan     = (0 << 2) | (1 << 1) | (1 << 0);
const uint8_t BacklightWhite    = (1 << 2) | (1 << 1) | (1 << 0);

const uint8_t PIN_LED_RED           = 17;   // PA04
const uint8_t PIN_LED_GREEN         = 18;   // PA05
const uint8_t PIN_LED_BLUE          = 8;    // PA06

const uint8_t PIN_BATTERY_LEVEL     = 9;    // PA07
const uint8_t PIN_BATTERY_CHARGING  = 31;   // PB23

const uint8_t PIN_SD_IDENTIFY       = 6;    // PA20

const uint8_t PIN_VIBRATION         = 32;   // PA22

const uint8_t PIN_SPEAKER           = 14;   // PA02

const uint8_t PIN_DC                = 22;   // PA12
const uint8_t PIN_CS                = 38;   // PA13
const uint8_t PIN_SHDN              = 27;   // PA28
const uint8_t PIN_RST               = 26;   // PA27

const uint8_t PIN_BUTTON_UP         = 7;    // PA21
const uint8_t PIN_BUTTON_LEFT       = 5;    // PA15
const uint8_t PIN_BUTTON_RIGHT      = 30;   // PB22
const uint8_t PIN_BUTTON_DOWN       = 2;    // PA14
const uint8_t PIN_BUTTON_A          = 42;   // PA03
const uint8_t PIN_BUTTON_B          = 4;    // PA08
const uint8_t PIN_BUTTON_PAUSE      = 25;   // PB03

class PocketStar : public Print {
  public:
    
    // init
    PocketStar();
    void begin(void);
    
    // general control
    void startTransfer(void);
    void endTransfer(void);
    void on(void);
    void off(void);
    void setMirror(boolean mirror);
    void setBitDepth(uint8_t bitDepth);
    void setColorMode(uint8_t colorMode);
    void setBrightness(uint8_t brightness);
    
    // accelerated graphics commands
    void clearWindow(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
    void clearScreen(void);
    void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);
    void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
    void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t red, uint8_t green, uint8_t blue);
    void drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, boolean fill, uint8_t color);
    void drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, boolean fill, uint16_t color);
    void drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, boolean fill, uint8_t red, uint8_t green, uint8_t blue);
    
    // pixel manipulation
    void drawPixel(uint8_t x, uint8_t y, uint16_t color);
    void setX(uint8_t start, uint8_t end);
    void setY(uint8_t start, uint8_t end);
    void goTo(uint8_t x, uint8_t y);
    void writeBuffer(const uint8_t *buffer, int count);
    void writeBuffer(const uint16_t *buffer, int count);
    
    // drawing images and sprites
    void drawImage(uint8_t x, uint8_t y, const PSImage *image);
    void drawSprites(const PSSprite *sprites[], uint8_t numSprites, uint8_t background = BLACK_8b, uint8_t ySkip = 0);
    void drawSprites(const PSSprite *sprites[], uint8_t numSprites, const PSImage *background, uint8_t ySkip = 0);
    
    // printing text
    void setFont(const PSFont &font);
    void setFontColor(uint16_t color, uint16_t colorBackground);
    void setCursor(uint8_t x, uint8_t y);
    uint8_t getPrintWidth(char *text);
    
    // buttons
    uint8_t getButtons(uint8_t buttonMask = ButtonUp | ButtonDown | ButtonLeft | ButtonRight | ButtonA | ButtonB);
    
    // vibration motor
    void vibrate(boolean vibrate);
    boolean vibrationEnabled;
    
    // battery information
    float batteryLevel();
    boolean isCharging();
    
    // sd information
    boolean isSDConnected();
    
    // led functions
    void setBacklightColor(uint8_t color);
    void disableBacklight();
    
    // bounds
    static const uint8_t xMax = 95;
    static const uint8_t yMax = 63;
    
    // pause menu helpers
    void saveVariables();
    void restoreVariables();
    
  private:
    boolean bitDepth16, colorModeBGR, mirror;
    uint8_t cursorX, cursorY, fontHeight, fontFirstChar, fontLastChar;
    uint16_t fontColor, fontColorBackground;
    const PSCharInfo *fontDescriptor;
    const unsigned char *fontBitmap;
    SPIClass *PocketStar_SPI;
    
    boolean xbitDepth16, xcolorModeBGR, xmirror;
    uint8_t xcursorX, xcursorY, xfontHeight, xfontFirstChar, xfontLastChar;
    uint16_t xfontColor, xfontColorBackground;
    const PSCharInfo *xfontDescriptor;
    const unsigned char *xfontBitmap;
    
    void startCommandTransfer(void);
    void writeRemap(void);
    void drawSprites(const PSSprite *sprites[], uint8_t numSprites, const PSImage *backgroundImg, uint8_t backgroundCol, boolean image, uint8_t ySkip = 0);
    virtual size_t write(uint8_t c);
};

extern PocketStar pocketstar;
extern boolean _paused;
extern uint8_t _brightness;
extern boolean _vibration;
extern uint8_t _sound;
extern uint8_t _ledInMenu;
extern uint8_t _standbyNr;
extern float _timeSinceLastInput;
extern float _vibrationTimer;
extern boolean _vibrating;



void initialize();
void update(float dt);
boolean pause();
void resume();

void _enterPauseMenu();
void _exitPauseMenu();
void _updatePauseMenu();
void _drawPauseMenu(boolean full);
float _getPercentage();
void _resetFlashTimer();
void _backToMainMenu();
void _standbyMode();
void _goIntoStandby(boolean canResume);

void _initSD();
void _loadSettings();
void _saveSettings();

#endif