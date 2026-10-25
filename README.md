# IR_iButton

Pocket tool on an STM32L051K6 with a 0.96" ST7735 display, four buttons,
an iButton reader/emulator contact pair and an IR LED + TSOP4838 receiver.

## Layout

```
firmware/
  Core/            CubeMX-generated init code (main, gpio, spi, tim, it, ...)
  App/             menu.c (menu tree), apps.c (screens), ui.c, power.c
  Lib/display/     ST7735 driver + 8x12 font
  Lib/buttons/     debounced buttons, press queue, auto-repeat (TIM2, 10 ms)
  Lib/timebase/    1 MHz microsecond counter (TIM22), delay_us()
  Lib/ibutton/     1-Wire master/slave bit-banging, DS1990A read + emulate
  Lib/gauge/       MAX17043 battery gauge (I2C1)
  Lib/ir/          38 kHz transmit (TIM21), NEC/raw receive via EXTI, NEC decode
hardware/          schematic, PCB, Gerbers, BOM
docs/              datasheets
```

Every `.c` file under `App/` and `Lib/*/` is picked up by the Makefile.

## Build and flash

```
# from the repo root or from firmware/
make            # needs arm-none-eabi-gcc
make flash      # stm32flash over the CP2102 (hold OK while resetting for the bootloader)
```

## Menu

- iButton: Read (DS1990A, CRC checked), Emulate (saved key), Saved ID
- IR: Receive (NEC decoded, anything else kept as raw), Replay, Send NEC
- Battery: cell voltage, charge percentage and a bar (MAX17043)
- Settings: LED test, Sleep (Stop mode, OK wakes it)

BACK on the top level also goes to sleep.

Sleep uses Stop mode, not Standby: waking from Standby is a reset, and the OK
button shares its net with BOOT0, so the chip would enter the system bootloader.

Reset is UP + BACK pressed together (74LVC1G32 on NRST). To flash, hold OK and
reset that way.

## Pins

| Function | Pin |
|---|---|
| Buttons OK (active high) / BACK / UP / DOWN | PA0 / PA1 / PA9 / PA10 |
| iButton reader / emulation line | PA12 / PA11 |
| IR LED / IR receiver | PB1 / PA15 |
| Peripheral power switch (low = on) | PB4 |

## Limitations

- The hardware has no TIM3 on this MCU, so the 38 kHz IR carrier is produced
  by a TIM21 interrupt toggling PB1 (50 % duty).
- Saved keys and IR signals live in RAM only and are lost on reset/sleep.
- iButton: DS1990A-style keys only (no Cyfral/Metakom), no writing of RW1990.
