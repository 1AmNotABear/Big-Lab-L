#include "admin_screen.h"
#include "pinpad/touch.h"
#include "../lcd/lcd_hw.h"
#include "../lcd/lcd_grph.h"
#include "../lcd/lcd_cfg.h"
#include "../lcd/sdram.h"

#define BACKGROUND_COLOR  NAVY
#define TITLE_COLOR       GREEN
#define TEXT_COLOR        WHITE
#define BUTTON_COLOR      DARK_GRAY
#define BORDER_COLOR      CYAN

void drawButton(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, const char *label);

static int active = 0;

static TouchDebounceState touchState;

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
    int screen_x;
    int screen_y;
    int option;
    AdminResult result;

    if (!active) {
        touch_debounce_init(&touchState);
        drawAdminScreen();
        active = 1;
    }

    result = ADMIN_IN_PROGRESS;

    if (touch_poll_press(&touchState, &screen_x, &screen_y)) {
        option = coordinates_to_option(screen_x, screen_y);
        if (option == 1) {
            result = ADMIN_SELECTED_CONTROL;
        } else if (option == 0) {
            result = ADMIN_SELECTED_PROFILE;
        }
    }

    if (result != ADMIN_IN_PROGRESS) {
        // next call starts a fresh screen
        active = 0;
    }

    return result;
}
