/*
 * PocketStar Main Menu
 * Version 1.4
 * written by Florian Keller
 * last changed 21 Dec 18
 */

#define MENU_TITLE          0
#define MENU_MAIN           1
#define MENU_GAMES          2
#define MENU_APPS           3
#define MENU_SETTINGS       4
#define MENU_INFO           5
#define MENU_DEVICE_INFO    6
#define MENU_STATUS         7
#define MENU_CREDITS        8
#define MENU_SD_NOT_FOUND   9
#define MENU_SD_ERROR       10
#define MENU_EJECT_SD       11

#include <PocketStar.h>
#include <SdFat.h>
#include "Images.h"
#include "IndexedColorImages.h"
#include "Config.h"

SdFat games;
SdFat apps;
SdFile gameDir;
SdFile appDir;

#include "Loader.h"

uint8_t currentMenu = MENU_TITLE;
float lockInput;
long flashTimer;
boolean sdAviable = true;
uint8_t mainX = 0;
uint8_t mainY = 0;
uint8_t gameSelected = 0;
uint8_t appSelected = 0;
uint8_t availableGames = 0;
uint8_t availableApps = 0;
uint8_t nomaki = 0;
uint8_t poorlilzep = 0;
uint8_t curOption = 0;
uint8_t currentInfoSelect = 0;
uint8_t currentPage = 0;
static uint8_t infoPages = 3;
static uint8_t statusPages = 2;
static uint8_t creditsPages = 4;
float lastBatteryChange = 0;

void initialize() {
    pocketstar.setBacklightColor(_ledInMenu);
    changeMenu(MENU_TITLE);
    pocketstar.setFont(pocketStar7pt);
    pocketstar.setFontColor(WHITE_16b, BLACK_16b);

    if (!games.begin() || !apps.begin()) {
        sdAviable = false;
    }
    
    if (games.exists("games")) {
        games.chdir("/games");
        while (gameDir.openNext(games.vwd(), FILE_READ)) {
            if (isValid(true)) {
                availableGames++;
            }
            gameDir.close();
        }
    }
    
    if (apps.exists("apps")) {
        apps.chdir("/apps");
        while (appDir.openNext(apps.vwd(), FILE_READ)) {
            if (isValid(false)) {
                availableApps++;
            }
            appDir.close();
        }
    }
}

