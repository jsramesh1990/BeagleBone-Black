# BeagleBone Black P8 Header Pinout

## 1. Overview

The **P8 header** is one of the two 46-pin expansion headers available on the BeagleBone Black.

```text
BeagleBone Black
+--------------------------------+
|                                |
|          P8 Header             |
|          46 Pins               |
|                                |
+--------------------------------+
```

P8 provides access to:

* GPIO
* UART
* SPI
* PWM
* Timers
* PRU-related signals
* Other AM335x peripheral functions

> **Important:** Many P8 pins are multiplexed. A physical pin can provide
> different functions depending on the Device Tree pinmux configuration.

---

# 2. P8 Header Layout

The P8 header contains **46 pins**.

```text
              P8 HEADER

        1   3   5   7   9  11  13  15  17  19  21  23
        2   4   6   8  10  12  14  16  18  20  22  24
       26  28  30  32  34  36  38  40  42  44  46
       25  27  29  31  33  35  37  39  41  43  45
```

For practical wiring, always use the board's official pinout for the
exact board revision.

---

# 3. P8 Power and Ground Pins

|   Pin | Function           |
| ----: | ------------------ |
|  P8.1 | GND                |
|  P8.2 | GND                |
|  P8.3 | GPIO / multiplexed |
|  P8.4 | GPIO / multiplexed |
|  P8.5 | GPIO / multiplexed |
|  P8.6 | GPIO / multiplexed |
| P8.43 | GND                |
| P8.44 | GND                |
| P8.45 | GND                |
| P8.46 | GND                |

Use a GND pin when connecting external test equipment.

---

# 4. P8 GPIO Mapping

The P8 header provides many GPIO-capable pins.

| Header Pin | AM335x GPIO | Common Signal |
| ---------- | ----------- | ------------- |
| P8.3       | GPIO1_6     | GPIO          |
| P8.4       | GPIO1_7     | GPIO          |
| P8.5       | GPIO1_2     | GPIO          |
| P8.6       | GPIO1_3     | GPIO          |
| P8.7       | GPIO2_2     | GPIO          |
| P8.8       | GPIO2_3     | GPIO          |
| P8.9       | GPIO2_5     | GPIO          |
| P8.10      | GPIO2_4     | GPIO          |
| P8.11      | GPIO1_13    | GPIO          |
| P8.12      | GPIO1_12    | GPIO          |
| P8.13      | GPIO0_23    | GPIO          |
| P8.14      | GPIO0_26    | GPIO          |
| P8.15      | GPIO1_15    | GPIO          |
| P8.16      | GPIO1_14    | GPIO          |
| P8.17      | GPIO0_27    | GPIO          |
| P8.18      | GPIO2_1     | GPIO          |
| P8.19      | GPIO0_22    | GPIO          |
| P8.20      | GPIO1_31    | GPIO          |
| P8.21      | GPIO0_22    | GPIO          |
| P8.22      | GPIO0_26    | GPIO          |
| P8.23      | GPIO1_31    | GPIO          |
| P8.24      | GPIO0_10    | GPIO          |
| P8.25      | GPIO1_0     | GPIO          |
| P8.26      | GPIO1_29    | GPIO          |
| P8.27      | GPIO2_22    | GPIO          |
| P8.28      | GPIO2_24    | GPIO          |
| P8.29      | GPIO2_23    | GPIO          |
| P8.30      | GPIO2_25    | GPIO          |
| P8.31      | GPIO0_10    | GPIO          |
| P8.32      | GPIO0_11    | GPIO          |
| P8.33      | GPIO0_9     | GPIO          |
| P8.34      | GPIO2_17    | GPIO          |
| P8.35      | GPIO0_8     | GPIO          |
| P8.36      | GPIO2_16    | GPIO          |
| P8.37      | GPIO2_14    | GPIO          |
| P8.38      | GPIO2_15    | GPIO          |
| P8.39      | GPIO2_12    | GPIO          |
| P8.40      | GPIO2_13    | GPIO          |
| P8.41      | GPIO2_10    | GPIO          |
| P8.42      | GPIO2_11    | GPIO          |

> **Note:** GPIO naming and available mux functions depend on the AM335x
> pinmux configuration. For your driver project, keep the Device Tree
> pinctrl configuration as the authoritative configuration.

---

# 5. UART Pins on P8

UART functionality can be mapped to P8 pins through pin multiplexing.

A commonly used UART interface is:

| Signal  | Header Pin | Function |
| ------- | ---------- | -------- |
| UART TX | P8.37      | UART     |
| UART RX | P8.38      | UART     |

Typical connection:

```text
BeagleBone Black       USB-UART Adapter
----------------       ----------------
UART TX  ------------> RX
UART RX  <------------ TX
GND      ------------- GND
```

---

# 6. SPI Pins on P8

