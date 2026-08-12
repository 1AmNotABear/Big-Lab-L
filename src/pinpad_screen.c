#include "pinpad_screen.h"
#include "pinpad/touch.h"
#include "users.h"
#include "delay.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"
#include <stdlib.h>

#define PIN_LENGTH 4

#define BACKGROUND_COLOR  NAVY
#define TITLE_COLOR       GREEN
#define TEXT_COLOR        WHITE
#define BUTTON_COLOR      DARK_GRAY
#define BORDER_COLOR      CYAN
#define ENTRY_COLOR       BLACK

// coordinates_to_key() return values for the non-digit keys
#define KEY_NONE  (-1)
#define KEY_CLEAR (-2)
#define KEY_OK    (-3)

// a touch only counts once its pressure reading has been on the same side
// of the threshold for this many consecutive samples - filters contact
// bounce / SPI noise that was causing double/phantom key presses
#define TOUCH_THRESHOLD   2
#define DEBOUNCE_SAMPLES  4

void drawButton(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, const char *label);
unsigned int getTextLength(const char *text);

static unsigned char entered_pin[PIN_LENGTH];
static int counter = 0;
static int active = 0;

// debounce state
static int raw_touching = 0;
static int stable_count = 0;
static int confirmed_touching = 0;

static int coordinates_to_key(int x, int y)
{
    int column;
    int row;

    if (x <= 72)
        column = 0;
    else if (x >= 91 && x <= 147)
        column = 1;
    else if (x >= 166 && x <= 222)
        column = 2;
    else
        return KEY_NONE;

    if (y >= 120 && y <= 156)
        row = 0;
    else if (y >= 164 && y <= 200)
        row = 1;
    else if (y >= 208 && y <= 244)
        row = 2;
    else if (y >= 245)
        row = 3;
    else
        return KEY_NONE;

    if (row == 0)
        return column + 1;
    if (row == 1)
        return column + 4;
    if (row == 2)
        return column + 7;
    if (row == 3) {
        if (column == 0)
            return KEY_CLEAR;
        if (column == 1)
            return 0;
        if (column == 2)
            return KEY_OK;
    }

    return KEY_NONE;
}

// redraw just the entry box (the black box that shows the digits typed so far)
static void resetEntryDisplay(void)
{
    lcd_fillRect(55, 77, 185, 106, ENTRY_COLOR);
    lcd_drawRect(55, 77, 185, 106, BORDER_COLOR);
    lcd_fontColor(TEXT_COLOR, ENTRY_COLOR);
    lcd_putString(108, 89, (unsigned char *)"____");
}

static void clearEntry(void)
{
    int i;
    for (i = 0; i < PIN_LENGTH; i++)
        entered_pin[i] = 0;
    counter = 0;
    resetEntryDisplay();
}

static void drawLoginScreen(void)
{
    // clear the screen and make the background navy
    lcd_fillScreen(BACKGROUND_COLOR);

    // display the USER ID heading
    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(99, 35, (unsigned char *)"USER ID");

    // draw the green line underneath the heading
    lcd_line(74, 49, 166, 49, TITLE_COLOR);

    // display the instruction above the input box
    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString(78, 60, (unsigned char *)"ENTER 4 DIGITS");

    // draw the entry box where the entered PIN will appear
    resetEntryDisplay();

    // first second third fourth keypad rows
    drawButton(16, 120, 72, 156, "1");
    drawButton(91, 120, 147, 156, "2");
    drawButton(166, 120, 222, 156, "3");

    drawButton(16, 164, 72, 200, "4");
    drawButton(91, 164, 147, 200, "5");
    drawButton(166, 164, 222, 200, "6");

    drawButton(16, 208, 72, 244, "7");
    drawButton(91, 208, 147, 244, "8");
    drawButton(166, 208, 222, 244, "9");

    drawButton(16, 252, 72, 288, "CLR");
    drawButton(91, 252, 147, 288, "0");
    drawButton(166, 252, 222, 288, "OK");
}

PinpadResult pinpad_screen_step(void)
{
    unsigned char x = 0;
    unsigned char y = 0;
    unsigned char z1;
    unsigned char z2;
    int pressure;
    int touching;
    int key;
    int screen_x;
    int screen_y;
    unsigned char digit_text[2];
    int i;
    int submit_requested;
    char attempt[PIN_LENGTH + 1];
    UserSettings *matched;
    PinpadResult result;

    if (!active) {
        clearEntry();
        raw_touching = 0;
        stable_count = 0;
        confirmed_touching = 0;
        drawLoginScreen();
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

    submit_requested = 0;

    if (stable_count >= DEBOUNCE_SAMPLES && confirmed_touching != raw_touching) {
        confirmed_touching = raw_touching;

        if (confirmed_touching) {
            key = coordinates_to_key(screen_x, screen_y);

            if (key >= 0 && key <= 9) {
                if (counter < PIN_LENGTH) {
                    entered_pin[counter] = (unsigned char)key;
                    digit_text[0] = (char)('0' + key);
                    digit_text[1] = '\0';
                    lcd_fontColor(TEXT_COLOR, ENTRY_COLOR);
                    lcd_putString((unsigned short)(108 + counter * 6), 89, digit_text);
                    counter++;
                }
            } else if (key == KEY_CLEAR) {
                clearEntry();
            } else if (key == KEY_OK) {
                submit_requested = 1;
            }
        }
    }

    if (counter >= PIN_LENGTH || submit_requested) {
        for (i = 0; i < counter; i++)
            attempt[i] = (char)('0' + entered_pin[i]);
        attempt[counter] = '\0';

        matched = findUserByPassword(attempt);

        lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
        if (matched != NULL) {
            currentUser = matched;
            lcd_putString(50, 105, (unsigned char *)"ACCESS GRANTED");
            result = PINPAD_ACCESS_GRANTED;
        } else {
            lcd_putString(55, 105, (unsigned char *)"ACCESS DENIED ");
            result = PINPAD_ACCESS_DENIED;
        }

        // next call starts a fresh entry
        active = 0;
        return result;
    }

    return PINPAD_IN_PROGRESS;
}
