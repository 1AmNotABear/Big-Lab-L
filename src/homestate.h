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

/* --- Room light bit positions + Smart Plug (within the single roomLights byte) ------
 * Bit  |  Light
 * -----|------------
 *  0   |  Bedroom 1
 *  1   |  Bedroom 2
 *  2   |  Bedroom 3
 *  3   |  Bathroom
 *  4   |  Kitchen
 *  5   |  Living room
 */
#define LIGHT_BEDROOM1  0x01
#define LIGHT_BEDROOM2  0x02
#define LIGHT_BEDROOM3  0x04
#define LIGHT_BATHROOM  0x08
#define LIGHT_KITCHEN   0x10
#define LIGHT_LIVING    0x20
#define COFFEE_STATUS		0x40

/* --- Combined home state ----------------------------------------------- */
typedef struct {
    BlindPosition  blind1;
    BlindPosition  blind2;
    unsigned char  roomLights;
} HomeState;

/* Declared here, actually defined in homestate.c */
extern HomeState homeState;

#endif
