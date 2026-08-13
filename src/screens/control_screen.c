#include "control_screen.h"
#include "pinpad/touch.h"
#include "../users.h"
#include "../lcd/lcd_hw.h"
#include "../lcd/lcd_grph.h"
#include "../lcd/lcd_cfg.h"
#include "../lcd/sdram.h"

#define BACKGROUND_COLOR  NAVY
#define TITLE_COLOR       GREEN
#define TEXT_COLOR        WHITE
#define BORDER_COLOR      CYAN

void drawButton(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, const char *label);

static int active = 0;

static TouchDebounceState touchState;

// -1 = nothing, 0 = BACK, 1 = HI+, 2 = HI-, 3 = LO+, 4 = LO-
static int coordinates_to_option(int x, int y)
{
    // BACK button, top-left (same spot as the BACK button on the home screen)
    if (x >= 8 && x <= 60 && y >= 8 && y <= 30)
        return 0;

    if (y >= 80 && y <= 110) {
        if (x >= 16 && x <= 118) return 1;
        if (x >= 122 && x <= 222) return 2;
    }

    if (y >= 150 && y <= 180) {
        if (x >= 16 && x <= 118) return 3;
        if (x >= 122 && x <= 222) return 4;
    }

    return -1;
}

// keep the value a plain 2-digit number so it always fits its box
static int clampTemp(int value)
{
    if (value < 0) return 0;
    if (value > 99) return 99;
    return value;
}

// redraws just the number inside a value box (no full-screen redraw)
static void drawValue(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, int value)
{
    unsigned char text[3];
    text[0] = (unsigned char)('0' + value / 10);
    text[1] = (unsigned char)('0' + value % 10);
    text[2] = '\0';

    lcd_fillRect(x0, y0, x1, y1, BACKGROUND_COLOR);
    lcd_drawRect(x0, y0, x1, y1, BORDER_COLOR);
    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString(x0 + 12, y0 + 6, text);
}

static void drawControlScreen(void)
{
    lcd_fillScreen(BACKGROUND_COLOR);

    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(96, 35, (unsigned char *)"CONTROLS");
    lcd_line(74, 49, 166, 49, TITLE_COLOR);

    drawButton(8, 8, 60, 30, "BACK");

    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString(16, 60, (unsigned char *)"HI Temp:");
    drawValue(170, 54, 206, 74, currentUser->tempHighLimit);
    drawButton(16, 80, 118, 110, "+");
    drawButton(122, 80, 222, 110, "-");

    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString(16, 130, (unsigned char *)"LO Temp:");
    drawValue(170, 124, 206, 144, currentUser->tempLowLimit);
    drawButton(16, 150, 118, 180, "+");
    drawButton(122, 150, 222, 180, "-");
}

ControlResult control_screen_step(void)
{
    int screen_x;
    int screen_y;
    int option;
    ControlResult result;

    // first call after a reset, draw the screen from scratch
    if (!active) {
        touch_debounce_init(&touchState);
        drawControlScreen();
        active = 1;
    }

    result = CONTROL_IN_PROGRESS;

    // got a touch this poll, work out which button it landed on
    if (touch_poll_press(&touchState, &screen_x, &screen_y)) {
        option = coordinates_to_option(screen_x, screen_y);

        if (option == 0) {
            result = CONTROL_SELECTED_BACK;
        } else if (option == 1) {
            // HI+ - bump the high limit and redraw just that number
            currentUser->tempHighLimit = clampTemp(currentUser->tempHighLimit + 1);
            drawValue(170, 54, 206, 74, currentUser->tempHighLimit);
        } else if (option == 2) {
            // HI- - drop the high limit and redraw just that number
            currentUser->tempHighLimit = clampTemp(currentUser->tempHighLimit - 1);
            drawValue(170, 54, 206, 74, currentUser->tempHighLimit);
        } else if (option == 3) {
            // LO+ - bump the low limit and redraw just that number
            currentUser->tempLowLimit = clampTemp(currentUser->tempLowLimit + 1);
            drawValue(170, 124, 206, 144, currentUser->tempLowLimit);
        } else if (option == 4) {
            // LO- - drop the low limit and redraw just that number
            currentUser->tempLowLimit = clampTemp(currentUser->tempLowLimit - 1);
            drawValue(170, 124, 206, 144, currentUser->tempLowLimit);
        }
    }

    // next call starts a fresh screen
    if (result != CONTROL_IN_PROGRESS) {
        active = 0;
    }

    return result;
}
