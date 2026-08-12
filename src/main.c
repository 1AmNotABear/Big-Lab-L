#include "delay.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"
#include "screens/pinpad_screen.h"
#include "screens/admin_screen.h"
#include "screens/homescreen.h"
#include "screens/control_screen.h"
#include "screens/lights_screen.h"
#include "screens/temp_screen.h"
#include "users.h"

void touch_init(void);

int main(void) {
    PinpadResult loginResult;
    AdminResult adminResult;
    HomeResult homeResult;
    ControlResult controlResult;
    LightsResult lightsResult;
    TempResult tempResult;

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
                do {
                    adminResult = admin_screen_step();
                } while (adminResult == ADMIN_IN_PROGRESS);

                if (adminResult == ADMIN_SELECTED_CONTROL) {
                    do {
                        controlResult = control_screen_step();
                    } while (controlResult == CONTROL_IN_PROGRESS);
                    // HOME was pressed, loop back and show the admin screen again
                } else {
                    // dedicated USER PROFILE screen doesn't exist yet
                    lcd_fontColor(WHITE, NAVY);
                    lcd_putString(60, 105, (unsigned char *)"PROFILE SELECTED");
                    mdelay(1000);
                }
            } while (adminResult == ADMIN_SELECTED_CONTROL);

            continue;
        }

        // regular users land on the homescreen. Polled the same
        // non-blocking way as the login gate above.
        do {
            do {
                homeResult = home_screen_step(currentUser->userId);
            } while (homeResult == HOME_IN_PROGRESS);

            if (homeResult == HOME_SELECTED_LIGHT) {
                do {
                    lightsResult = lights_screen_step();
                } while (lightsResult == LIGHTS_IN_PROGRESS);
                // HOME was pressed, loop back and show the home screen again
            } else if (homeResult == HOME_SELECTED_TEMP) {
                do {
                    tempResult = temp_screen_step();
                } while (tempResult == TEMP_IN_PROGRESS);
                // HOME was pressed, loop back and show the home screen again
            } else if (homeResult != HOME_SELECTED_BACK) {
                // dedicated BLIND / COFFEE screens don't exist yet - just
                // acknowledge the choice for now and go back to the pinpad.
                lcd_fontColor(WHITE, NAVY);
                switch (homeResult) {
                    case HOME_SELECTED_BLIND:
                        lcd_putString(75, 105, (unsigned char *)"BLIND SELECTED");
                        break;
                    case HOME_SELECTED_COFFEE:
                        lcd_putString(68, 105, (unsigned char *)"COFFEE SELECTED");
                        break;
                    default:
                        break;
                }
                mdelay(1000);
            }
        } while (homeResult == HOME_SELECTED_LIGHT || homeResult == HOME_SELECTED_TEMP);
    }

    return 0;
}

