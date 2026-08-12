#include "delay.h"
#include "lpc24xx.h"   
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"
#include <stdlib.h>

//Constant
#define PAGE_BACKGROUND     BLUE_BG
#define TITLE_COLOUR        NAVY
#define LABEL_COLOUR        BLACK

#define BUTTON_COLOUR       PURPLE
#define BUTTON_BORDER       BLACK
#define BUTTON_TEXT         WHITE

#define VALUE_BACKGROUND    WHITE
#define VALUE_TEXT          BLACK

#define SAVED_COLOUR        DARK_GREEN

#define HOUR_MINUS_X1       20
#define HOUR_MINUS_Y1       75
#define HOUR_MINUS_X2       70
#define HOUR_MINUS_Y2       120

#define HOUR_PLUS_X1        170
#define HOUR_PLUS_Y1        75
#define HOUR_PLUS_X2        220
#define HOUR_PLUS_Y2        120

#define HOUR_VALUE_X1       90
#define HOUR_VALUE_Y1       75
#define HOUR_VALUE_X2       150
#define HOUR_VALUE_Y2       120

#define MINUTE_MINUS_X1     20
#define MINUTE_MINUS_Y1     160
#define MINUTE_MINUS_X2     70
#define MINUTE_MINUS_Y2     205

#define MINUTE_PLUS_X1      170
#define MINUTE_PLUS_Y1      160
#define MINUTE_PLUS_X2      220
#define MINUTE_PLUS_Y2      205

#define MINUTE_VALUE_X1     90
#define MINUTE_VALUE_Y1     160
#define MINUTE_VALUE_X2     150
#define MINUTE_VALUE_Y2     205

#define SET_BUTTON_X1       60
#define SET_BUTTON_Y1       235
#define SET_BUTTON_X2       180
#define SET_BUTTON_Y2       280


//Current time selected by the user
static int selected_hour = 12;
static int selected_minute = 0;

//Whether the SET button has been pressed
static int time_saved = 0;


void time_ui_init(void);
void time_ui_draw(void);
int time_ui_get_hour(void);
int time_ui_get_minute(void);


//Draw a filled rectangular button
static void draw_button(
    unsigned short x1,
    unsigned short y1,
    unsigned short x2,
    unsigned short y2,
    unsigned char *text,
    unsigned short text_x,
    unsigned short text_y
) {
    lcd_fillRect(x1, y1, x2, y2, BUTTON_COLOUR);
    lcd_drawRect(x1, y1, x2, y2, BUTTON_BORDER);
    lcd_fontColor(BUTTON_TEXT, BUTTON_COLOUR);
    lcd_putString(text_x, text_y, text);
}

//Convert a number into a two-character string
static void make_two_digit_string(int value, unsigned char text[3]) {
    text[0] = (unsigned char)('0' + value / 10);
    text[1] = (unsigned char)('0' + value % 10);
    text[2] = '\0';
}

//Draw one time value inside a white rectangle
static void draw_time_value(
    unsigned short x1,
    unsigned short y1,
    unsigned short x2,
    unsigned short y2,
    unsigned short text_x,
    unsigned short text_y,
    int value
) {
    unsigned char text[3];
    make_two_digit_string(value, text);

    lcd_fillRect(x1, y1, x2, y2, VALUE_BACKGROUND);
    lcd_drawRect(x1, y1, x2, y2, BLACK);
    lcd_fontColor(VALUE_TEXT, VALUE_BACKGROUND);
    lcd_putString(text_x, text_y, text);
}


//Initialise the LCD
void time_ui_init(void) {
    //Initialise external SDRAM used by the LCD frame buffer
    sdramInit();

    //Initialise the LCD controller
    lcdInit(&lcd_config);

    //Turn the LCD on
    lcdTurnOn();

    //Draw the first version of the interface
    time_ui_draw();
}

//Draw the complete time-setting interface
void time_ui_draw(void) {
    lcd_fillScreen(PAGE_BACKGROUND);
    lcd_fontColor(TITLE_COLOUR, PAGE_BACKGROUND);
    lcd_putString(78, 20, (unsigned char *)"SET TIME");

    //Hour label
    lcd_fontColor(LABEL_COLOUR, PAGE_BACKGROUND);
    lcd_putString(96, 50, (unsigned char *)"HOUR");
    //Hour minus button
    draw_button(
        HOUR_MINUS_X1,
        HOUR_MINUS_Y1,
        HOUR_MINUS_X2,
        HOUR_MINUS_Y2,
        (unsigned char *)"-",
        42,
        91
    );
    //Current hour
    draw_time_value(
        HOUR_VALUE_X1,
        HOUR_VALUE_Y1,
        HOUR_VALUE_X2,
        HOUR_VALUE_Y2,
        110,
        91,
        selected_hour
    );
    //Hour plus button
    draw_button(
        HOUR_PLUS_X1,
        HOUR_PLUS_Y1,
        HOUR_PLUS_X2,
        HOUR_PLUS_Y2,
        (unsigned char *)"+",
        191,
        91
    );

    //Minute label
    lcd_fontColor(LABEL_COLOUR, PAGE_BACKGROUND);
    lcd_putString(88, 135, (unsigned char *)"MINUTE");
    //Minute minus button
    draw_button(
        MINUTE_MINUS_X1,
        MINUTE_MINUS_Y1,
        MINUTE_MINUS_X2,
        MINUTE_MINUS_Y2,
        (unsigned char *)"-",
        42,
        176
    );
    //Current minute
    draw_time_value(
        MINUTE_VALUE_X1,
        MINUTE_VALUE_Y1,
        MINUTE_VALUE_X2,
        MINUTE_VALUE_Y2,
        110,
        176,
        selected_minute
    );
    //Minute plus button
    draw_button(
        MINUTE_PLUS_X1,
        MINUTE_PLUS_Y1,
        MINUTE_PLUS_X2,
        MINUTE_PLUS_Y2,
        (unsigned char *)"+",
        191,
        176
    );

    //SET button
    draw_button(
        SET_BUTTON_X1,
        SET_BUTTON_Y1,
        SET_BUTTON_X2,
        SET_BUTTON_Y2,
        (unsigned char *)"SET",
        106,
        251
    );

    //Displaying SAVED after the SET button is pressed
    if (time_saved) {
        lcd_fontColor(SAVED_COLOUR, PAGE_BACKGROUND);
        lcd_putString(96, 295, (unsigned char *)"SAVED");
    }
}

//Return the currently selected hour
int time_ui_get_hour(void) {
    return selected_hour;
}

//Return the currently selected minute
int time_ui_get_minute(void) {
    return selected_minute;
}