void update(float deltaTime) {
    if (!pocketstar.isSDConnected()) {
        sdAviable = false;
        if (currentMenu == MENU_GAMES || currentMenu == MENU_APPS)
            changeMenu(MENU_SD_NOT_FOUND);
    }
    flashTimer += deltaTime * 1000;
    calcInput(deltaTime);

    if (currentMenu == MENU_TITLE) {
    } else if (currentMenu == MENU_MAIN) {
        float perc = getPercentage() * 2;
        if (perc > 1)
            perc = 2 - perc;
        uint16_t color = 63 * perc;
        color = color << 5;
        
        if (mainX == 0 && mainY == 0) {
            pocketstar.drawRect(0, 0, 48, 32, false, color);
        } else {
            pocketstar.drawRect(0, 0, 48, 32, false, BLACK_16b);
        }
        if (mainX == 1 && mainY == 0) {
            pocketstar.drawRect(48, 0, 48, 32, false, color);
        } else {
            pocketstar.drawRect(48, 0, 48, 32, false, BLACK_16b);
        }
        if (mainX == 0 && mainY == 1) {
            pocketstar.drawRect(0, 32, 48, 32, false, color);
        } else {
            pocketstar.drawRect(0, 32, 48, 32, false, BLACK_16b);
        }
        if (mainX == 1 && mainY == 1) {
            pocketstar.drawRect(48, 32, 48, 32, false, color);
        } else {
            pocketstar.drawRect(48, 32, 48, 32, false, BLACK_16b);
        }
        pocketstar.setCursor(1, 1);
    } else if (currentMenu == MENU_GAMES) {
    } else if (currentMenu == MENU_APPS) {
    } else if (currentMenu == MENU_SETTINGS) {
        float perc = getPercentage() * 2;
        if (perc > 1)
            perc = 2 - perc;
        uint16_t color = 31 * perc;
        color = RED_16b | (color << 6);
        char *strings[] = {"Brightness", "Vibration", "LED Color", "Auto-Standby", "Eject SD Card"};
        char *stringsLED[] = {" off", " blue", "green", " cyan", "    red", "magenta", "   yellow", " white"};
        char *stringsStandby[] = {"1min", "3min", " 5min", "10min", "15min", "42min", "never"};
        for (int i = 0; i < 5; i++) {
            int y = 12 + i * 10;
            if (i == curOption)
                pocketstar.setFontColor(color, BLACK_16b);
            else
                pocketstar.setFontColor(WHITE_16b, BLACK_16b);
            pocketstar.setCursor(4, y);
            pocketstar.print(strings[i]);
            if (i == 0) {
                int value = 20 + (_brightness + 1) * 5;
                char strbuf[7];
                sprintf(strbuf, "  %i%%", value);
                pocketstar.setCursor(92 - pocketstar.getPrintWidth(strbuf), y);
                pocketstar.print(strbuf);
            } else if (i == 1) {
                if (_vibration) {
                    pocketstar.setCursor(92 - pocketstar.getPrintWidth("  on"), y);
                    pocketstar.print("  on");
                } else {
                    pocketstar.setCursor(92 - pocketstar.getPrintWidth("  off"), y);
                    pocketstar.print("  off");
                }
            } else if (i == 2) {
                pocketstar.setCursor(92 - pocketstar.getPrintWidth(stringsLED[_ledInMenu]), y);
                pocketstar.print(stringsLED[_ledInMenu]);
            } else if (i == 3) {
                pocketstar.setCursor(92 - pocketstar.getPrintWidth(stringsStandby[_standbyNr]), y);
                pocketstar.print(stringsStandby[_standbyNr]);
            }
        }
    } else if (currentMenu == MENU_STATUS) {
        if (currentPage == 0) {
            pocketstar.setFontColor(WHITE_16b, BLACK_16b);
            pocketstar.setCursor(5, 17);
            pocketstar.print("Battery:");

            if (pocketstar.isCharging()) {
                pocketstar.print("       ");
                pocketstar.setCursor(5, 27);
                pocketstar.print("(charging)");
                lastBatteryChange = 1000;
            } else {
                lastBatteryChange += deltaTime;
                if (lastBatteryChange > 60) {
                    lastBatteryChange = 0;
                    int battery = pocketstar.batteryLevel() * 10;
                    if (battery < 1)
                        battery = 1;
                    pocketstar.print(" ~");
                    pocketstar.print(battery * 10);
                    pocketstar.print("%  ");
                    pocketstar.setCursor(5, 27);
                    pocketstar.print("          ");
                }
            }
        }
    } else if (currentMenu == MENU_INFO) {
        float perc = getPercentage() * 2;
        if (perc > 1)
            perc = 2 - perc;
        uint16_t color = 31 * perc;
        color = RED_16b | (color << 6);
        char *strings[] = {"Device Information", "Status", "Credits"};
        for (int i = 0; i < 3; i++) {
            pocketstar.setCursor(48 - pocketstar.getPrintWidth(strings[i])/2, 20 + 10 * i);
            if (i == currentInfoSelect)
                pocketstar.setFontColor(color, BLACK_16b);
            else
                pocketstar.setFontColor(WHITE_16b, BLACK_16b);
            pocketstar.print(strings[i]);
        }
    } else if (currentMenu == MENU_SD_ERROR) {
        if (!pocketstar.isSDConnected())
            changeMenu(MENU_SD_NOT_FOUND);
    } else if (currentMenu == MENU_SD_NOT_FOUND) {
        if (pocketstar.isSDConnected())
            changeMenu(MENU_SD_ERROR);
    }
}

float getPercentage() {
    long ms = flashTimer % FLASH_CYCLE;
    return ((float) ms) / FLASH_CYCLE;
}

void resetFlashTimer() {
    flashTimer = FLASH_CYCLE/2;
}

