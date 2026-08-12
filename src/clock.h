#ifndef CLOCK_H
#define CLOCK_H

// Simulated wall clock. Runs 60x real time (1 real second = 1 sim minute,
// so 1 real minute = 1 sim hour).
extern int simHour;
extern int simMinute;

// Starts Timer0 as a free-running millisecond counter. Call once at boot.
void clock_init(void);

// Call once per main loop lap. Advances simHour/simMinute once enough real
// time has passed.
void clock_poll(void);

#endif