SPI signals can also be available on P8 depending on the selected
pinmux configuration.

Typical SPI signals include:

```text
SCLK
MOSI
MISO
CS0
CS1
```

Connection concept:

```text
BBB                 SPI Device
---                 ----------

SCLK  ------------> SCLK
MOSI  ------------> MOSI
MISO  <------------ MISO
CS    ------------> CS
GND   ------------- GND
```

Verify the exact pin assignment against the active Device Tree before
connecting the device.

---

# 7. PWM Pins on P8

Several P8 pins support PWM through AM335x pin multiplexing.

Typical PWM-capable functions include:

```text
EHRPWM
PWM
TIMER
```

PWM test flow:

```text
Linux PWM subsystem
        |
        v
PWM driver
        |
        v
Device Tree pinmux
        |
        v
P8 PWM pin
        |
        v
Oscilloscope
```

Measure:

* Frequency
* Period
* Duty cycle
* Polarity

---

# 8. GPIO LED Test

A simple GPIO output test can use an LED.

```text
P8 GPIO
   |
  330R
   |
  LED
   |
  GND
```

Test:

```text
GPIO = 1
   |
   v
LED ON

GPIO = 0
   |
   v
LED OFF
```

Use an appropriate GPIO line based on the active Device Tree.

---

# 9. GPIO Input Test

A push button can be used for GPIO input testing.

```text
             +3.3V
               |
             Button
               |
               +------ GPIO
               |
            Pull-down
               |
              GND
```

Test:

```text
Button Released → GPIO LOW

Button Pressed   → GPIO HIGH
```

---

# 10. P8 Pinmux Concept

The AM335x uses pin multiplexing.

Conceptually:

```text
                 P8 Physical Pin
                        |
        +---------------+---------------+
        |               |               |
       GPIO            UART            SPI
        |               |               |
        +---------------+---------------+
                        |
                  Selected by
                   Device Tree
```

Example Device Tree concept:

```dts
pinctrl_uart1: uart1_pins {
    pinctrl-single,pins = <
        /* UART pin configuration */
    >;
};
```

The selected mux mode determines which peripheral controls the pin.

---

# 11. Device Driver Relationship

The P8 header is the physical interface. The Linux driver controls the
hardware through the appropriate subsystem.

```text
P8 Header
    |
    v
AM335x Pinmux
    |
    v
Peripheral Controller
    |
    v
Linux Kernel Driver
    |
    v
Linux Subsystem
    |
    v
User Space
```

Examples:

```text
P8 GPIO
   ↓
GPIO Controller
   ↓
GPIO Driver
   ↓
GPIO Character Device
   ↓
Application
```

```text
P8 UART
   ↓
UART Controller
   ↓
Serial Driver
   ↓
/dev/tty*
   ↓
Application
```

---

# 12. P8 Usage in This Project

The BeagleBone Black device-driver project uses P8 for practical
peripheral testing.

```text
hardware/
└── pinout/
    ├── bbb_header_p8.md
    ├── bbb_header_p9.md
    ├── gpio_pin_map.md
    ├── uart_pin_map.md
    ├── i2c_pin_map.md
    ├── spi_pin_map.md
    ├── pwm_pin_map.md
    ├── adc_pin_map.md
    └── can_pin_map.md
```

The P8 document provides the physical-header reference, while the
individual peripheral files provide the detailed driver-specific
mapping.

---

# 13. Testing Checklist

```text
[ ] Identify P8 pin
[ ] Verify pinmux
[ ] Verify Device Tree
[ ] Check GPIO/peripheral ownership
[ ] Connect external hardware
[ ] Verify voltage levels
[ ] Load driver
[ ] Verify probe
[ ] Perform functional test
[ ] Verify signal using multimeter/logic analyzer
[ ] Perform stress test
```

---

# 14. Important Electrical Rules

```text
1. Verify the pin function before wiring.
2. Do not apply unsupported voltage levels.
3. Use common GND between BBB and external equipment.
4. Check pinmux conflicts before enabling peripherals.
5. Never assume a GPIO-capable pin is currently configured as GPIO.
6. Device Tree determines the active peripheral configuration.
7. Use current-limiting resistors when connecting LEDs.
8. Use proper level shifting when interfacing incompatible voltage levels.
```

---

# 15. Quick Reference

```text
                 BEAGLEBONE BLACK P8

       +-----------------------------------+
       |                                   |
       |  P8 Header                        |
       |                                   |
       |  GPIO                             |
       |  UART                             |
       |  SPI                              |
       |  PWM                              |
       |  Timers                           |
       |                                   |
       +-----------------------------------+

                 |
                 v

             Device Tree
                 |
                 v
              Pinmux
                 |
                 v
           Linux Driver
                 |
                 v
          Hardware Testing
```

---

## Project Path

```text
beaglebone-black/hardware/pinout/bbb_header_p8.md
```