void changeMenu(uint8_t newMenu) {
    currentMenu = newMenu;
    if (currentMenu == MENU_TITLE) {
        pocketstar.setBitDepth(BitDepth8);
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.drawImage(24, 4, &logo);
        pocketstar.setFont(pocketStar7pt);
        pocketstar.setFontColor(WHITE_16b, BLACK_16b);
        pocketstar.setCursor(48 - pocketstar.getPrintWidth("PocketStar")/2, 54);
        pocketstar.print("PocketStar");
    } else if (currentMenu == MENU_MAIN) {
        pocketstar.setBitDepth(BitDepth8);
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        drawImage(12, 3, &menuGamesImage);
        drawImage(64, 2, &menuAppsImage);
        drawImage(16, 35, &menuSettingsImage);
        drawImage(64, 35, &menuInfoImage);
        pocketstar.setFont(pocketStar7pt);
        pocketstar.setFontColor(WHITE_16b, BLACK_16b);
        pocketstar.setCursor(24 - pocketstar.getPrintWidth("Games")/2, 20);
        pocketstar.print("Games");
        pocketstar.setCursor(72 - pocketstar.getPrintWidth("Apps")/2, 20);
        pocketstar.print("Apps");
        pocketstar.setCursor(24 - pocketstar.getPrintWidth("Settings")/2, 52);
        pocketstar.print("Settings");
        pocketstar.setCursor(72 - pocketstar.getPrintWidth("Info")/2, 52);
        pocketstar.print("Info");
        resetFlashTimer();
    } else if (currentMenu == MENU_GAMES) {
        pocketstar.setFont(pocketStar6pt);
        pocketstar.setBitDepth(BitDepth16);
        show(gameSelected, true);
    } else if (currentMenu == MENU_APPS) {
        pocketstar.setFont(pocketStar6pt);
        pocketstar.setBitDepth(BitDepth16);
        show(appSelected, false);
    } else if (currentMenu == MENU_SETTINGS) {
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.setBitDepth(BitDepth16);
        pocketstar.setFont(pocketStar7pt);
        pocketstar.setCursor(28, 0);
        pocketstar.setFontColor(GREEN_16b, BLACK_16b);
        pocketstar.print("Settings");
        curOption = 0;
        pocketstar.setFont(pocketStar6pt);
    } else if (currentMenu == MENU_INFO) {
        resetFlashTimer();
        currentPage = 0;
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.setBitDepth(BitDepth16);
        pocketstar.setFont(pocketStar6pt);
        pocketstar.setCursor(48 - pocketstar.getPrintWidth("Info")/2, 4);
        pocketstar.setFontColor(GREEN_16b, BLACK_16b);
        pocketstar.print("Info");
    } else if (currentMenu == MENU_DEVICE_INFO) {
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.setBitDepth(BitDepth16);
        pocketstar.setFont(pocketStar6pt);
        if (currentPage == 0) {
            pocketstar.setFontColor(GREEN_16b, BLACK_16b);
            pocketstar.setCursor(11, 5);
            pocketstar.print("Device Information");
            pocketstar.setFontColor(WHITE_16b, BLACK_16b);
            pocketstar.setCursor(5, 17);
            pocketstar.print("Model Type:");
            pocketstar.setCursor(5, 27);
            pocketstar.print(MODELTYPE);
            pocketstar.setCursor(5, 37);
            pocketstar.print(DATE);
            pocketstar.setFontColor(0x041F, BLACK_16b);
            pocketstar.setCursor(30, 53);
            pocketstar.print("Page 1/3");
        } else if (currentPage == 1) {
            pocketstar.setFontColor(GREEN_16b, BLACK_16b);
            pocketstar.setCursor(11, 5);
            pocketstar.print("Device Information");
            pocketstar.setFontColor(WHITE_16b, BLACK_16b);
            pocketstar.setCursor(5, 17);
            pocketstar.print("Menu version: ");
            pocketstar.print(MENUVERSION);
            pocketstar.setFontColor(0x041F, BLACK_16b);
            pocketstar.setCursor(30, 53);
            pocketstar.print("Page 2/3");
        } else if (currentPage == 2) {
            pocketstar.setFontColor(GREEN_16b, BLACK_16b);
            pocketstar.setCursor(11, 5);
            pocketstar.print("Device Information");
            pocketstar.setFontColor(0x041F, BLACK_16b);
            pocketstar.setCursor(30, 53);
            pocketstar.print("Page 3/3");
            drawColorPalette();
        }
    } else if (currentMenu == MENU_STATUS) {
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.setBitDepth(BitDepth16);
        pocketstar.setFont(pocketStar6pt);
        lastBatteryChange = 1000;
        if (currentPage == 0) {
            pocketstar.setFontColor(GREEN_16b, BLACK_16b);
            pocketstar.setCursor(34, 5);
            pocketstar.print("Status");
            pocketstar.setFontColor(0x041F, BLACK_16b);
            pocketstar.setCursor(30, 53);
            pocketstar.print("Page 1/2");
            lastBatteryChange = 1000;
        } else if (currentPage == 1) {
            pocketstar.setFontColor(GREEN_16b, BLACK_16b);
            pocketstar.setCursor(34, 5);
            pocketstar.print("Status");
            pocketstar.setFontColor(WHITE_16b, BLACK_16b);
            pocketstar.setCursor(5, 17);
            if (sdAviable) {
                pocketstar.print("SD card:");
                pocketstar.setCursor(5, 27);
                pocketstar.print(availableGames);
                pocketstar.print(" games");
                pocketstar.setCursor(5, 37);
                pocketstar.print(availableApps);
                pocketstar.print(" apps");
            } else if (pocketstar.isSDConnected()) {
                pocketstar.print("Failed to load SD.");
            } else {
                pocketstar.print("No SD Connected.");
            }
            pocketstar.setFont(pocketStar6pt);
            pocketstar.setFontColor(0x041F, BLACK_16b);
            pocketstar.setCursor(30, 53);
            pocketstar.print("Page 2/2");
        }
    } else if (currentMenu == MENU_CREDITS) {
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.setBitDepth(BitDepth16);
        pocketstar.setFont(pocketStar6pt);
        if (currentPage == 0) {
            pocketstar.setFontColor(GREEN_16b, BLACK_16b);
            pocketstar.setCursor(34, 5);
            pocketstar.print("Credits");
            pocketstar.setFontColor(WHITE_16b, BLACK_16b);
            pocketstar.setCursor(5, 17);
            pocketstar.print("Hardware by:");
            pocketstar.setCursor(5, 27);
            pocketstar.print("Gurkan Dogan");
            if (poorlilzep >= 5) {
                pocketstar.setFontColor(0x0841, BLACK_16b);
                pocketstar.setCursor(0, 57);
                pocketstar.print("175");
            }
            aouHelper(11, 28, WHITE_16b);
            gHelper(50, 27, WHITE_16b);
            pocketstar.setFontColor(0x041F, BLACK_16b);
            pocketstar.setCursor(30, 53);
            pocketstar.print("Page 1/4");
        } else if (currentPage == 1) {
            pocketstar.setFontColor(GREEN_16b, BLACK_16b);
            pocketstar.setCursor(34, 5);
            pocketstar.print("Credits");
            pocketstar.setFontColor(WHITE_16b, BLACK_16b);
            pocketstar.setCursor(5, 17);
            pocketstar.print("Main software by:");
            pocketstar.setCursor(5, 27);
            pocketstar.print("Florian Keller");
            pocketstar.setFontColor(0x041F, BLACK_16b);
            pocketstar.setCursor(30, 53);
            pocketstar.print("Page 2/4");
        } else if (currentPage == 2) {
            pocketstar.setFontColor(GREEN_16b, BLACK_16b);
            pocketstar.setCursor(34, 5);
            pocketstar.print("Credits");
            pocketstar.setFontColor(WHITE_16b, BLACK_16b);
            pocketstar.setCursor(5, 17);
            pocketstar.print("Special thanks to");
            pocketstar.setCursor(5, 27);
            pocketstar.print("Benjamin Beck");
            pocketstar.setCursor(5, 37);
            pocketstar.print("Onder Kefoglu");
            aouHelper(6, 36, WHITE_16b);
            gHelper(54, 37, WHITE_16b);
            pocketstar.setFontColor(0x041F, BLACK_16b);
            pocketstar.setCursor(30, 53);
            pocketstar.print("Page 3/4");
        } else if (currentPage == 3) {
            pocketstar.setFontColor(GREEN_16b, BLACK_16b);
            pocketstar.setCursor(34, 5);
            pocketstar.print("Credits");
            pocketstar.setFontColor(WHITE_16b, BLACK_16b);
            pocketstar.setCursor(48 - pocketstar.getPrintWidth("PocketStar (r)")/2, 24);
            pocketstar.print("PocketStar ");
            pocketstar.setFont(pocketStarSymbols);
            pocketstar.print("t"); // (r)
            pocketstar.setFont(pocketStar6pt);
            pocketstar.setCursor(48 - pocketstar.getPrintWidth("By Zepsch")/2, 34);
            pocketstar.print("By Zepsch");
            pocketstar.setFontColor(0x041F, BLACK_16b);
            pocketstar.setCursor(30, 53);
            pocketstar.print("Page 4/4");
        }
    } else if (currentMenu == MENU_SD_NOT_FOUND) {
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.setBitDepth(BitDepth16);
        pocketstar.setFont(pocketStar6pt);
        pocketstar.setFontColor(RED_16b, BLACK_16b);
        pocketstar.setCursor(3, 3);
        pocketstar.print("SD card not found.");
        pocketstar.setCursor(3, 12);
        pocketstar.print("Please insert a valid");
        pocketstar.setCursor(3, 21);
        pocketstar.print("SD card and press A");
        pocketstar.setCursor(3, 30);
        pocketstar.print("to try again.");
    } else if (currentMenu == MENU_SD_ERROR) {
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.setBitDepth(BitDepth16);
        pocketstar.setFont(pocketStar6pt);
        pocketstar.setFontColor(RED_16b, BLACK_16b);
        pocketstar.setCursor(3, 3);
        pocketstar.print("Failed to load SD.");
        pocketstar.setCursor(3, 12);
        pocketstar.print("Please insert a valid");
        pocketstar.setCursor(3, 21);
        pocketstar.print("SD card and press A");
        pocketstar.setCursor(3, 30);
        pocketstar.print("to try again.");
    } else if (currentMenu == MENU_EJECT_SD) {
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.setBitDepth(BitDepth16);
        pocketstar.setFont(pocketStar6pt);
        pocketstar.setFontColor(GREEN_16b, BLACK_16b);
        pocketstar.setCursor(3, 3);
        pocketstar.print("You can now remove");
        pocketstar.setCursor(3, 12);
        pocketstar.print("the SD card. Please");
        pocketstar.setCursor(3, 21);
        pocketstar.print("press A after");
        pocketstar.setCursor(3, 30);
        pocketstar.print("reinserting it to");
        pocketstar.setCursor(3, 39);
        pocketstar.print("restart the");
        pocketstar.setCursor(3, 48);
        pocketstar.print("PocketStar.");
    }
}

