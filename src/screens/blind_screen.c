#include "blind_screen.h"
#include "pinpad/touch.h"
#include "../homestate.h"
#include "../blinds.h"
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
    int row = index % 3;
    int col = index / 3;
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

// draws the manual-override toggle button for one blind (1 or 2), yellow
// when override is active
static void drawOverrideAt(int blindNum)
{
    unsigned short x0 = (blindNum == 1) ? 16 : 122;
    unsigned short x1 = (blindNum == 1) ? 118 : 224;
    unsigned short y0 = 208;
    unsigned short y1 = 248;
    int isOn = (blindNum == 1) ? homeState.blind1Override : homeState.blind2Override;
    unsigned short bg = isOn ? YELLOW : BUTTON_COLOR;
    unsigned short fg = isOn ? BLACK : TEXT_COLOR;
    unsigned int labelLength = getTextLength("OVERRIDE");
    unsigned short textX = (unsigned short)(x0 + (((x1 - x0 + 1) - labelLength * 6U) / 2U));
    unsigned short textY = (unsigned short)(y0 + (((y1 - y0 + 1) - 8U) / 2U));

    lcd_fillRect(x0, y0, x1, y1, bg);
    lcd_drawRect(x0, y0, x1, y1, BORDER_COLOR);
    lcd_fontColor(fg, bg);
    lcd_putString(textX, textY, (unsigned char *)"OVERRIDE");
}

static void drawBlindScreen(void)
{
    int i;

    lcd_fillScreen(BACKGROUND_COLOR);

    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(102, 35, (unsigned char *)"BLINDS");
    lcd_line(74, 49, 166, 49, TITLE_COLOR);

    drawButton(BACK_X0, BACK_Y0, BACK_X1, BACK_Y1, "HOME");

    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(30, 56, (unsigned char *)"Blind 1");
    lcd_putString(136, 56, (unsigned char *)"Blind 2");

    for (i = 0; i < 6; i++)
        drawBlindAt(i);

    drawOverrideAt(1);
    drawOverrideAt(2);
}

// -1 = nothing, 0 = HOME, 1-3 = blind 1 buttons, 4-6 = blind 2 buttons,
// 7 = blind 1 override, 8 = blind 2 override
static int coordinates_to_option(int x, int y)
{
    int col;
    int row;

    if (x >= BACK_X0 && x <= BACK_X1 && y >= BACK_Y0 && y <= BACK_Y1)
        return 0;

    if (x >= 16 && x <= 118)
        col = 0;
    else if (x >= 122 && x <= 224)
        col = 1;
    else
        return -1;

    if (y < 70 || y > 248)
        return -1;

    row = (y - 70) / 46;
    if (row > 3 || y > 70 + row * 46 + 40)
        return -1;

    if (row == 3)
        return 7 + col;

    return (col * 3 + row) + 1;
}

BlindResult blind_screen_step(void)
{
    int screen_x;
    int screen_y;
    int option;
    int i;
    BlindResult result;

    // first call after a reset, draw the screen from scratch
    if (!active) {
        touch_debounce_init(&touchState);
        drawBlindScreen();
        active = 1;
    }

    result = BLIND_IN_PROGRESS;

    // got a touch this poll, work out which button it landed on
    if (touch_poll_press(&touchState, &screen_x, &screen_y)) {
        option = coordinates_to_option(screen_x, screen_y);

        if (option == 0) {
            result = BLIND_SELECTED_HOME;
        } else if (option >= 1 && option <= 3) {
            // blind 1 button - move it and flag it as manually overridden
            homeState.blind1 = blindButtons[option - 1].position;
            homeState.blind1Override = 1;
            updateBlindState(&homeState);
            for (i = 0; i < 3; i++)
                drawBlindAt(i);
            drawOverrideAt(1);
        } else if (option >= 4 && option <= 6) {
            // blind 2 button - move it and flag it as manually overridden
            homeState.blind2 = blindButtons[option - 1].position;
            homeState.blind2Override = 1;
            updateBlindState(&homeState);
            for (i = 3; i < 6; i++)
                drawBlindAt(i);
            drawOverrideAt(2);
        } else if (option == 7) {
            // toggle blind 1's manual override
            homeState.blind1Override ^= 1;
            drawOverrideAt(1);
        } else if (option == 8) {
            // toggle blind 2's manual override
            homeState.blind2Override ^= 1;
            drawOverrideAt(2);
        }
    }

    // next call starts a fresh screen
    if (result != BLIND_IN_PROGRESS) {
        active = 0;
    }

    return result;
}
