#include "interface.h"
#include "SfloBruce_Touchscreen.h"
#include "core/powerSave.h"
#include "core/utils.h"

#define SFLOBRUCE_DISPLAY_HOR_RES_MAX 320
#define SFLOBRUCE_DISPLAY_VER_RES_MAX 240

SFLOBRUCE_Touch touch(SFLOBRUCE_DISPLAY_HOR_RES_MAX, SFLOBRUCE_DISPLAY_VER_RES_MAX);
SPIClass mySPI;

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() {
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);

    if (!touch.begin()) {
        log_i("Touch IC not Started");
    } else {
        log_i("Touch IC Started");
    }
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
void _post_setup_gpio() {
    pinMode(TFT_BL, OUTPUT);
    ledcSetup(0, 500, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, 255);
}

/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() { return 0; }

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    int dutyCycle;
    if (brightval == 100) dutyCycle = 255;
    else if (brightval == 75) dutyCycle = 130;
    else if (brightval == 50) dutyCycle = 70;
    else if (brightval == 25) dutyCycle = 20;
    else if (brightval == 0) dutyCycle = 0;
    else dutyCycle = ((brightval * 255) / 100);

    log_i("dutyCycle for bright 0-255: %d", dutyCycle);
    ledcWrite(0, dutyCycle); // Channel 0
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    static long tmp = 0;

    if (millis() - tmp > 200) {
        checkPowerSaveTime();

        if (touch.touched()) {
            auto t = touch.getPointScaled();
            t = touch.getPointScaled();

            if (bruceConfig.rotation == 3) {
                t.y = (tftHeight + 20) - t.y;
                t.x = tftWidth - t.x;
            }
            if (bruceConfig.rotation == 0) {
                int tmp = t.x;
                t.x = tftWidth - t.y;
                t.y = tmp;
            }
            if (bruceConfig.rotation == 2) {
                int tmp = t.x;
                t.x = t.y;
                t.y = (tftHeight + 20) - tmp;
            }

            // log_i("Touch Pressed on x=%d, y=%d\n", t.x, t.y);

            if (!wakeUpScreen()) {
                AnyKeyPress = true;
            } else {
                goto END;
            }

            touchPoint.x = t.x;
            touchPoint.y = t.y;
            touchPoint.pressed = true;
            touchHeatMap(touchPoint);

            tmp = millis();
        }
    }

END:
    delay(0);
}

/*********************************************************************
** Function: keyboard
** location: mykeyboard.cpp
** Starts keyboard to type data
**********************************************************************/
// String keyboard(String mytext, int maxSize, String msg) { }

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() {}

/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to tornoff the device (name is odd btw)
**********************************************************************/
void checkReboot() {}

/*********************************************************************
** Function: _checkNextPagePress
** location: mykeyboard.cpp
** returns the key from the keyboard
**********************************************************************/
bool _checkNextPagePress() { return false; }

/*********************************************************************
** Function: _checkPrevPagePress
** location: mykeyboard.cpp
** returns the key from the keyboard
**********************************************************************/
bool _checkPrevPagePress() { return false; }