void aouHelper(uint8_t x, uint8_t y, uint16_t color) { // helps drawing a german / turkish a o u A O U with dots
    pocketstar.drawPixel(x, y, color);
    pocketstar.drawPixel(x + 3, y, color);
}

void gHelper(uint8_t x, uint8_t y, uint16_t color) { // helps drawing a turkish g with breve
    pocketstar.drawPixel(x, y, color);
    pocketstar.drawPixel(x + 1, y + 1, color);
    pocketstar.drawPixel(x + 2, y + 1, color);
    pocketstar.drawPixel(x + 3, y, color);
}

void calcInput(float deltaTime) {
    /*if (!pocketstar.getButtons()) {
        lockInput -= deltaTime * 5;
        return;
    }*/
    
    if (lockInput > 0) {
        lockInput -= deltaTime;
        return;
    }
    
    if (pocketstar.getButtons()) {
        lockInput = INPUTLOCKTIME;
    }
    
    if (currentMenu == MENU_TITLE) {
        if (pocketstar.getButtons()) {
            changeMenu(MENU_MAIN);
        }
    } else if (currentMenu == MENU_MAIN) {
        if (pocketstar.getButtons(ButtonLeft)) {
            if (mainX > 0) {
                mainX -= 1;
                resetFlashTimer();
            }
            if (nomaki == 4 || nomaki == 6)
                nomaki++;
            else
                nomaki = 0;
        } else if (pocketstar.getButtons(ButtonRight)) {
            if (mainX < 1) {
                mainX += 1;
                resetFlashTimer();
            }
            if (nomaki == 5 || nomaki == 7)
                nomaki++;
            else
                nomaki = 0;
        } else if (pocketstar.getButtons(ButtonUp)) {
            if (mainY > 0) {
                mainY -= 1;
                resetFlashTimer();
            }
            if (nomaki == 0 || nomaki == 1)
                nomaki++;
            else
                nomaki = 0;
        } else if (pocketstar.getButtons(ButtonDown)) {
            if (mainY < 1) {
                mainY += 1;
                resetFlashTimer();
            }
            if (nomaki == 2 || nomaki == 3)
                nomaki++;
            else
                nomaki = 0;
        } else if (pocketstar.getButtons(ButtonA)) {
            if (nomaki == 9) {
                pocketstar.setMirror(true);
                nomaki = 0;
                changeMenu(MENU_MAIN);
                return;
            }
            nomaki = 0;
            
            if (mainX == 0 && mainY == 0) {
                if (sdAviable)
                    changeMenu(MENU_GAMES);
                else if (pocketstar.isSDConnected())
                    changeMenu(MENU_SD_ERROR);
                else 
                    changeMenu(MENU_SD_NOT_FOUND);
            } else if (mainX == 1 && mainY == 0) {
                if (sdAviable)
                    changeMenu(MENU_APPS);
                else if (pocketstar.isSDConnected())
                    changeMenu(MENU_SD_ERROR);
                else 
                    changeMenu(MENU_SD_NOT_FOUND);
            } else if (mainX == 0 && mainY == 1) {
                changeMenu(MENU_SETTINGS);
            } else if (mainX == 1 && mainY == 1) {
                changeMenu(MENU_INFO);
            }
        } else if (pocketstar.getButtons(ButtonB)) {
            if (nomaki == 8)
                nomaki++;
            else {
                nomaki = 0;
                //changeMenu(MENU_TITLE);
            }
        }
    } else if (currentMenu == MENU_GAMES) {
        if (pocketstar.getButtons(ButtonLeft)) {
            switchRelative(true, true);
        } else if (pocketstar.getButtons(ButtonRight)) {
            switchRelative(false, true);
        } else if (pocketstar.getButtons(ButtonA)) {
            load(true);
        } else if (pocketstar.getButtons(ButtonB)) {
            changeMenu(MENU_MAIN);
        }
    } else if (currentMenu == MENU_APPS) {
        if (pocketstar.getButtons(ButtonLeft)) {
            switchRelative(true, false);
        } else if (pocketstar.getButtons(ButtonRight)) {
            switchRelative(false, false);
        } else if (pocketstar.getButtons(ButtonA)) {
            load(false);
        } else if (pocketstar.getButtons(ButtonB)) {
            changeMenu(MENU_MAIN);
        }
    } else if (currentMenu == MENU_SETTINGS) {
        if (pocketstar.getButtons(ButtonDown)) {
            if (curOption < 4)
                curOption++;
                resetFlashTimer();
        } else if (pocketstar.getButtons(ButtonUp)) {
            if (curOption > 0)
                curOption--;
                resetFlashTimer();
        }
        
        switch (curOption) {
            case 0: {
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
                }
                break;
            }
            case 1: {
                if (pocketstar.getButtons(ButtonRight | ButtonLeft)) {
                    _vibration = !_vibration;
                    pocketstar.vibrationEnabled = _vibration;
                    if (_vibration) {
                        pocketstar.vibrate(true);
                        _vibrationTimer = 0.1f;
                        _vibrating = true;
                    }
                    _saveSettings();
                }
                break;
            }
            case 2: {
                boolean changed = false;
                if (_ledInMenu < BacklightWhite && pocketstar.getButtons(ButtonRight)) {
                    _ledInMenu++;
                    changed = true;
                } else if (_ledInMenu > 0 && pocketstar.getButtons(ButtonLeft)) {
                    _ledInMenu--;
                    changed = true;
                }
                if (changed) {
                    pocketstar.setBacklightColor(_ledInMenu);
                    _saveSettings();
                }
                break;
            }
            case 3: {
                boolean changed = false;
                if (_standbyNr < 6 && pocketstar.getButtons(ButtonRight)) {
                    _standbyNr++;
                    changed = true;
                } else if (_standbyNr > 0 && pocketstar.getButtons(ButtonLeft)) {
                    _standbyNr--;
                    changed = true;
                }
                if (changed) {
                    _saveSettings();
                }
                break;
            }
            case 4: {
                if (pocketstar.getButtons(ButtonA))
                    changeMenu(MENU_EJECT_SD);
                break;
            }
        }
        if (pocketstar.getButtons(ButtonB)) {
            changeMenu(MENU_MAIN);
        }
    } else if (currentMenu == MENU_INFO) {
        if (pocketstar.getButtons(ButtonB)) {
            changeMenu(MENU_MAIN);
        } else if (pocketstar.getButtons(ButtonA)) {
            if (currentInfoSelect == 0) {
                changeMenu(MENU_DEVICE_INFO);
            } else if (currentInfoSelect == 1) {
                changeMenu(MENU_STATUS);
            } else if (currentInfoSelect == 2) {
                changeMenu(MENU_CREDITS);
            }
        } else if (pocketstar.getButtons(ButtonUp) && currentInfoSelect > 0) {
            currentInfoSelect -= 1;
            changeMenu(MENU_INFO);
        } else if (pocketstar.getButtons(ButtonDown) && currentInfoSelect < 2) {
            currentInfoSelect += 1;
            changeMenu(MENU_INFO);
        }
    } else if (currentMenu == MENU_DEVICE_INFO) {
        if (pocketstar.getButtons(ButtonB)) {
            changeMenu(MENU_INFO);
        } else if (pocketstar.getButtons(ButtonLeft) && currentPage > 0) {
            currentPage -= 1;
            changeMenu(MENU_DEVICE_INFO);
        } else if (pocketstar.getButtons(ButtonRight) && currentPage < infoPages - 1) {
            currentPage += 1;
            changeMenu(MENU_DEVICE_INFO);
        }
    } else if (currentMenu == MENU_STATUS) {
        if (pocketstar.getButtons(ButtonB)) {
            changeMenu(MENU_INFO);
        } else if (pocketstar.getButtons(ButtonLeft) && currentPage > 0) {
            currentPage -= 1;
            changeMenu(MENU_STATUS);
        } else if (pocketstar.getButtons(ButtonRight) && currentPage < statusPages - 1) {
            currentPage += 1;
            changeMenu(MENU_STATUS);
        }
    } else if (currentMenu == MENU_CREDITS) {
        if (pocketstar.getButtons(ButtonB)) {
            changeMenu(MENU_INFO);
        } else if (pocketstar.getButtons(ButtonLeft) && currentPage > 0) {
            currentPage -= 1;
            changeMenu(MENU_CREDITS);
        } else if (pocketstar.getButtons(ButtonRight) && currentPage < creditsPages - 1) {
            currentPage += 1;
            changeMenu(MENU_CREDITS);
        } else if (currentPage == 0 && poorlilzep < 5 && pocketstar.getButtons(ButtonUp)) {
            poorlilzep++;
            changeMenu(MENU_CREDITS);
        }
    } else if (currentMenu == MENU_SD_NOT_FOUND || currentMenu == MENU_SD_ERROR) {
        if (pocketstar.getButtons(ButtonA)) {
            NVIC_SystemReset();
        } else if (pocketstar.getButtons(ButtonB)) {
            changeMenu(MENU_MAIN);
        }
    } else if (currentMenu == MENU_EJECT_SD) {
        if (pocketstar.getButtons(ButtonA)) {
            NVIC_SystemReset();
        }
    }
}

