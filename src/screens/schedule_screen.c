#include "schedule_screen.h"
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
unsigned int getTextLength(const char *text);

static int active = 0;
static int saved_flag = 0;

static int selected_hour = 0;
static int selected_minute = 0;

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

static void drawSavedText(void)
{
    lcd_fontColor(saved_flag ? GREEN : BACKGROUND_COLOR, BACKGROUND_COLOR);
    lcd_putString(96, 260, (unsigned char *)"SAVED");
}

static void drawScheduleScreen(void)
{
    lcd_fillScreen(BACKGROUND_COLOR);

    drawButton(8, 8, 60, 30, "HOME");

    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(78, 35, (unsigned char *)"SET SCHEDULE");
    lcd_line(74, 49, 166, 49, TITLE_COLOR);

    drawButton(16, 70, 118, 100, "+");
    drawValue(16, 110, 118, 140, selected_hour);
    drawButton(16, 150, 118, 180, "-");
    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString(56, 190, (unsigned char *)"HR");

    drawButton(122, 70, 224, 100, "+");
    drawValue(122, 110, 224, 140, selected_minute);
    drawButton(122, 150, 224, 180, "-");
    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString(158, 190, (unsigned char *)"MIN");

    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString(108, 220, (unsigned char *)"ON");

    drawButton(140, 280, 224, 310, "SET");

    drawSavedText();
}

// -1 = nothing, 0 = HOME, 1 = HR+, 2 = HR-, 3 = MIN+, 4 = MIN-, 5 = SET
static int coordinates_to_option(int x, int y)
{
    if (x >= 8 && x <= 60 && y >= 8 && y <= 30)
        return 0;

    if (y >= 70 && y <= 100) {
        if (x >= 16 && x <= 118) return 1;
        if (x >= 122 && x <= 224) return 3;
    }

    if (y >= 150 && y <= 180) {
        if (x >= 16 && x <= 118) return 2;
        if (x >= 122 && x <= 224) return 4;
    }

    if (x >= 140 && x <= 224 && y >= 280 && y <= 310)
        return 5;

    return -1;
}

ScheduleResult schedule_screen_step(void)
{
    int screen_x;
    int screen_y;
    int option;
    ScheduleResult result;

    if (!active) {
        touch_debounce_init(&touchState);
        selected_hour = currentUser->coffeeSchedule.time.hour;
        selected_minute = currentUser->coffeeSchedule.time.minute;
        saved_flag = 0;
        drawScheduleScreen();
        active = 1;
    }

    result = SCHEDULE_IN_PROGRESS;

    if (touch_poll_press(&touchState, &screen_x, &screen_y)) {
        option = coordinates_to_option(screen_x, screen_y);

        if (option == 0) {
            result = SCHEDULE_SELECTED_HOME;
        } else if (option == 1) {
            selected_hour = (selected_hour + 1 > 23) ? 0 : selected_hour + 1;
            drawValue(16, 110, 118, 140, selected_hour);
        } else if (option == 2) {
            selected_hour = (selected_hour - 1 < 0) ? 23 : selected_hour - 1;
            drawValue(16, 110, 118, 140, selected_hour);
        } else if (option == 3) {
            selected_minute = (selected_minute + 1 > 59) ? 0 : selected_minute + 1;
            drawValue(122, 110, 224, 140, selected_minute);
        } else if (option == 4) {
            selected_minute = (selected_minute - 1 < 0) ? 59 : selected_minute - 1;
            drawValue(122, 110, 224, 140, selected_minute);
        } else if (option == 5) {
            currentUser->coffeeSchedule.time.hour = selected_hour;
            currentUser->coffeeSchedule.time.minute = selected_minute;
            currentUser->coffeeSchedule.enabled = 1;
            saved_flag = 1;
            drawSavedText();
        }
    }

    if (result != SCHEDULE_IN_PROGRESS) {
        active = 0;
    }

    return result;
}
