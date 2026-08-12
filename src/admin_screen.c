#include "admin_screen.h"
#include "pinpad/touch.h"
#include "delay.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"

#define BACKGROUND_COLOR  NAVY
#define TITLE_COLOR       GREEN
#define TEXT_COLOR        WHITE
#define BUTTON_COLOR      DARK_GRAY
#define BORDER_COLOR      CYAN

// same tuning as pinpad_screen.c - keeps touch feel consistent across screens
#define TOUCH_THRESHOLD   2
#define DEBOUNCE_SAMPLES  4

void drawButton(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, const char *label);

static int active = 0;

// debounce state
static int raw_touching = 0;
static int stable_count = 0;
static int confirmed_touching = 0;

// -1 = no option under this point, 1 = CONTROL, 0 = USER PROFILE
static int coordinates_to_option(int x, int y)
{
    if (x < 16 || x > 222)
        return -1;

    if (y >= 120 && y <= 200)
        return 1;
    if (y >= 208 && y <= 288)
        return 0;

    return -1;
}

static void drawAdminScreen(void)
{
    // clear the screen and make the background navy
    lcd_fillScreen(BACKGROUND_COLOR);

    // display the ADMIN heading
    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(99, 35, (unsigned char *)"ADMIN");

    // draw the green line underneath the heading
    lcd_line(74, 49, 166, 49, TITLE_COLOR);

    // admin control / user profile buttons
    drawButton(16, 120, 222, 200, "CONTROL");
    drawButton(16, 208, 222, 288, "USER PROFILE");
}

AdminResult admin_screen_step(void)
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
    AdminResult result;

    if (!active) {
        raw_touching = 0;
        stable_count = 0;
        confirmed_touching = 0;
        drawAdminScreen();
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

    result = ADMIN_IN_PROGRESS;

    if (stable_count >= DEBOUNCE_SAMPLES && confirmed_touching != raw_touching) {
        confirmed_touching = raw_touching;

        if (confirmed_touching) {
            option = coordinates_to_option(screen_x, screen_y);
            if (option == 1) {
                result = ADMIN_SELECTED_CONTROL;
            } else if (option == 0) {
                result = ADMIN_SELECTED_PROFILE;
            }
        }
    }

    if (result != ADMIN_IN_PROGRESS) {
        // next call starts a fresh screen
        active = 0;
    }

    return result;
}
