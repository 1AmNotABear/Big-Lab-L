#include "delay.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"
#include "pinpad_screen.h"

void drawCoffeeScreen(void);
void time_ui_draw(void);
void touch_init(void);

int main(void) {

    // setup the external memory used by the LCD
    sdramInit();

    // setup the LCD using the supplied configuration
    lcdInit(&lcd_config);

    // turn the LCD screen on
    lcdTurnOn();

    // initialise the touch controller
    touch_init();

    // login gate: poll once per lap until it resolves. This is where other
    // non-blocking work (doorbell, sensors, ...) would later be polled too,
    // since pinpad_screen_step() no longer blocks.
    while (pinpad_screen_step() == PINPAD_IN_PROGRESS) {
    }
    mdelay(1000); // leave the ACCESS GRANTED/DENIED message up briefly

    while (1) {
        drawCoffeeScreen();
        mdelay(1000);

        time_ui_draw();
        mdelay(1000);
    }

    return 0;
}

