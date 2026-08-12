#include "homescreen.h"
#include "pinpad/touch.h"
#include "../delay.h"
#include "../lcd/lcd_hw.h"
#include "../lcd/lcd_grph.h"
#include "../lcd/lcd_cfg.h"
#include "../lcd/sdram.h"

#define BACKGROUND_COLOR  NAVY
#define TITLE_COLOR       GREEN
#define TEXT_COLOR        WHITE
#define BUTTON_COLOR      DARK_GRAY
#define BORDER_COLOR      CYAN

// same tuning as the other touch screens - keeps touch feel consistent
#define TOUCH_THRESHOLD   2
#define DEBOUNCE_SAMPLES  4

// BACK button, top-left corner
#define BACK_X0  8
#define BACK_Y0  8
#define BACK_X1  60
#define BACK_Y1  30

// 2x2 grid of feature buttons
#define GRID_X0    16
#define GRID_X_MID 120
#define GRID_X1    224
#define GRID_Y0    140
#define GRID_Y_MID 214
#define GRID_Y1    288

void drawButton(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, const char *label);
unsigned int getTextLength(const char *text);

static int active = 0;

// debounce state
static int raw_touching = 0;
static int stable_count = 0;
static int confirmed_touching = 0;

// -1 = no option under this point, otherwise a HomeResult value
// (excluding HOME_IN_PROGRESS)
static int coordinates_to_option(int x, int y)
{
    int column;
    int row;

    if (x >= BACK_X0 && x <= BACK_X1 && y >= BACK_Y0 && y <= BACK_Y1)
        return HOME_SELECTED_BACK;

    if (x >= GRID_X0 && x < GRID_X_MID)
        column = 0;
    else if (x >= GRID_X_MID && x <= GRID_X1)
        column = 1;
    else
        return -1;

    if (y >= GRID_Y0 && y < GRID_Y_MID)
        row = 0;
    else if (y >= GRID_Y_MID && y <= GRID_Y1)
        row = 1;
    else
        return -1;

    if (row == 0)
        return (column == 0) ? HOME_SELECTED_LIGHT : HOME_SELECTED_BLIND;
    return (column == 0) ? HOME_SELECTED_COFFEE : HOME_SELECTED_TEMP;
}

// builds "HELLO ####!" into dest, dest must be large enough
// (6 for "HELLO ", up to strlen(userId), 1 for '!', 1 for the null)
static void buildGreeting(char *dest, const char *userId)
{
    static const char prefix[] = "HELLO ";
    unsigned int i;
    unsigned int j;

    for (i = 0; prefix[i] != '\0'; i++)
        dest[i] = prefix[i];

    for (j = 0; userId[j] != '\0'; j++)
        dest[i++] = userId[j];

    dest[i++] = '!';
    dest[i] = '\0';
}

static void drawHomeScreen(const char *userId)
{
    char greeting[16];
    unsigned int length;
    unsigned short textX;

    buildGreeting(greeting, userId);

    // clear the screen and make the background navy
    lcd_fillScreen(BACKGROUND_COLOR);

    // BACK button
    drawButton(BACK_X0, BACK_Y0, BACK_X1, BACK_Y1, "BACK");

    // "HELLO ####!" heading, centered
    length = getTextLength(greeting);
    textX = (unsigned short)((DISPLAY_WIDTH - (length * 6U)) / 2U);
    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(textX, 60, (unsigned char *)greeting);

    // green line underneath the heading
    lcd_line(40, 74, 200, 74, TITLE_COLOR);

    // 2x2 grid of feature buttons
    drawButton(GRID_X0, GRID_Y0, GRID_X_MID - 1, GRID_Y_MID - 1, "LIGHT");
    drawButton(GRID_X_MID + 1, GRID_Y0, GRID_X1, GRID_Y_MID - 1, "BLIND");
    drawButton(GRID_X0, GRID_Y_MID + 1, GRID_X_MID - 1, GRID_Y1, "COFFEE");
    drawButton(GRID_X_MID + 1, GRID_Y_MID + 1, GRID_X1, GRID_Y1, "TEMP");
}

HomeResult home_screen_step(const char *userId)
{
    unsigned char x = 0;
    unsigned char y = 0;
    unsigned char z1;
    unsigned char z2;
    int pressure;
    int touching;
    int screen_x;
    int screen_y;
    int option;
    HomeResult result;

    if (!active) {
        raw_touching = 0;
        stable_count = 0;
        confirmed_touching = 0;
        drawHomeScreen(userId);
        active = 1;
    }

    // small settle delay between samples - lets the resistive panel/SPI
    // reading stabilise instead of hammering it as fast as possible
    mdelay(2);

    touch_read_xy((char *)&x, (char *)&y);
    z1 = touch_read(0xB8);
    z2 = touch_read(0xC8);

    // raw touch reads are 0-255; scale into screen-pixel space before
    // testing against button hitboxes
    screen_x = ((int)x * DISPLAY_WIDTH) / 255;
    screen_y = ((int)y * DISPLAY_HEIGHT) / 255;

    if (z1 == 0) {
        pressure = 0;
    } else {
        pressure = (int)(400.0f * ((float)x / 256.0f) *
            (((float)z2 / (float)z1) - 1.0f));
    }

    touching = (pressure > TOUCH_THRESHOLD) ? 1 : 0;

    // debounce: only accept a state change once it has been consistent
    // for DEBOUNCE_SAMPLES consecutive reads
    if (touching == raw_touching) {
        if (stable_count < DEBOUNCE_SAMPLES)
            stable_count++;
    } else {
        raw_touching = touching;
        stable_count = 1;
    }

    result = HOME_IN_PROGRESS;

    if (stable_count >= DEBOUNCE_SAMPLES && confirmed_touching != raw_touching) {
        confirmed_touching = raw_touching;

        if (confirmed_touching) {
            option = coordinates_to_option(screen_x, screen_y);
            if (option != -1) {
                result = (HomeResult)option;
            }
        }
    }

    if (result != HOME_IN_PROGRESS) {
        // next call starts a fresh screen
        active = 0;
    }

    return result;
}
