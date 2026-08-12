#include "touch.h"
#include <stdio.h>
#include "lpc24xx.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"
#include "delay.h"
#include <stdlib.h>
#include <string.h>

#define BACKGROUND_COLOR  NAVY
#define TITLE_COLOR       GREEN
#define TEXT_COLOR        WHITE
#define BUTTON_COLOR      DARK_GRAY
#define BORDER_COLOR      CYAN
#define ENTRY_COLOR       BLACK

static int selected = -1;

// figures out if admin control (1)/ user profile (2) selected
int select(unsigned short x, unsigned short y)
{
    if ((16 <= x) && (x <= 222)) {
        if ((120 <= y) && (y <= 200)){
            return 1;
        }
        if ((208 <= y) && (y <= 288)){
            return 0;
        }
    }
    return -1;
}

void drawButton(unsigned short x0,
                unsigned short y0,
                unsigned short x1,
                unsigned short y1,
                const char *label)
{
    unsigned int labelLength;
    unsigned short buttonWidth;
    unsigned short buttonHeight;
    unsigned short textX;
    unsigned short textY;

    lcd_fillRect(x0, y0, x1, y1, BUTTON_COLOR);
    lcd_drawRect(x0, y0, x1, y1, BORDER_COLOR);

    labelLength = strlen(label);
    buttonWidth  = x1 - x0;
    buttonHeight = y1 - y0;

    textX = x0 + (buttonWidth - labelLength * 8) / 2;
    textY = y0 + (buttonHeight - 16) / 2;

    lcd_fontColor(TEXT_COLOR, BUTTON_COLOR);
    lcd_putString(textX, textY, (unsigned char *)label);
}


// draws the admin screen
void drawAdminScreen(void)
{   
    // clear the screen and make the background navy
    lcd_fillScreen(BACKGROUND_COLOR);

    // display the USER ID heading
    lcd_fontColor(TITLE_COLOR, BACKGROUND_COLOR);
    lcd_putString(99, 35, (unsigned char *)"ADMIN");

    // draw the green line underneath the heading
    lcd_line(74, 49, 166, 49, TITLE_COLOR);

    // Admin and User buttons
    drawButton(16, 120, 222, 200, "CONTROL");
    drawButton(16, 208, 222, 288, "USER PROFILE");
}

int main(void)
{
    unsigned short x = 0;
    unsigned short y = 0;
    unsigned char z1;
    unsigned char z2;
    int pressure;
    int touching;
    int previous_touching = 0;
    int key;
    int counter;
    char status_text[50];
    unsigned char digit_text[2];
    
    // setup the external memory used by the LCD
    sdramInit();
    
    // Setup the LCD using the supplied config
    lcdInit(&lcd_config);

    // turn the LCD screen ON
    lcdTurnOn();

    // initialise the touch controller
    touch_init();
    
    // draw the full user ID screen
    drawAdminScreen();

        while (1) {
            
        touch_read_xy((char *)&x, (char *)&y);
        z1 = touch_read(0xB0);
        z2 = touch_read(0xC0);

        if (z1 == 0) {
            pressure = 0;
        } else {
            pressure = (int)(400.0f * ((float)x / 256.0f) *
                (((float)z2 / (float)z1) - 1.0f));
        }

        // pressure debugging
        sprintf(status_text, "x:%u y:%u p:%d", x, y, pressure);
        if (pressure > 20) {
            touching = 1;
        } else {
            touching = 0;
        }
        
        // checking selection
        if (touching && !previous_touching) {
            selected = select(x, y);
            if (selected != -1) {
                break;
            }
        }
        previous_touching = touching;
        }
    return selected;
}
