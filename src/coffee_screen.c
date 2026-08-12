#include "delay.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"

// colours used for the login screen
#define BACKGROUND_COLOR  NAVY
#define TEXT_COLOR        WHITE
#define BUTTON_COLOR      DARK_GRAY
#define BORDER_COLOR      CYAN

// function declarations
void drawCoffeeScreen(void);

void drawButton(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, const char *label);

unsigned int getTextLength(const char *text);

void drawCoffeeScreen(void)
{
    // clear the screen and make the background navy
    lcd_fillScreen(BACKGROUND_COLOR);

    // display the COFFEE heading
    lcd_fontColor(TEXT_COLOR, BACKGROUND_COLOR);
    lcd_putString(120, 35, (unsigned char *)"COFFEE");


    // turn on button
    drawButton(16, 80, 222, 130, "TURN ON");

    // scehdule button
    drawButton(16, 140, 222, 190, "SCHEDULE");

    // turn off button
    drawButton(16, 200, 222, 250, "TURN OFF");

    // House icon top right
    lcd_drawRect(200, 25 , 220, 45, WHITE);
    lcd_line(200,25,210,15,WHITE);
    lcd_line(210,15,220,25,WHITE);

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

    // fill the inside of the button
    lcd_fillRect(x0, y0, x1, y1, BUTTON_COLOR);

    // draw the border around the button
    lcd_drawRect(x0, y0, x1, y1, BORDER_COLOR);

    // find how many characters are in the button label
    labelLength = getTextLength(label);

    // calculate the width and height of the button
    buttonWidth = x1 - x0 + 1;
    buttonHeight = y1 - y0 + 1;

    // calculate where the label should be placed
    textX = x0 + ((buttonWidth - (labelLength * 6U)) / 2U);
    textY = y0 + ((buttonHeight - 8U) / 2U);

    // display the label in the middle of the button
    lcd_fontColor(TEXT_COLOR, BUTTON_COLOR);
    lcd_putString(textX, textY, (unsigned char *)label);
}

unsigned int getTextLength(const char *text)
{
    unsigned int length;

    length = 0;

    // count each character until the end of the text
    while (text[length] != '\0')
    {
        length++;
    }

    return length;
}