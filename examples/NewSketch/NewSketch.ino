#include <PocketStar.h>

void initialize() {
    // put your initialization code here - runs once
}

void update(float tpf) {
    // put your main code here - gets repeated endlessly
    // 'tpf' is the time that has passed since the last call
}

boolean pause() {
    // gets called before the pause menu is displayed or standby mode is activated
    // return false to prevent the pause menu from being displayed
    // be aware that the pause menu must be availble at some point to be able to return to the menu
    return true;
}

void resume() {
    // gets called after leaving pause menu or standby or screen
    // redraw the entire screen to ensure the pause menu is no longer visible
}
