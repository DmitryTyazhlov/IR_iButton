#ifndef MAX17043_H_
#define MAX17043_H_

#include <stdbool.h>
#include <stdint.h>

/* MAX17043 battery fuel gauge on I2C1 (PB6 SCL / PB7 SDA, 7-bit addr 0x36). */
bool max17043_init(void);

/* Cell voltage in millivolts. */
bool max17043_read_mv(uint16_t *mv);

/* State of charge, 0..100 %. */
bool max17043_read_percent(uint8_t *percent);

uint8_t max17043_address(void);  /* address that answered */
uint16_t max17043_version(void); /* raw VERSION register */
uint16_t max17043_vcell_raw(void); /* last raw VCELL register */

/* Diagnostics for a gauge that does not answer. */
uint32_t max17043_last_error(void); /* HAL_I2C_ERROR_* bits, 0x100 = init failed */
uint8_t max17043_bus_levels(void);  /* bit0 = SCL high, bit1 = SDA high */
uint8_t max17043_scan(uint8_t *found, uint8_t max); /* 7-bit addresses */

#endif
