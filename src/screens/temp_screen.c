#include "temp_screen.h"
#include "pinpad/touch.h"
#include "../homestate.h"
#include "../users.h"
#include "../temperature.h"
#include "../lcd/lcd_hw.h"
#include "../lcd/lcd_grph.h"
#include "../lcd/lcd_cfg.h"
#include "../lcd/sdram.h"

#define BACKGROUND_COLOR  NAVY
#define TITLE_COLOR       GREEN
#define TEXT_COLOR        WHITE
#define BUTTON_COLOR      DARK_GRAY
#define BORDER_COLOR      CYAN

// HOME button, top-left corner
#define BACK_X0  4
#define BACK_Y0  4
#define BACK_X1  68
#define BACK_Y1  50

void drawButton(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, const char *label);
unsigned int getTextLength(const char *text);

static int active = 0;
static int displaySetPoint = 0;

static TouchDebounceState touchState;

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

// redraws the current-temperature value box, handling negative readings
// (read_temp() returns -10 to 50, unlike the other 0-99 value boxes)
static void drawCurrentTemp(int value)
{
    unsigned char text[4];
    int i = 0;
    unsigned int mag = (value < 0) ? (unsigned int)(-value) : (unsigned int)value;

    if (value < 0)
        text[i++] = '-';
    text[i++] = (unsigned char)('0' + mag / 10);
    text[i++] = (unsigned char)('0' + mag % 10);
    text[i] = '\0';

    lcd_fillRect(172, 54, 208, 74, BACKGROUND_COLOR);
    lcd_drawRect(172, 54, 208, 74, BORDER_COLOR);
    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString(178, 60, text);
}

static void drawTemperatureScreen(void)
{
    lcd_fillScreen(BACKGROUND_COLOR);

    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(87, 35, (unsigned char *)"TEMPERATURE");
    lcd_line(74, 49, 166, 49, TITLE_COLOR);

    drawButton(BACK_X0, BACK_Y0, BACK_X1, BACK_Y1, "HOME");

    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString(16, 60, (unsigned char *)"Current:");
    drawCurrentTemp(read_temp());

    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString(16, 90, (unsigned char *)"Set:");
    if (homeState.tempSetPoint >= users[0].tempLowLimit && homeState.tempSetPoint <= users[0].tempHighLimit)
        displaySetPoint = homeState.tempSetPoint;
    else
        displaySetPoint = users[0].tempLowLimit;
    drawValue(172, 84, 208, 104, displaySetPoint);

    drawButton(16, 110, 118, 146, "+");
    drawButton(122, 110, 224, 146, "-");

    // users[0] is the seeded admin account (see src/users.c) - the limits
    // set on the CONTROLS screen
    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString(16, 160, (unsigned char *)"LO:");
    drawValue(40, 154, 76, 174, users[0].tempLowLimit);
    lcd_putString(100, 160, (unsigned char *)"HI:");
    drawValue(124, 154, 160, 174, users[0].tempHighLimit);
}

// -1 = nothing, 0 = HOME, 1 = SET+, 2 = SET-
static int coordinates_to_option(int x, int y)
{
    if (x >= BACK_X0 && x <= BACK_X1 && y >= BACK_Y0 && y <= BACK_Y1)
        return 0;

    if (y >= 110 && y <= 146) {
        if (x >= 16 && x <= 118) return 1;
        if (x >= 122 && x <= 224) return 2;
    }

    return -1;
}

TempResult temp_screen_step(void)
{
    int screen_x;
    int screen_y;
    int option;
    TempResult result;

    // first call after a reset, draw the screen from scratch
    if (!active) {
        touch_debounce_init(&touchState);
        drawTemperatureScreen();
        active = 1;
    }

    result = TEMP_IN_PROGRESS;

    // got a touch this poll, work out which button it landed on
    if (touch_poll_press(&touchState, &screen_x, &screen_y)) {
        option = coordinates_to_option(screen_x, screen_y);

        if (option == 0) {
            result = TEMP_SELECTED_HOME;
        } else if (option == 1) {
            // SET+ - bump the displayed set point up to the admin's HI limit, save on press
            if (displaySetPoint < users[0].tempHighLimit) {
                displaySetPoint++;
                homeState.tempSetPoint = displaySetPoint;
                drawValue(172, 84, 208, 104, displaySetPoint);
            }
        } else if (option == 2) {
            // SET- - drop the displayed set point down to the admin's LO limit, save on press
            if (displaySetPoint > users[0].tempLowLimit) {
                displaySetPoint--;
                homeState.tempSetPoint = displaySetPoint;
                drawValue(172, 84, 208, 104, displaySetPoint);
            }
        }
    }

    // next call starts a fresh screen
    if (result != TEMP_IN_PROGRESS) {
        active = 0;
    }

    return result;
}