void switchRelative(boolean next, boolean g) {
    if (g) {
        if (availableGames == 0) {
            show(0, g);
            return;
        }
        if (next)
            gameSelected = (gameSelected - 1 + availableGames) % availableGames;
        else
            gameSelected = (gameSelected + 1) % availableGames;
        show(gameSelected, g);
    } else {
        if (availableApps == 0) {
            show(0, g);
            return;
        }
        if (next)
            appSelected = (appSelected - 1 + availableApps) % availableApps;
        else
            appSelected = (appSelected + 1) % availableApps;
        show(appSelected, g);
    }
}

void show(int i, boolean g) {
    if (g && availableGames == 0) {
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.setCursor(0, 0);
        pocketstar.setBitDepth(BitDepth16);
        pocketstar.setFontColor(RED_16b, BLACK_16b);
        pocketstar.print("No game found.");
        return;
    }
    if (!g && availableApps == 0) {
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.setCursor(0, 0);
        pocketstar.setBitDepth(BitDepth16);
        pocketstar.setFontColor(RED_16b, BLACK_16b);
        pocketstar.print("No app found.");
        return;
    }
    
    if (g) games.vwd()->rewind();
    else apps.vwd()->rewind();
    
    boolean found = false;
    while (!found) {
        if (g) {
            gameDir.close();
            gameDir.openNext(games.vwd(), FILE_READ);
        } else {
            appDir.close();
            appDir.openNext(apps.vwd(), FILE_READ);
        }
        if (isValid(g)) {
            if (i == 0)
                found = true;
            i--;
        }
    }
    
    drawThumbnail(g);
}

