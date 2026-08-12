#include "delay.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"
#include "screens/pinpad_screen.h"
#include "screens/admin_screen.h"
#include "screens/homescreen.h"
#include "users.h"

void touch_init(void);

int main(void) {
    PinpadResult loginResult;
    AdminResult adminResult;
    HomeResult homeResult;

    // setup the external memory used by the LCD
    sdramInit();

    // setup the LCD using the supplied configuration
    lcdInit(&lcd_config);

    // turn the LCD screen on
    lcdTurnOn();

    // initialise the touch controller
    touch_init();

    while (1) {
        // login gate: poll once per lap until it resolves. This is where
        // other non-blocking work (doorbell, sensors, ...) would later be
        // polled too, since pinpad_screen_step() no longer blocks. A denied
        // attempt shows the message briefly then automatically resets for
        // another try; only a correct password breaks out to the rest of
        // the app.
        do {
            loginResult = pinpad_screen_step();
            if (loginResult != PINPAD_IN_PROGRESS) {
                mdelay(1000); // leave the ACCESS GRANTED/DENIED message up briefly
            }
        } while (loginResult != PINPAD_ACCESS_GRANTED);

        if (currentUser != NULL && currentUser->isAdmin) {
            // admin accounts are offered the admin screen (control panel vs.
            // user profile) right after login. Polled the same non-blocking
            // way as the login gate above.
            do {
                adminResult = admin_screen_step();
            } while (adminResult == ADMIN_IN_PROGRESS);

            // dedicated CONTROL / USER PROFILE screens don't exist yet - just
            // acknowledge the choice for now and go back to the pinpad.
            // Real routing (using adminResult) comes once those screens
            // are built.
            lcd_fontColor(WHITE, NAVY);
            if (adminResult == ADMIN_SELECTED_CONTROL) {
                lcd_putString(70, 105, (unsigned char *)"CONTROL SELECTED");
            } else {
                lcd_putString(60, 105, (unsigned char *)"PROFILE SELECTED");
            }
            mdelay(1000);
            continue;
        }

        // regular users land on the homescreen. Polled the same
        // non-blocking way as the login gate above.
        do {
            homeResult = home_screen_step(currentUser->userId);
        } while (homeResult == HOME_IN_PROGRESS);

        // dedicated LIGHT / BLIND / COFFEE / TEMP screens don't exist yet -
        // BACK just returns to the pinpad, and the others acknowledge the
        // choice for now and go back to the pinpad too. Real routing
        // (using homeResult) comes once those screens are built.
        if (homeResult != HOME_SELECTED_BACK) {
            lcd_fontColor(WHITE, NAVY);
            switch (homeResult) {
                case HOME_SELECTED_LIGHT:
                    lcd_putString(75, 105, (unsigned char *)"LIGHT SELECTED");
                    break;
                case HOME_SELECTED_BLIND:
                    lcd_putString(75, 105, (unsigned char *)"BLIND SELECTED");
                    break;
                case HOME_SELECTED_COFFEE:
                    lcd_putString(68, 105, (unsigned char *)"COFFEE SELECTED");
                    break;
                case HOME_SELECTED_TEMP:
                    lcd_putString(78, 105, (unsigned char *)"TEMP SELECTED");
                    break;
                default:
                    break;
            }
            mdelay(1000);
        }
    }

    return 0;
}

