#include "coffee_screen.h"
#include "lpc24xx.h"
#include "homestate.h"
#include "clock.h"
#include "screens/pinpad/touch.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"

#define BACKGROUND_COLOR  NAVY
#define TEXT_COLOR        WHITE
#define BUTTON_COLOR      DARK_GRAY
#define BORDER_COLOR      CYAN

// HOME button, top-left corner
#define BACK_X0  4
#define BACK_Y0  4
#define BACK_X1  68
#define BACK_Y1  50

#define OVERRIDE_BTN (1 << 11) // P0.11, hardware-debounced push button (coffee manual override)

#define COFFEE_X0    16
#define COFFEE_X1    224
#define COFFEE_Y0    100
#define COFFEE_Y1    150

#define SCHEDULE_X0  16
#define SCHEDULE_X1  224
#define SCHEDULE_Y0  170
#define SCHEDULE_Y1  220

#define TIME_Y       290

void drawButton(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, const char *label);
unsigned int getTextLength(const char *text);

static int active = 0;
static int overridePrev = 0;

static TouchDebounceState touchState;

static int shownHour = -1;
static int shownMinute = -1;

// redraws the current time text in the bottom row, only when it has changed
static void drawTime(void)
{
    unsigned char text[6];
    unsigned int textX;

    if (shownHour == simHour && shownMinute == simMinute)
        return;

    shownHour = simHour;
    shownMinute = simMinute;

    text[0] = (unsigned char)('0' + shownHour / 10);
    text[1] = (unsigned char)('0' + shownHour % 10);
    text[2] = ':';
    text[3] = (unsigned char)('0' + shownMinute / 10);
    text[4] = (unsigned char)('0' + shownMinute % 10);
    text[5] = '\0';

    textX = (unsigned int)((COFFEE_X1 - COFFEE_X0 + 1 - 5U * 6U) / 2U) + COFFEE_X0;

    lcd_fillRect(COFFEE_X0, TIME_Y, COFFEE_X1, (unsigned short)(TIME_Y + 8), BACKGROUND_COLOR);
    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString((unsigned short)textX, TIME_Y, text);
}

// redraws just the COFFEE button, yellow when on, gray when off (same as drawLightAt in lights_screen.c)
static void drawCoffeeButton(void)
{
    int isOn = (homeState.roomLights & COFFEE_STATUS) != 0;
    unsigned short bg = isOn ? YELLOW : BUTTON_COLOR;
    unsigned short fg = isOn ? BLACK : TEXT_COLOR;
    unsigned int labelLength = getTextLength("COFFEE");
    unsigned short textX = (unsigned short)(COFFEE_X0 + (((COFFEE_X1 - COFFEE_X0 + 1) - labelLength * 6U) / 2U));
    unsigned short textY = (unsigned short)(COFFEE_Y0 + (((COFFEE_Y1 - COFFEE_Y0 + 1) - 8U) / 2U));

    lcd_fillRect(COFFEE_X0, COFFEE_Y0, COFFEE_X1, COFFEE_Y1, bg);
    lcd_drawRect(COFFEE_X0, COFFEE_Y0, COFFEE_X1, COFFEE_Y1, BORDER_COLOR);
    lcd_fontColor(fg, bg);
    lcd_putString(textX, textY, (unsigned char *)"COFFEE");
}

static void drawCoffeeScreen(void)
{
    lcd_fillScreen(BACKGROUND_COLOR);

    drawButton(BACK_X0, BACK_Y0, BACK_X1, BACK_Y1, "HOME");

    drawCoffeeButton();

    drawButton(SCHEDULE_X0, SCHEDULE_Y0, SCHEDULE_X1, SCHEDULE_Y1, "COFFEE SCHEDULE");

    shownHour = -1;
    shownMinute = -1;
    drawTime();
}

// -1 = nothing, 0 = HOME, 1 = COFFEE, 2 = COFFEE SCHEDULE
static int coordinates_to_option(int x, int y)
{
    if (x >= BACK_X0 && x <= BACK_X1 && y >= BACK_Y0 && y <= BACK_Y1)
        return 0;

    if (x >= COFFEE_X0 && x <= COFFEE_X1 && y >= COFFEE_Y0 && y <= COFFEE_Y1)
        return 1;

    if (x >= SCHEDULE_X0 && x <= SCHEDULE_X1 && y >= SCHEDULE_Y0 && y <= SCHEDULE_Y1)
        return 2;

    return -1;
}

CoffeeResult coffee_screen_step(void)
{
    int screen_x;
    int screen_y;
    int option;
    CoffeeResult result;

    // first call after a reset, draw the screen from scratch
    if (!active) {
        touch_debounce_init(&touchState);
        drawCoffeeScreen();
        active = 1;
    }

    result = COFFEE_IN_PROGRESS;

    // update the clock display every poll, not just on a touch
    drawTime();

    // hardware override button - toggle the coffee machine on the press edge
    {
        int overrideNow = (FIO0PIN & OVERRIDE_BTN) != 0;
        if (overrideNow && !overridePrev) {
            homeState.roomLights ^= COFFEE_STATUS;
            drawCoffeeButton();
        }
        overridePrev = overrideNow;
    }

    // got a touch this poll, work out which button it landed on
    if (touch_poll_press(&touchState, &screen_x, &screen_y)) {
        option = coordinates_to_option(screen_x, screen_y);

        if (option == 0) {
            result = COFFEE_SELECTED_HOME;
        } else if (option == 1) {
            // toggle the coffee machine on/off
            homeState.roomLights ^= COFFEE_STATUS;
            drawCoffeeButton();
        } else if (option == 2) {
            result = COFFEE_SELECTED_SCHEDULE;
        }
    }

    // next call starts a fresh screen
    if (result != COFFEE_IN_PROGRESS) {
        active = 0;
    }

    return result;
}

void drawButton(unsigned short x0,
                unsigned short y0,
                unsigned short x1,
                unsigned short y1,
                const char *label)
{
    unsigned int labelLength;
    unsigned short buttonWidth;
    unsigned short buttonHeight;
    unsigned short textX;
    unsigned short textY;

    // fill the inside of the button
    lcd_fillRect(x0, y0, x1, y1, BUTTON_COLOR);

    // draw the border around the button
    lcd_drawRect(x0, y0, x1, y1, BORDER_COLOR);

    // find how many characters are in the button label
    labelLength = getTextLength(label);

    // calculate the width and height of the button
    buttonWidth = x1 - x0 + 1;
    buttonHeight = y1 - y0 + 1;

    // calculate where the label should be placed
    textX = x0 + ((buttonWidth - (labelLength * 6U)) / 2U);
    textY = y0 + ((buttonHeight - 8U) / 2U);

    // display the label in the middle of the button
    lcd_fontColor(TEXT_COLOR, BUTTON_COLOR);
    lcd_putString(textX, textY, (unsigned char *)label);
}

unsigned int getTextLength(const char *text)
{
    unsigned int length;

    length = 0;

    // count each character until the end of the text
    while (text[length] != '\0')
    {
        length++;
    }

    return length;
}
