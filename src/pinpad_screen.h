#ifndef PINPAD_SCREEN_H
#define PINPAD_SCREEN_H

typedef enum {
    PINPAD_IN_PROGRESS,
    PINPAD_ACCESS_GRANTED,
    PINPAD_ACCESS_DENIED
} PinpadResult;

PinpadResult pinpad_screen_step(void);

#endif