boolean isValid(boolean g) {
    if (g && !gameDir.isDir())
        return false;
    if (!g && !appDir.isDir())
        return false;
    
    char path[33];
    char binPath[70];
    if (g) gameDir.getName(path, 32);
    else appDir.getName(path, 32);
    int pathLength = strlen(path);
    strcpy(binPath, path);
    //binPath[pathLength] = '/';
    strcpy(binPath + pathLength, "/");
    strcpy(binPath + pathLength + 1, path);
    strcpy(binPath + pathLength + 1 + pathLength, ".bin");
    
    if (g && !games.exists(binPath))
        return false;
    if (!g && !apps.exists(binPath))
        return false;
    
    return true;
}

void drawThumbnail(boolean g) {
    char path[33];
    char imgPath[70];
    if (g) gameDir.getName(path, 32);
    else appDir.getName(path, 32);
    int pathLength = strlen(path);
    strcpy(imgPath, path);
    imgPath[pathLength] = '/';
    strcpy(imgPath + pathLength + 1, path);
    strcpy(imgPath + pathLength * 2 + 1, ".psi");
    if ((g && !games.exists(imgPath)) || (!g && !apps.exists(imgPath))) {
        pocketstar.setBitDepth(BitDepth8);
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.drawImage(0, 56, &arrowLeftImg);
        pocketstar.drawImage(88, 56, &arrowRightImg);
        pocketstar.setBitDepth(BitDepth16);
        pocketstar.setFontColor(WHITE_16b, BLACK_16b);
        pocketstar.setCursor(0, 0);
        pocketstar.print(path);
        return;
    }
    
    File file;
    if (g)
        file = games.open(imgPath, FILE_READ);
    else
        file = apps.open(imgPath, FILE_READ);
    
    //  signature: PSI                                                    version: 2          width and height: 96 & 64
    if (file.read() != 'P' || file.read() != 'S' || file.read() != 'I' || file.read() != 2 || file.read() != 96 || file.read() != 64) {
        pocketstar.setBitDepth(BitDepth16);
        pocketstar.setFontColor(RED_16b, BLACK_16b);
        pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
        pocketstar.setCursor(0, 0);
        pocketstar.print(path);
        pocketstar.setCursor(0, 10);
        pocketstar.print("unable to read image");
        return;
    }
    boolean bitDepth16 = file.read() == 0;
    if (bitDepth16)
        pocketstar.setBitDepth(BitDepth16);
    else
        pocketstar.setBitDepth(BitDepth8);
    
    int datalines = 64;
    if (bitDepth16)
        datalines = datalines << 1;
    
    uint8_t buffer[96];
    pocketstar.setX(0, 95);
    pocketstar.setY(0, 63);
    pocketstar.startTransfer();
    for (int i = 0; i < datalines; i++) {
        file.read(&buffer, 96);
        pocketstar.writeBuffer(buffer, 96);
    }
    pocketstar.endTransfer();
    file.close();

    pocketstar.setBitDepth(BitDepth8);
    pocketstar.drawImage(0, 56, &arrowLeftImg);
    pocketstar.drawImage(88, 56, &arrowRightImg);
}

