#include "delay.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"

void drawCoffeeScreen(void);
void time_ui_draw(void);

int main(void) {

    // setup the external memory used by the LCD
    sdramInit();

    // setup the LCD using the supplied configuration
    lcdInit(&lcd_config);

    // turn the LCD screen on
    lcdTurnOn();

    while (1) {
        drawCoffeeScreen();
        mdelay(1000);

        time_ui_draw();
        mdelay(1000);
    }

    return 0;
}

