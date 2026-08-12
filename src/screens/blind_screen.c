#include "blind_screen.h"
#include "pinpad/touch.h"
#include "../homestate.h"
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
unsigned int getTextLength(const char *text);

static int active = 0;

static TouchDebounceState touchState;

typedef struct {
    const char *label;
    BlindPosition position;
} BlindButton;

static const BlindButton blindButtons[6] = {
    { "OPEN",  BLIND_ROLLED_UP },
    { "MID",   BLIND_MID_WAY },
    { "CLOSE", BLIND_ROLLED_DOWN },
    { "OPEN",  BLIND_ROLLED_UP },
    { "MID",   BLIND_MID_WAY },
    { "CLOSE", BLIND_ROLLED_DOWN }
};

// draws one blind button, yellow when it matches the blind's current position
static void drawBlindAt(int index)
{
    int row = index / 2;
    int col = index % 2;
    unsigned short x0 = (col == 0) ? 16 : 122;
    unsigned short x1 = (col == 0) ? 118 : 224;
    unsigned short y0 = (unsigned short)(70 + row * 46);
    unsigned short y1 = (unsigned short)(y0 + 40);
    BlindPosition current = (index < 3) ? homeState.blind1 : homeState.blind2;
    int isSelected = (current == blindButtons[index].position);
    unsigned short bg = isSelected ? YELLOW : BUTTON_COLOR;
    unsigned short fg = isSelected ? BLACK : TEXT_COLOR;
    unsigned int labelLength = getTextLength(blindButtons[index].label);
    unsigned short textX = (unsigned short)(x0 + (((x1 - x0 + 1) - labelLength * 6U) / 2U));
    unsigned short textY = (unsigned short)(y0 + (((y1 - y0 + 1) - 8U) / 2U));

    lcd_fillRect(x0, y0, x1, y1, bg);
    lcd_drawRect(x0, y0, x1, y1, BORDER_COLOR);
    lcd_fontColor(fg, bg);
    lcd_putString(textX, textY, (unsigned char *)blindButtons[index].label);
}

static void drawBlindScreen(void)
{
    int i;

    lcd_fillScreen(BACKGROUND_COLOR);

    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(102, 35, (unsigned char *)"BLINDS");
    lcd_line(74, 49, 166, 49, TITLE_COLOR);

    drawButton(8, 8, 60, 30, "HOME");

    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(30, 56, (unsigned char *)"Blind 1");
    lcd_putString(136, 56, (unsigned char *)"Blind 2");

    for (i = 0; i < 6; i++)
        drawBlindAt(i);
}

// -1 = nothing, 0 = HOME, 1-3 = blind 1 buttons, 4-6 = blind 2 buttons
static int coordinates_to_option(int x, int y)
{
    int col;
    int row;

    if (x >= 8 && x <= 60 && y >= 8 && y <= 30)
        return 0;

    if (x >= 16 && x <= 118)
        col = 0;
    else if (x >= 122 && x <= 224)
        col = 1;
    else
        return -1;

    if (y < 70 || y > 202)
        return -1;

    row = (y - 70) / 46;
    if (row > 2 || y > 70 + row * 46 + 40)
        return -1;

    return (row * 2 + col) + 1;
}

BlindResult blind_screen_step(void)
{
    int screen_x;
    int screen_y;
    int option;
    int i;
    BlindResult result;

    if (!active) {
        touch_debounce_init(&touchState);
        drawBlindScreen();
        active = 1;
    }

    result = BLIND_IN_PROGRESS;

    if (touch_poll_press(&touchState, &screen_x, &screen_y)) {
        option = coordinates_to_option(screen_x, screen_y);

        if (option == 0) {
            result = BLIND_SELECTED_HOME;
        } else if (option >= 1 && option <= 3) {
            homeState.blind1 = blindButtons[option - 1].position;
            for (i = 0; i < 3; i++)
                drawBlindAt(i);
        } else if (option >= 4 && option <= 6) {
            homeState.blind2 = blindButtons[option - 1].position;
            for (i = 3; i < 6; i++)
                drawBlindAt(i);
        }
    }

    if (result != BLIND_IN_PROGRESS) {
        active = 0;
    }

    return result;
}
