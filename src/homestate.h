#ifndef HOMESTATE_H
#define HOMESTATE_H

/* --- Blind position -------------------------------------------------
 * Matches the tricolour LED mapping in the project brief:
 *   red = rolled up, green = mid-way, blue = rolled down
 */
typedef enum {
    BLIND_ROLLED_UP   = 0,   // red
    BLIND_MID_WAY     = 1,   // green
    BLIND_ROLLED_DOWN = 2    // blue
} BlindPosition;

/* --- Room light bit positions + Smart Plug (within the roomLights field) ------
 * Bit   |  Light
 * ------|------------
 *  0    |  Bedroom 1
 *  1    |  Bedroom 2
 *  2    |  Bedroom 3
 *  3    |  Bathroom
 *  4    |  Kitchen
 *  5    |  Living room
 *  6    |  (Smart Plug / coffee, see COFFEE_STATUS)
 *  7    |  Toilet
 *  8    |  Deck
 */
#define LIGHT_BEDROOM1  0x01
#define LIGHT_BEDROOM2  0x02
#define LIGHT_BEDROOM3  0x04
#define LIGHT_BATHROOM  0x08
#define LIGHT_KITCHEN   0x10
#define LIGHT_LIVING    0x20
#define COFFEE_STATUS		0x40
#define LIGHT_TOILET    0x80
#define LIGHT_DECK      0x100

/* --- Combined home state ----------------------------------------------- */
typedef struct {
    BlindPosition  blind1;
    BlindPosition  blind2;
    int            blind1Override;  // 0 = automatic, 1 = manual override active
    int            blind2Override;  // 0 = automatic, 1 = manual override active
    unsigned short roomLights;
    int            tempSetPoint;  // thermostat set point, chosen by the user
    int            tempOff;       // 0 = on, 1 = user pressed OFF
} HomeState;

/* Declared here, actually defined in homestate.c */
extern HomeState homeState;

#endif
