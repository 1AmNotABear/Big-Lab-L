#include "lights_screen.h"
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
    unsigned short bit;
} LightButton;

static const LightButton lightButtons[8] = {
    { "Living",  LIGHT_LIVING },
    { "Kitchen", LIGHT_KITCHEN },
    { "Bed 1",   LIGHT_BEDROOM1 },
    { "Bath",    LIGHT_BATHROOM },
    { "Bed 2",   LIGHT_BEDROOM2 },
    { "Toilet",  LIGHT_TOILET },
    { "Bed 3",   LIGHT_BEDROOM3 },
    { "Deck",    LIGHT_DECK }
};

#define ALL_LIGHTS (LIGHT_LIVING | LIGHT_KITCHEN | LIGHT_BEDROOM1 | LIGHT_BEDROOM2 | \
                    LIGHT_BEDROOM3 | LIGHT_BATHROOM | LIGHT_TOILET | LIGHT_DECK)

// draws one light button, yellow when its bit is on, gray when off
static void drawLightAt(int index)
{
    int row = index / 2;
    int col = index % 2;
    unsigned short x0 = (col == 0) ? 16 : 122;
    unsigned short x1 = (col == 0) ? 118 : 224;
    unsigned short y0 = (unsigned short)(56 + row * 46);
    unsigned short y1 = (unsigned short)(y0 + 40);
    int isOn = (homeState.roomLights & lightButtons[index].bit) != 0;
    unsigned short bg = isOn ? YELLOW : BUTTON_COLOR;
    unsigned short fg = isOn ? BLACK : TEXT_COLOR;
    unsigned int labelLength = getTextLength(lightButtons[index].label);
    unsigned short textX = (unsigned short)(x0 + (((x1 - x0 + 1) - labelLength * 6U) / 2U));
    unsigned short textY = (unsigned short)(y0 + (((y1 - y0 + 1) - 8U) / 2U));

    lcd_fillRect(x0, y0, x1, y1, bg);
    lcd_drawRect(x0, y0, x1, y1, BORDER_COLOR);
    lcd_fontColor(fg, bg);
    lcd_putString(textX, textY, (unsigned char *)lightButtons[index].label);
}

static void drawLightsScreen(void)
{
    int i;

    lcd_fillScreen(BACKGROUND_COLOR);

    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(102, 35, (unsigned char *)"LIGHTS");
    lcd_line(74, 49, 166, 49, TITLE_COLOR);

    drawButton(8, 8, 60, 30, "HOME");

    for (i = 0; i < 8; i++)
        drawLightAt(i);

    drawButton(16, 240, 118, 280, "ALL ON");
    drawButton(122, 240, 224, 280, "ALL OFF");
}

// -1 = nothing, 0 = HOME, 1-8 = light buttons, 9 = ALL ON, 10 = ALL OFF
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

    if (y < 56 || y > 280)
        return -1;

    row = (y - 56) / 46;
    if (row > 4 || y > 56 + row * 46 + 40)
        return -1;

    if (row == 4)
        return (col == 0) ? 9 : 10;

    return (row * 2 + col) + 1;
}

LightsResult lights_screen_step(void)
{
    int screen_x;
    int screen_y;
    int option;
    int i;
    LightsResult result;

    // first call after a reset, draw the screen from scratch
    if (!active) {
        touch_debounce_init(&touchState);
        drawLightsScreen();
        active = 1;
    }

    result = LIGHTS_IN_PROGRESS;

    // got a touch this poll, work out which button it landed on
    if (touch_poll_press(&touchState, &screen_x, &screen_y)) {
        option = coordinates_to_option(screen_x, screen_y);

        if (option == 0) {
            result = LIGHTS_SELECTED_HOME;
        } else if (option >= 1 && option <= 8) {
            // toggle just that one light and redraw its button
            homeState.roomLights ^= lightButtons[option - 1].bit;
            drawLightAt(option - 1);
        } else if (option == 9) {
            // ALL ON - flip every light on and redraw the whole grid
            homeState.roomLights |= ALL_LIGHTS;
            for (i = 0; i < 8; i++)
                drawLightAt(i);
        } else if (option == 10) {
            // ALL OFF - flip every light off and redraw the whole grid
            homeState.roomLights &= (unsigned short)~ALL_LIGHTS;
            for (i = 0; i < 8; i++)
                drawLightAt(i);
        }
    }

    // next call starts a fresh screen
    if (result != LIGHTS_IN_PROGRESS) {
        active = 0;
    }

    return result;
}
