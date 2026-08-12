#include "delay.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"
#include "pinpad_screen.h"
#include "admin_screen.h"
#include "users.h"

void drawCoffeeScreen(void);
void time_ui_draw(void);
void touch_init(void);

int main(void) {
    PinpadResult loginResult;
    AdminResult adminResult;

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
    // since pinpad_screen_step() no longer blocks. A denied attempt shows
    // the message briefly then automatically resets for another try; only
    // a correct password breaks out to the rest of the app.
    do {
        loginResult = pinpad_screen_step();
        if (loginResult != PINPAD_IN_PROGRESS) {
            mdelay(1000); // leave the ACCESS GRANTED/DENIED message up briefly
        }
    } while (loginResult != PINPAD_ACCESS_GRANTED);

    // admin accounts are offered the admin screen (control panel vs. user
    // profile) right after login; regular users skip straight past it.
    // Polled the same non-blocking way as the login gate above.
    if (currentUser != NULL && currentUser->isAdmin) {
        do {
            adminResult = admin_screen_step();
        } while (adminResult == ADMIN_IN_PROGRESS);

        // dedicated CONTROL / USER PROFILE screens don't exist yet - just
        // acknowledge the choice for now and fall through like everyone
        // else. Real routing (using adminResult) comes once those screens
        // are built.
        lcd_fontColor(WHITE, NAVY);
        if (adminResult == ADMIN_SELECTED_CONTROL) {
            lcd_putString(70, 105, (unsigned char *)"CONTROL SELECTED");
        } else {
            lcd_putString(60, 105, (unsigned char *)"PROFILE SELECTED");
        }
        mdelay(1000);
    }

    // falls through to the coffee screen for now - real per-user routing
    // (using currentUser, see users.h) comes later
    while (1) {
        drawCoffeeScreen();
        mdelay(1000);

        time_ui_draw();
        mdelay(1000);
    }

    return 0;
}

