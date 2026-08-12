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

    drawButton(8, 8, 60, 30, "HOME");

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

    if (x >= 8 && x <= 60 && y >= 8 && y <= 30)
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

    if (!active) {
        touch_debounce_init(&touchState);
        drawProfilesScreen();
        active = 1;
    }

    result = PROFILES_IN_PROGRESS;

    if (touch_poll_press(&touchState, &screen_x, &screen_y)) {
        option = coordinates_to_option(screen_x, screen_y);

        if (option == 0) {
            result = PROFILES_SELECTED_HOME;
        } else if (option >= 1) {
            row = (option - 1) / 2;
            isDelete = (option - 1) % 2;
            userIndex = rowUserIndex[row];

            if (isDelete) {
                users[userIndex].active = 0;
                drawProfilesScreen();
            } else {
                currentUser = &users[userIndex];
                result = PROFILES_SELECTED_USER;
            }
        }
    }

    if (result != PROFILES_IN_PROGRESS) {
        active = 0;
    }

    return result;
}
