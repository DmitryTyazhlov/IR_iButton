#ifndef POWER_H_
#define POWER_H_

/* Switches off the display and peripherals and enters Stop mode.
 * Pressing OK wakes the device; execution then continues in this function
 * with clocks, pins and display restored. (Standby is not used: wake-up from
 * it is a reset and OK shares its net with BOOT0.) */
void power_sleep(void);

#endif
