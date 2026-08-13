#include "profiles_screen.h"
#include "pinpad/touch.h"
#include "../users.h"
#include "../lcd/lcd_hw.h"
#include "../lcd/lcd_grph.h"
#include "../lcd/lcd_cfg.h"
#include "../lcd/sdram.h"

#define BACKGROUND_COLOR  NAVY
#define TITLE_COLOR       GREEN
#define TEXT_COLOR        WHITE
#define BUTTON_COLOR      DARK_GRAY
#define DELETE_COLOR      RED
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

// maps a visual row back to its index in users[] (admin/inactive users are
// skipped, so row order isn't 1:1 with the users[] array)
static int rowUserIndex[NUM_USERS];
static int rowCount;

// draws one user row: a select region showing the userId, and a DEL region
static void drawProfileRow(int row, int userIndex)
{
    unsigned short y0 = (unsigned short)(70 + row * 46);
    unsigned short y1 = (unsigned short)(y0 + 40);

    lcd_fillRect(16, y0, 176, y1, BUTTON_COLOR);
    lcd_drawRect(16, y0, 176, y1, BORDER_COLOR);
    lcd_fontColor(TEXT_COLOR, BUTTON_COLOR);
    lcd_putString(24, (unsigned short)(y0 + 16), (unsigned char *)users[userIndex].userId);

    lcd_fillRect(180, y0, 224, y1, DELETE_COLOR);
    lcd_drawRect(180, y0, 224, y1, BORDER_COLOR);
    lcd_fontColor(TEXT_COLOR, DELETE_COLOR);
    lcd_putString(188, (unsigned short)(y0 + 16), (unsigned char *)"DEL");
}

static void drawProfilesScreen(void)
{
    static const char title[] = "USER PROFILES";
    unsigned int length;
    unsigned short textX;
    int i;

    lcd_fillScreen(BACKGROUND_COLOR);

    length = getTextLength(title);
    textX = (unsigned short)((DISPLAY_WIDTH - (length * 6U)) / 2U);
    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(textX, 35, (unsigned char *)title);
    lcd_line(74, 49, 166, 49, TITLE_COLOR);

    drawButton(BACK_X0, BACK_Y0, BACK_X1, BACK_Y1, "HOME");

    rowCount = 0;
    for (i = 0; i < NUM_USERS; i++) {
        if (users[i].isAdmin || !users[i].active)
            continue;

        rowUserIndex[rowCount] = i;
        drawProfileRow(rowCount, i);
        rowCount++;
    }
}

// -1 = nothing, 0 = HOME, else (row*2 + col + 1): col 0 = select, col 1 = delete
static int coordinates_to_option(int x, int y)
{
    int col;
    int row;

    if (x >= BACK_X0 && x <= BACK_X1 && y >= BACK_Y0 && y <= BACK_Y1)
        return 0;

    if (x >= 16 && x <= 176)
        col = 0;
    else if (x >= 180 && x <= 224)
        col = 1;
    else
        return -1;

    if (y < 70)
        return -1;

    row = (y - 70) / 46;
    if (row >= rowCount || y > 70 + row * 46 + 40)
        return -1;

    return row * 2 + col + 1;
}

ProfilesResult profiles_screen_step(void)
{
    int screen_x;
    int screen_y;
    int option;
    int row;
    int isDelete;
    int userIndex;
    ProfilesResult result;

    // first call after a reset, draw the screen from scratch
    if (!active) {
        touch_debounce_init(&touchState);
        drawProfilesScreen();
        active = 1;
    }

    result = PROFILES_IN_PROGRESS;

    // got a touch this poll, work out which button it landed on
    if (touch_poll_press(&touchState, &screen_x, &screen_y)) {
        option = coordinates_to_option(screen_x, screen_y);

        if (option == 0) {
            result = PROFILES_SELECTED_HOME;
        } else if (option >= 1) {
            // turn the option back into a row + whether it's the DEL side
            row = (option - 1) / 2;
            isDelete = (option - 1) % 2;
            userIndex = rowUserIndex[row];

            if (isDelete) {
                // deactivate the user and redraw the list without them
                users[userIndex].active = 0;
                drawProfilesScreen();
            } else {
                // pick this user and hand back to the caller
                currentUser = &users[userIndex];
                result = PROFILES_SELECTED_USER;
            }
        }
    }

    // next call starts a fresh screen
    if (result != PROFILES_IN_PROGRESS) {
        active = 0;
    }

    return result;
}