void drawColorPalette() {
    uint8_t steps = 16;
    for (int x = 0; x < 96; x++) {
        uint8_t red, green, blue;
        if (x <= 16) {
            red = 255;
            green = 255 * x / 16;
            blue = 0;
        } else if (x <= 32) {
            red = 255 * (32 - x) / 16;
            green = 255;
            blue = 0;
        } else if (x <= 48) {
            red = 0;
            green = 255;
            blue = 255 * (x - 32) / 16;
        } else if (x <= 64) {
            red = 0;
            green = 255 * (64 - x) / 16;
            blue = 255;
        } else if (x <= 80) {
            red = 255 * (x - 64) / 16;
            green = 0;
            blue = 255;
        } else { // x <= 96
            red = 255;
            green = 0;
            blue = 255 * (96 - x) / 16;
        }
        
        for (int y = 0; y < steps; y++) {
            uint8_t redDark = red * (steps - 1 - y) / (steps - 1);
            uint8_t greenDark = green * (steps - 1 - y) / (steps - 1);
            uint8_t blueDark = blue * (steps - 1 - y) / (steps - 1);
            uint16_t colorDark = ((redDark >> 3) << 11) | ((greenDark >> 2) << 5) | (blueDark >> 3);
            pocketstar.drawPixel(x, 32 + y, colorDark);
            
            uint8_t redBright = red + (255 - red) * y / (steps - 1);
            uint8_t greenBright = green + (255 - green) * y / (steps - 1);
            uint8_t blueBright = blue + (255 - blue) * y / (steps - 1);
            uint16_t colorBright = ((redBright >> 3) << 11) | ((greenBright >> 2) << 5) | (blueBright >> 3);
            pocketstar.drawPixel(x, 31 - y, colorBright);
        }
    }
}

boolean pause() {
    return false;
}

void resume() {
    pocketstar.setBacklightColor(_ledInMenu);
    changeMenu(currentMenu);
}
