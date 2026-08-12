#include "touch.h"
#include <stdio.h>
#include "lpc24xx.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"
#include "delay.h"
#include <stdlib.h>

#define PIN_LENGTH 4

#define BACKGROUND_COLOR  NAVY
#define TITLE_COLOR       GREEN
#define TEXT_COLOR        WHITE
#define BUTTON_COLOR      DARK_GRAY
#define BORDER_COLOR      CYAN
#define ENTRY_COLOR       BLACK

static const unsigned char correct_pin[PIN_LENGTH] = {1, 3, 5, 9};
static unsigned char entered_pin[PIN_LENGTH];

int coordinates_to_key(unsigned char x, unsigned char y)
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
        return -1;

    if (y >= 120 && y <= 156)
        row = 0;
    else if (y >= 164 && y <= 200)
        row = 1;
    else if (y >= 208 && y <= 244)
        row = 2;
    else if (y >= 245)
        row = 3;
    else
        return -1;

    if (row == 0)
        return column + 1;
    if (row == 1)
        return column + 4;
    if (row == 2)
        return column + 7;
    if (row == 3 && column == 1)
        return 0;

    return -1;
}

int pin_is_correct(void)
{
    int i;
    for (i = 0; i < PIN_LENGTH; i++) {
        if (entered_pin[i] != correct_pin[i])
            return 0;
    }
    return 1;
}

unsigned int getTextLength(const char *text)
{
    unsigned int length = 0;
    while (text[length] != '\0') {
        length++;
    }
    return length;
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

    labelLength = getTextLength(label);
    buttonWidth = x1 - x0 + 1;
    buttonHeight = y1 - y0 + 1;

    textX = x0 + ((buttonWidth - (labelLength * 6U)) / 2U);
    textY = y0 + ((buttonHeight - 8U) / 2U);

    lcd_fontColor(TEXT_COLOR, BUTTON_COLOR);
    lcd_putString(textX, textY, (unsigned char *)label);
}

void drawLoginScreen(void)
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

    // draw the black box where the entered ID will appear
    lcd_fillRect(55, 77, 185, 106, ENTRY_COLOR);
    lcd_drawRect(55, 77, 185, 106, BORDER_COLOR);

    // display four empty spaces for the four-digit ID
    lcd_fontColor(TEXT_COLOR, ENTRY_COLOR);
    lcd_putString(108, 89, (unsigned char *)"____");

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

int main(void)
{
    unsigned char x = 0;
    unsigned char y = 0;
    unsigned char z1;
    unsigned char z2;
    int pressure;
    int touching;
    int previous_touching = 0;
    int key;
    int counter;
    char status_text[50];
    unsigned char digit_text[2];
    int i;

    for (i = 0; i < PIN_LENGTH; i++)
        entered_pin[i] = 0;
    
    // setup the external memory used by the LCD
    sdramInit();
    
    // Setup the LCD using the supplied config
    lcdInit(&lcd_config);

    // turn the LCD screen ON
    lcdTurnOn();

    // initialise the touch controller
    touch_init();
    
    // draw the full user ID screen
    drawLoginScreen();

    counter = 0;
    while (counter < PIN_LENGTH) {
        touch_read_xy((char *)&x, (char *)&y);
        z1 = touch_read(0xB0);
        z2 = touch_read(0xC0);

        if (z1 == 0) {
            pressure = 0;
        } else {
            pressure = (int)(400.0f * ((float)x / 256.0f) *
                (((float)z2 / (float)z1) - 1.0f));
        }

        sprintf(status_text, "x:%u y:%u p:%d", x, y, pressure);
        lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
        lcd_putString(10, 10, (unsigned char *)status_text);

        touching = (pressure > 20) ? 1 : 0;

        if (touching && !previous_touching) {
            key = coordinates_to_key(x, y);
            if (key >= 0 && key <= 9) {
                entered_pin[counter] = (unsigned char)key;
                digit_text[0] = (char)('0' + key);
                digit_text[1] = '\0';
                lcd_fontColor(TEXT_COLOR, ENTRY_COLOR);
                lcd_putString((unsigned short)(108 + counter * 12), 89, digit_text);
                counter++;
            }
        }

        previous_touching = touching;
    }

    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    if (pin_is_correct()) {
        lcd_putString(50, 105, (unsigned char *)"ACCESS GRANTED");
    } else {
        lcd_putString(55, 105, (unsigned char *)"ACCESS DENIED ");
    }

    while (1) {
    }

    return 0;
}
