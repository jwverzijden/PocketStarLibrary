/*
PocketStar.cpp
Last modified 07 Mar 2019

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

#include "PocketStar.h"

#define PS_send(x) SERCOM4->SPI.DATA.bit.DATA = (x);
#define PS_wait() while (SERCOM4->SPI.INTFLAG.bit.DRE == 0);
#define PS_max(x, y) ((x) > (y)) ? (x) : (y)
#define PS_min(x, y) ((x) < (y)) ? (x) : (y)

// init
PocketStar::PocketStar() {
    cursorX = 0;
    cursorY = 0;
    fontHeight = 0;
    fontFirstChar = 0;
    fontLastChar = 0;
    fontDescriptor = 0;
    fontBitmap = 0;
    fontColor = 0xFFFF;
    fontColorBackground = 0x0000;
    bitDepth16 = false;
    mirror = false;
    colorModeBGR = false;
    PocketStar_SPI = &SPI1;
}

void PocketStar::begin(void) {
    PocketStar_SPI->begin();
    PocketStar_SPI->setDataMode(SPI_MODE0);
    PocketStar_SPI->setClockDivider(4);

    pinMode(PIN_SHDN, OUTPUT);
    pinMode(PIN_DC, OUTPUT);
    pinMode(PIN_CS, OUTPUT);
    pinMode(PIN_RST, OUTPUT);
    
    digitalWrite(PIN_SHDN, LOW);
    digitalWrite(PIN_DC, HIGH);
    digitalWrite(PIN_CS, HIGH);
    digitalWrite(PIN_RST, HIGH);
    
    pinMode(PIN_BUTTON_UP, INPUT_PULLUP);
    pinMode(PIN_BUTTON_DOWN, INPUT_PULLUP);
    pinMode(PIN_BUTTON_LEFT, INPUT_PULLUP);
    pinMode(PIN_BUTTON_RIGHT, INPUT_PULLUP);
    pinMode(PIN_BUTTON_A, INPUT_PULLUP);
    pinMode(PIN_BUTTON_B, INPUT_PULLUP);
    pinMode(PIN_BUTTON_PAUSE, INPUT_PULLUP);
    
    pinMode(PIN_BATTERY_LEVEL, INPUT);
    pinMode(PIN_BATTERY_CHARGING, INPUT);
    
    pinMode(PIN_SD_IDENTIFY, INPUT);
    
    pinMode(PIN_VIBRATION, OUTPUT);
    
    pinMode(PIN_SPEAKER, OUTPUT);
    analogWrite(PIN_SPEAKER, HIGH);
    
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);
    digitalWrite(PIN_LED_RED, HIGH);
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_BLUE, HIGH);
    
    digitalWrite(PIN_RST, LOW);
    delay(5);
    digitalWrite(PIN_RST, HIGH);
    delay(10);
    
    const uint8_t init[32] = {0xAE, 0xA1, 0x00, 0xA2, 0x00, 0xA4, 0xA8, 0x3F, 0xAD, 0x8E, 0xB0, 0x0B, 0xB1, 0x31, 0xB3, 0xF0, 0x8A, 0x64, 0x8B, 0x78, 0x8C, 0x64, 0xBB, 0x3A, 0xBE, 0x3E, 0x81, 0x91, 0x82, 0x50, 0x83, 0x7D};
    off();
    startCommandTransfer();
    for (uint8_t i = 0; i < 32; i++)
        PocketStar_SPI->transfer(init[i]);
    endTransfer();
    
    writeRemap();
    setBrightness(5);
    clearScreen();
    on();
}

// general control
void PocketStar::startTransfer(void) {
    digitalWrite(PIN_DC, HIGH);
    digitalWrite(PIN_CS, LOW);
}

void PocketStar::startCommandTransfer(void) {
    digitalWrite(PIN_DC, LOW);
    digitalWrite(PIN_CS, LOW);
}

void PocketStar::endTransfer(void) {
    digitalWrite(PIN_CS, HIGH);
}

void PocketStar::on(void) {
    digitalWrite(PIN_SHDN, HIGH);
    startCommandTransfer();
    delayMicroseconds(10000);
    PocketStar_SPI->transfer(COMMAND_DISPLAY_ON);
    endTransfer();
}

void PocketStar::off(void) {
    startCommandTransfer();
    PocketStar_SPI->transfer(COMMAND_DISPLAY_OFF);
    endTransfer();
    digitalWrite(PIN_SHDN, LOW);
}

void PocketStar::setMirror(boolean _mirror) {
    mirror = _mirror;
    writeRemap();
}

void PocketStar::setBitDepth(uint8_t bitDepth) {
    bitDepth16 = bitDepth;
    writeRemap();
}

void PocketStar::setColorMode(uint8_t colorMode) {
    colorModeBGR = colorMode;
    writeRemap();
}

void PocketStar::writeRemap() {
    uint8_t remap = (1 << 5);
    if (mirror)
        remap ^= (1 << 1);
    if (bitDepth16)
        remap ^= (1 << 6);
    if (colorModeBGR)
        remap ^= (1 << 2);
    
    remap ^= ((1 << 4) | (1 << 1));
    
    startCommandTransfer();
    PocketStar_SPI->transfer(COMMAND_SET_REMAP);
    PocketStar_SPI->transfer(remap);
    endTransfer();
}

void PocketStar::setBrightness(uint8_t brightness) {
    if (brightness > 15)
        brightness = 15;    
    startCommandTransfer();
    PocketStar_SPI->transfer(COMMAND_MASTER_CURRENT);
    PocketStar_SPI->transfer(brightness);
    endTransfer();
}

// accelerated graphics commands
void PocketStar::clearWindow(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    if (x > xMax || y > yMax)
        return;
    uint8_t x2 = x + width - 1;
    uint8_t y2 = y + height - 1;
    if (x2 > xMax)
        x2 = xMax;
    if (y2 > yMax)
        y2 = yMax;
    
    startCommandTransfer();
    PocketStar_SPI->transfer(COMMAND_CLEAR_WINDOW);
    PocketStar_SPI->transfer(x);
    PocketStar_SPI->transfer(y);
    PocketStar_SPI->transfer(x2);
    PocketStar_SPI->transfer(y2);
    endTransfer();
    delayMicroseconds(400);
}

void PocketStar::clearScreen() {
    clearWindow(0, 0, 96, 64);
}

void PocketStar::drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color) {
    uint8_t red = (color >> 5) & 0x7;
    uint8_t green = (color >> 2) & 0x7;
    uint8_t blue = color & 0x3;
    
    red = red | (red << 3);
    green = green | (green << 3);
    blue = blue | (blue << 2) | (blue << 4);
    
    drawLine(x1, y1, x2, y2, red, green, blue);
}

void PocketStar::drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color) {
    uint8_t red = (color >> 10) & 0x3E;
    uint8_t green = (color >> 5) & 0x3F;
    uint8_t blue = (color << 1) & 0x3E;
    
    drawLine(x1, y1, x2, y2, red, green, blue);
}

void PocketStar::drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t red, uint8_t green, uint8_t blue) {
    if (x1 > xMax)
        x1 = xMax;
    if (y1 > yMax)
        y1 = yMax;
    if (x2 > xMax)
        x2 = xMax;
    if (y2 > yMax)
        y2 = yMax;
    
    startCommandTransfer();
    PocketStar_SPI->transfer(COMMAND_DRAW_LINE);
    PocketStar_SPI->transfer(x1);
    PocketStar_SPI->transfer(y1);
    PocketStar_SPI->transfer(x2);
    PocketStar_SPI->transfer(y2);
    PocketStar_SPI->transfer(red);
    PocketStar_SPI->transfer(green);
    PocketStar_SPI->transfer(blue);
    endTransfer();
    delayMicroseconds(100);
}

void PocketStar::drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, boolean fill, uint8_t color) {
    uint8_t red = (color >> 5) & 0x7;
    uint8_t green = (color >> 2) & 0x7;
    uint8_t blue = color & 0x3;
    
    red = red | (red << 3);
    green = green | (green << 3);
    blue = blue | (blue << 2) | (blue << 4);
    
    drawRect(x, y, width, height, fill, red, green, blue);
}

void PocketStar::drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, boolean fill, uint16_t color) {
    uint8_t red = (color >> 10) & 0x3E;
    uint8_t green = (color >> 5) & 0x3F;
    uint8_t blue = (color << 1) & 0x3E;
    
    drawRect(x, y, width, height, fill, red, green, blue);
}

void PocketStar::drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, boolean fill, uint8_t red, uint8_t green, uint8_t blue) {
    if (x > xMax || y > yMax || width == 0 || height == 0)
        return;
    uint8_t x2 = x + width - 1;
    uint8_t y2 = y + height - 1;
    if (x2 > xMax)
        x2 = xMax;
    if (y2 > yMax)
        y2 = yMax;
    
    uint8_t fillx = 0;
    if (fill)
        fillx = 1;
    
    startCommandTransfer();
    PocketStar_SPI->transfer(COMMAND_SET_FILL);
    PocketStar_SPI->transfer(fillx);
    PocketStar_SPI->transfer(COMMAND_DRAW_RECTANGLE);
    PocketStar_SPI->transfer(x);
    PocketStar_SPI->transfer(y);
    PocketStar_SPI->transfer(x2);
    PocketStar_SPI->transfer(y2);
    PocketStar_SPI->transfer(red);
    PocketStar_SPI->transfer(green);
    PocketStar_SPI->transfer(blue);
    PocketStar_SPI->transfer(red);
    PocketStar_SPI->transfer(green);
    PocketStar_SPI->transfer(blue);
    endTransfer();
    delayMicroseconds(400);
}

// pixel manipulation
void PocketStar::drawPixel(uint8_t x, uint8_t y, uint16_t color) {
    if (x > xMax || y > yMax)
        return;
    goTo(x, y);
    startTransfer();
    if (bitDepth16)
        PocketStar_SPI->transfer(color >> 8);
    PocketStar_SPI->transfer(color);
    endTransfer();
}

void PocketStar::setX(uint8_t start, uint8_t end) {
    if(start > xMax)
        start = xMax;
    if(end > xMax)
        end = xMax;
    startCommandTransfer();
    PocketStar_SPI->transfer(COMMAND_SET_COLOUMN);
    PocketStar_SPI->transfer(start);
    PocketStar_SPI->transfer(end);
    endTransfer();
}

void PocketStar::setY(uint8_t start, uint8_t end) {
    if(start > yMax)
        start = yMax;
    if(end > yMax)
        end = yMax;
    startCommandTransfer();
    PocketStar_SPI->transfer(COMMAND_SET_ROW);
    PocketStar_SPI->transfer(start);
    PocketStar_SPI->transfer(end);
    endTransfer();
}

void PocketStar::goTo(uint8_t x, uint8_t y) {
    if(x > xMax || y > yMax)
        return;
    setX(x, xMax);
    setY(y, yMax);
}

void PocketStar::writeBuffer(const uint8_t *buffer, int count) {
    uint8_t temp;
    PS_send(buffer[0]);
    for (int i = 1; i < count; i++) {
        temp = buffer[i];
        PS_wait();
        PS_send(temp);
    }
    PS_wait();
}

void PocketStar::writeBuffer(const uint16_t *buffer, int count) {
    uint16_t temp;
    PS_send(buffer[0] >> 8);
    PS_wait();
    PS_send(buffer[0]);
    for (int i = 1; i < count; i++) {
        temp = buffer[i];
        PS_wait();
        PS_send(temp >> 8);
        PS_wait();
        PS_send(temp);
    }
    PS_wait();
}

// drawing images and sprites
void PocketStar::drawImage(uint8_t x, uint8_t y, const PSImage *image) {
    if(x < 0 || y < 0 || x > xMax || y > yMax)
        return;
    
    uint8_t width = image->width;
    uint8_t height = image->height;
    uint32_t datalength = ((uint32_t) width) * height;
    uint16_t bitDepth16 = image->bitDepth == BitDepth16;
    uint16_t cur;
    
    if (bitDepth16)
        datalength = datalength << 1;
    
    setX(x, x + width - 1);
    setY(y, y + height - 1);
    
    startTransfer();
    for (int i = 0; i < datalength; i++) {
        cur = image->data[i];
        PS_wait();
        PS_send(cur);
    }
    endTransfer();
}

void PocketStar::drawSprites(const PSSprite *sprites[], uint8_t numSprites, uint8_t background, uint8_t ySkip) {
    drawSprites(sprites, numSprites, NULL, background, false, ySkip);
}

void PocketStar::drawSprites(const PSSprite *sprites[], uint8_t numSprites, const PSImage *background, uint8_t ySkip) {
    drawSprites(sprites, numSprites, background, 0, true, ySkip);
}

void PocketStar::drawSprites(const PSSprite *sprites[], uint8_t numSprites, const PSImage *backgroundImg, uint8_t backgroundCol, boolean image, uint8_t ySkip) {
    if (ySkip >= 64)
        return;
    if (image && (backgroundImg->width != 96 || backgroundImg->height != 64))
        return;
    
    uint8_t buffer[96];
    goTo(0, ySkip);
    startTransfer();
    for (uint8_t y = ySkip; y < 64; y++) {
        if (image) {
            const uint8_t *first = backgroundImg->data + y * 96;
            memcpy(buffer, first, 96);
        } else
            memset(buffer, backgroundCol, 96);
        
        for (uint8_t i = 0; i < numSprites; i++) {
            if (sprites[i]->enabled) {
                if (sprites[i]->y <= y && y <= sprites[i]->y + sprites[i]->image->height - 1) {
                    uint8_t begin = PS_max(sprites[i]->x, 0);
                    uint8_t end = PS_min(sprites[i]->x + sprites[i]->image->width, 96);
                    if (begin > end)
                        continue;
                    
                    const uint8_t *spriteDataBegin = sprites[i]->image->data + (y - sprites[i]->y) * sprites[i]->image->width - sprites[i]->x + begin;
                    uint8_t *bufferBegin = buffer + begin;
                    uint8_t d = end - begin;
                    if (!sprites[i]->mirrorFlag) {
                        for (uint8_t j = 0; j < d; j++) {
                            if (spriteDataBegin[j] != ALPHA)
                                bufferBegin[j] = spriteDataBegin[j];
                        }
                    } else {
                        for (uint8_t j = 0; j < d; j++) {
                            if (spriteDataBegin[d - j - 1] != ALPHA)
                                bufferBegin[j] = spriteDataBegin[d - j - 1];
                        }
                    }
                }
            }
        }
        
        writeBuffer(buffer, 96);
    }
    endTransfer();
}

// printing text
void PocketStar::setFont(const PSFont &font) {
    fontHeight = font.height;
    fontFirstChar = font.startChar;
    fontLastChar = font.endChar;
    fontDescriptor = font.charInfo;
    fontBitmap = font.bitmap;
}

void PocketStar::setFontColor(uint16_t color, uint16_t colorBackground) {
    fontColor = color;
    fontColorBackground = colorBackground;
}

void PocketStar::setCursor(uint8_t x, uint8_t y) {
    cursorX = x;
    cursorY = y;
}

uint8_t PocketStar::getPrintWidth(char *text) {
    if(!fontFirstChar)
        return 0;
    uint8_t charAmount, result = 0;
    charAmount = strlen(text);
    for(int i = 0; i < charAmount; i++) {
        result += pgm_read_byte(&fontDescriptor[text[i] - fontFirstChar].width) + 1;
    }
    return result;
}

size_t PocketStar::write(uint8_t c) {
    if (!fontFirstChar)
        return 1;
    if (c < fontFirstChar || c > fontLastChar)
        return 1;
    if (cursorX > xMax || cursorY > yMax)
        return 1;
    
    uint8_t charWidth = pgm_read_byte(&fontDescriptor[c - fontFirstChar].width);
    uint8_t bytesPerRow = charWidth / 8;
    if (charWidth > bytesPerRow * 8)
        bytesPerRow++;
    uint16_t offset = pgm_read_word(&fontDescriptor[c - fontFirstChar].offset) + (bytesPerRow * fontHeight) - 1;
    
    setX(cursorX, cursorX + charWidth + 1);
    setY(cursorY, cursorY + fontHeight);
    
    startTransfer();
    for (uint8_t y = 0; y < fontHeight && y + cursorY < yMax + 1; y++) {
        if (bitDepth16) {
            PS_send(fontColorBackground >> 8);
            PS_wait();
        }
        PS_send(fontColorBackground);
        for (uint8_t byte = 0; byte < bytesPerRow; byte++) {
            uint8_t data = pgm_read_byte(fontBitmap + offset - y - ((bytesPerRow - byte - 1) * fontHeight));
            uint8_t bits = byte * 8;
            for (uint8_t i = 0; i < 8 && (bits + i) < charWidth && (bits + i + cursorX) < xMax; i++) {
                PS_wait();
                if (data & (0x80 >> i)) {
                    if (bitDepth16) {
                        PS_send(fontColor >> 8);
                        PS_wait();
                    }
                    PS_send(fontColor);
                 } else {
                    if (bitDepth16) {
                        PS_send(fontColorBackground >> 8);
                        PS_wait();
                    }
                    PS_send(fontColorBackground);
                }
            }
        }
        PS_wait();
        if ((cursorX + charWidth) < xMax) {
            if (bitDepth16) {
                PS_send(fontColorBackground >> 8);
                PS_wait();
            }
            PS_send(fontColorBackground);
            PS_wait();
        }
    }
    endTransfer();
    cursorX += (charWidth + 1);
    return 1;
}

// buttons
uint8_t PocketStar::getButtons(uint8_t buttonMask) {
    uint8_t result = 0;
    
    if (!digitalRead(PIN_BUTTON_UP))
        result |= ButtonUp;
    if (!digitalRead(PIN_BUTTON_DOWN))
        result |= ButtonDown;
    if (!digitalRead(PIN_BUTTON_LEFT))
        result |= ButtonLeft;
    if (!digitalRead(PIN_BUTTON_RIGHT))
        result |= ButtonRight;
    if (!digitalRead(PIN_BUTTON_A))
        result |= ButtonA;
    if (!digitalRead(PIN_BUTTON_B))
        result |= ButtonB;

    return result & buttonMask;
}

// vibration motor
void PocketStar::vibrate(boolean vibrate) {
    if (vibrate && vibrationEnabled)
        digitalWrite(PIN_VIBRATION, HIGH);
    else
        digitalWrite(PIN_VIBRATION, LOW);
}

// battery information
float PocketStar::batteryLevel() {
    int value = analogRead(PIN_BATTERY_LEVEL);
    float battery;
    
    if (!isCharging()) {
        if (value < 552)
            battery = 1/360.f*value - 89/60.f;
        else if (value < 564)
            battery = 1/240.f*value - 9/4.f;
        else if (value < 574)
            battery = 1/100.f*value - 277/50.f;
        else if (value < 581)
            battery = 1/35.f*value - 81/5.f;
        else if (value < 592)
            battery = 1/55.f*value - 559/55.f;
        else if (value < 615)
            battery = 1/115.f*value - 523/115.f;
        else
            battery = 1/150.f*value - 33/10.f;
    } else {
        if (value < 583)
            battery = 3/2650.f*value - 3/5.f;
        else if (value < 600)
            battery = 4/425.f*value - 4613/850.f;
        else if (value < 620)
            battery = 9/400.f*value - 332/25.f;
        else
            battery = 11/1000.f*value - 123/20.f;
    }
    
    if (battery < 0.01)
        battery = 0.01;
    if (battery > 1)
        battery = 1;
    
    return battery;
}

boolean PocketStar::isCharging() {
    return !digitalRead(PIN_BATTERY_CHARGING);
}

// sd information
boolean PocketStar::isSDConnected() {
    return !digitalRead(PIN_SD_IDENTIFY);
}

// led functions
void PocketStar::setBacklightColor(uint8_t color) {
    if (color & (1 << 2))
        digitalWrite(PIN_LED_RED, LOW);
    else
        digitalWrite(PIN_LED_RED, HIGH);
    
    if (color & (1 << 1))
        digitalWrite(PIN_LED_GREEN, LOW);
    else
        digitalWrite(PIN_LED_GREEN, HIGH);
    
    if (color & (1 << 0))
        digitalWrite(PIN_LED_BLUE, LOW);
    else
        digitalWrite(PIN_LED_BLUE, HIGH);
}

void PocketStar::disableBacklight() {
    setBacklightColor(BacklightBlack);
}

// pause menu helpers
void PocketStar::saveVariables() {
    xbitDepth16 = bitDepth16;
    xcolorModeBGR = colorModeBGR;
    xmirror = mirror;
    xfontHeight = fontHeight;
    xfontFirstChar = fontFirstChar;
    xfontLastChar = fontLastChar;
    xfontColor = fontColor;
    xfontColorBackground = fontColorBackground;
    xfontDescriptor = fontDescriptor;
    xfontBitmap = fontBitmap;
}

void PocketStar::restoreVariables() {
    bitDepth16 = xbitDepth16;
    colorModeBGR = xcolorModeBGR;
    mirror = xmirror;
    writeRemap();
    fontHeight = xfontHeight;
    fontFirstChar = xfontFirstChar;
    fontLastChar = xfontLastChar;
    fontColor = xfontColor;
    fontColorBackground = xfontColorBackground;
    fontDescriptor = xfontDescriptor;
    fontBitmap = xfontBitmap;
}



PocketStar pocketstar = PocketStar();
boolean _paused = false;
long _tickBeginning = 0;
float _tpf = 0;
float _timePauseButtonPressed = 0;
boolean _blockInput = false;
uint8_t _curOption = 0;
uint8_t _brightness = 5;
boolean _vibration = true;
uint8_t _sound = 0;
uint8_t _ledInMenu = BacklightBlack;
uint8_t _standbyNr = 2;
float _standbyTimes[7] = {1*60, 3*60, 5*60, 10*60, 15*60, 42*60, INFINITY};
float _timeSinceLastInput;
boolean _dimmed;
const int _FLASH_CYCLE = 1500;
long _flashTimer;
const float _VIBRATE_TIMER = 0.1;
boolean _vibrating;
float _vibrationTimer;
const int LOADING_IMAGE_TIMER = 1500;
float _lastBatteryImageChange;

void setup() {
    pocketstar.begin();
    pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
    
    _initSD();
    _loadSettings();
    pocketstar.setBrightness(_brightness);
    pocketstar.vibrationEnabled = _vibration;
    
    srand(pocketstar.batteryLevel() * 1000);
    initialize();
}

void _nothing() {}

void loop() {
    _tickBeginning = millis();
    if (!digitalRead(PIN_BUTTON_PAUSE)) {
        _timePauseButtonPressed += _tpf;
        if (_timePauseButtonPressed >= 1) {
            _goIntoStandby(true);
            _timePauseButtonPressed = 0.1f;
        }
    } else {
        if (_timePauseButtonPressed > 0) {
            if (!_paused && pause()) {
                _enterPauseMenu();
            }
        }
        _timePauseButtonPressed = 0;
    }
    
    if (!_paused) {
        update(_tpf);
    } else {
        _flashTimer += _tpf * 1000;
        _lastBatteryImageChange += _tpf;
        _drawPauseMenu(false);
        _updatePauseMenu();
    }
    if (_vibrating) {
        _vibrationTimer -= _tpf;
        if (_vibrationTimer <= 0) {
            pocketstar.vibrate(false);
            _vibrating = false;
        }
    }
    
    if (!pocketstar.getButtons()) {
        _timeSinceLastInput += _tpf;
        if (_timeSinceLastInput > _standbyTimes[_standbyNr] - 10 && !_dimmed) {
            pocketstar.setBrightness(_brightness/2);
            _dimmed = true;
        }
        if (_timeSinceLastInput > _standbyTimes[_standbyNr]) {
            _goIntoStandby(true);
            pocketstar.setBrightness(_brightness);
            _dimmed = false;
            _timeSinceLastInput = 0;
        }
    } else {
        _timeSinceLastInput = 0;
        if (_dimmed) {
            pocketstar.setBrightness(_brightness);
            _dimmed = false;
        }
    }
    
    if (pocketstar.batteryLevel() < 0.05 && !pocketstar.isCharging()) {
        pocketstar.saveVariables();
        while (!pocketstar.isCharging() || pocketstar.batteryLevel() > 0.10) {
            pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
            pocketstar.setBitDepth(BitDepth16);
            pocketstar.setFont(pocketStar6pt);
            pocketstar.setFontColor(RED_16b, BLACK_16b);
            pocketstar.setCursor(48 - pocketstar.getPrintWidth("LOW BATTERY")/2, 30);
            pocketstar.print("LOW BATTERY");
            delay(2000);
            _goIntoStandby(false);
        }
        _exitPauseMenu();
        //_backToMainMenu();
    }
    
    _tpf = (millis() - _tickBeginning) / 1000.f;
}

void _enterPauseMenu() {
    if (_paused)
        return;
    _paused = true;
    pocketstar.saveVariables();
    pocketstar.vibrate(false);
    _lastBatteryImageChange = 1000;
    _drawPauseMenu(true);
}

void _exitPauseMenu() {
    _paused = false;
    pocketstar.restoreVariables();
    resume();
    _tpf = 0;
}

void _updatePauseMenu() {
    if (!pocketstar.getButtons())
        _blockInput = false;
    
    if (_blockInput)
        return;
    
    if (pocketstar.getButtons(ButtonUp)) {
        if (_curOption > 0)
            _curOption--;
        else
            _curOption = 3;
        _blockInput = true;
        _resetFlashTimer();
    }
    if (pocketstar.getButtons(ButtonDown)) {
        if (_curOption < 3)
            _curOption++;
        else
            _curOption = 0;
        _blockInput = true;
        _resetFlashTimer();
    }
    if (pocketstar.getButtons(ButtonB)) {
        _exitPauseMenu();
        return;
    }
    
    switch (_curOption) {
        case 0: {
            if (pocketstar.getButtons(ButtonA)) {
                _exitPauseMenu();
                return;
            }
            break;
        }
        case 1: {
            boolean changed = false;
            if (_brightness < 15 && pocketstar.getButtons(ButtonRight)) {
                _brightness++;
                changed = true;
            } else if (_brightness > 0 && pocketstar.getButtons(ButtonLeft)) {
                _brightness--;
                changed = true;
            }
            if (changed) {
                pocketstar.setBrightness(_brightness);
                _saveSettings();
                _blockInput = true;
            }
            break;
        }
        case 2: {
            if (pocketstar.getButtons(ButtonRight | ButtonLeft)) {
                _vibration = !_vibration;
                pocketstar.vibrationEnabled = _vibration;
                if (_vibration) {
                    pocketstar.vibrate(true);
                    _vibrationTimer = _VIBRATE_TIMER;
                    _vibrating = true;
                }
                _saveSettings();
                _blockInput = true;
            }
            break;
        }
        case 3: {
            if (pocketstar.getButtons(ButtonA)) {
                _backToMainMenu();
            }
            break;
        }
    }
}

float _getPercentage() {
    long ms = _flashTimer % _FLASH_CYCLE;
    return ((float) ms) / _FLASH_CYCLE;
}

void _resetFlashTimer() {
    _flashTimer = _FLASH_CYCLE/2;
}

void _drawPauseMenu(boolean full) {
    char *strings[4] = {"Back", "Brightness", "Vibration", "Main Menu"};
    
    pocketstar.setBitDepth(BitDepth16);
    pocketstar.setFont(pocketStar6pt);
    pocketstar.setFontColor(CYAN_16b, DARKBLUE_16b);
    if (full) {
        pocketstar.drawRect(12, 9, 72, 45, true, DARKBLUE_16b);
    }
    pocketstar.setCursor(13, 10);
    pocketstar.print("SETTINGS");
    
    float perc = _getPercentage() * 2;
    if (perc > 1)
        perc = 2 - perc;
    uint16_t color = 31 * perc;
    color = RED_16b | (color << 6);
    
    for (int i = 0; i < 4; i++) {
        if (i == _curOption)
            pocketstar.setFontColor(color, DARKBLUE_16b);
        else
            pocketstar.setFontColor(WHITE_16b, DARKBLUE_16b);
        pocketstar.setCursor(13, 18 + i * 9);
        pocketstar.print(strings[i]);
        if (i == 1) {
            int value = 20 + (_brightness + 1) * 5;
            char strbuf[6];
            if (value < 100)
                sprintf(strbuf, " %i%%", value);
            else
                sprintf(strbuf, "%i%%", value);
            pocketstar.setCursor(83 - pocketstar.getPrintWidth(strbuf), 18 + i * 9);
            pocketstar.print(strbuf);
        } else if (i == 2) {
            if (_vibration) {
                pocketstar.setCursor(83 - pocketstar.getPrintWidth(" on"), 18 + i * 9);
                pocketstar.print(" on");
            } else {
                pocketstar.setCursor(83 - pocketstar.getPrintWidth("off"), 18 + i * 9);
                pocketstar.print("off");
            }
        }
    }
    
    pocketstar.setBitDepth(BitDepth8);
    if (!pocketstar.isCharging()) {
        if (_lastBatteryImageChange > 60) {
            _lastBatteryImageChange = 0;
            float batteryLevel = pocketstar.batteryLevel();
            int imageNr;
            if (batteryLevel > 0.75)
                imageNr = 3;
            else if (batteryLevel > 0.5)
                imageNr = 2;
            else if (batteryLevel > 0.25)
                imageNr = 1;
            else
                imageNr = 0;
            pocketstar.drawImage(71, 11, &batteryImages[imageNr]);
        }
    } else {
        int imageNr = (millis() % LOADING_IMAGE_TIMER) / (LOADING_IMAGE_TIMER / 4);
        pocketstar.drawImage(71, 11, &batteryLoadingImages[imageNr]);
        _lastBatteryImageChange = 1000;
    }
    
    pocketstar.setBitDepth(BitDepth16);
    
    pocketstar.drawRect(11, 8, 74, 47, false, BLACK_16b);
}

void _backToMainMenu() {
    NVIC_SystemReset();
}

void _goIntoStandby(boolean canResume) {
    pocketstar.off();
    pocketstar.vibrate(false);
    pocketstar.disableBacklight();
    delay(1000);
    attachInterrupt(PIN_BUTTON_PAUSE, _nothing, LOW);
    _standbyMode();
    if (_paused || pause()) {
        _enterPauseMenu();
    } else if (canResume) {
        _exitPauseMenu();
    }
    pocketstar.on();
    detachInterrupt(PIN_BUTTON_PAUSE);
}

void _standbyMode() {
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __WFI();
}



boolean _sdAviable = true;
SdFat _SD;

void _initSD() {
    if (!_SD.begin()) {
        _sdAviable = false;
    }
}

void _loadSettings() {
    if (!_sdAviable)
        return;
    if (_SD.exists(_SETTINGS_FILE)) {
        File _file = _SD.open(_SETTINGS_FILE, FILE_READ);
        if (_file.read() == 'P' && _file.read() == 'S' && _file.read() == 'S') {
            _brightness = _file.read();
            if (_brightness > 15)
                _brightness = 15;
            _vibration = _file.read() > 0;
            _sound = _file.read();
            _ledInMenu = _file.read();
            if (_ledInMenu > BacklightWhite)
                _ledInMenu = BacklightBlack;
            _standbyNr = _file.read();
            if (_standbyNr > 6)
                _standbyNr = 6;
        }
        _file.close();
    } else {
        _saveSettings();
    }
}

void _saveSettings() {
    if (!_sdAviable)
        return;
    File _file = _SD.open(_SETTINGS_FILE, FILE_WRITE);
    _file.seek(0);
    _file.write('P');
    _file.write('S');
    _file.write('S');
    _file.write(_brightness);
    _file.write(_vibration);
    _file.write(_sound);
    _file.write(_ledInMenu);
    _file.write(_standbyNr);
    _file.close();
}