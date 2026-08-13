# BeagleBone Black GPIO Pin Map

## 1. Overview

The **BeagleBone Black** is based on the **TI AM3358/AM3359 Sitara**
processor and provides multiple GPIO controllers.

GPIOs can be used for:

* Digital input
* Digital output
* LED control
* Push buttons
* Interrupts
* Sensor signals
* Chip-select signals
* Hardware control lines

Linux exposes GPIO through the **GPIO subsystem**.

```text
User Application
       |
       v
GPIO Character Device
       |
       v
Linux GPIO Subsystem
       |
       v
AM335x GPIO Controller
       |
       v
GPIO Pin
       |
       v
External Hardware
```

---

# 2. GPIO Controllers

The AM335x contains multiple GPIO banks:

```text
GPIO0
GPIO1
GPIO2
GPIO3
```

Each bank contains up to 32 GPIO lines.

Conceptually:

```text
GPIO0 ── GPIO0_0  ... GPIO0_31
GPIO1 ── GPIO1_0  ... GPIO1_31
GPIO2 ── GPIO2_0  ... GPIO2_31
GPIO3 ── GPIO3_0  ... GPIO3_31
```

Not every GPIO line is necessarily available on the BeagleBone Black
headers because some pins are used internally or by other peripherals.

---

# 3. GPIO Number Calculation

For the legacy Linux GPIO numbering scheme, the global GPIO number is
commonly calculated as:

```text
GPIO Number = GPIO Bank × 32 + GPIO Offset
```

Example:

```text
GPIO0_30

= 0 × 32 + 30
= GPIO 30
```

Another example:

```text
GPIO1_28

= 1 × 32 + 28
= GPIO 60
```

> **Important:** Modern Linux applications should generally use the
> GPIO character-device interface and line offsets rather than relying
> on deprecated global GPIO numbers.

---

# 4. P8 GPIO Pin Map

Common GPIO-capable P8 header pins:

| Header Pin | AM335x GPIO | Legacy GPIO Number |
| ---------: | ----------- | -----------------: |
|       P8.3 | GPIO1_6     |                 38 |
|       P8.4 | GPIO1_7     |                 39 |
|       P8.5 | GPIO1_2     |                 34 |
|       P8.6 | GPIO1_3     |                 35 |
|       P8.7 | GPIO2_2     |                 66 |
|       P8.8 | GPIO2_3     |                 67 |
|       P8.9 | GPIO2_5     |                 69 |
|      P8.10 | GPIO2_4     |                 68 |
|      P8.11 | GPIO1_13    |                 45 |
|      P8.12 | GPIO1_12    |                 44 |
|      P8.13 | GPIO0_23    |                 23 |
|      P8.14 | GPIO0_26    |                 26 |
|      P8.15 | GPIO1_15    |                 47 |
|      P8.16 | GPIO1_14    |                 46 |
|      P8.17 | GPIO0_27    |                 27 |
|      P8.18 | GPIO2_1     |                 65 |
|      P8.19 | GPIO0_22    |                 22 |
|      P8.20 | GPIO1_31    |                 63 |
|      P8.21 | GPIO0_14    |                 14 |
|      P8.22 | GPIO0_27    |                 27 |
|      P8.23 | GPIO1_31    |                 63 |
|      P8.24 | GPIO1_10    |                 42 |
|      P8.25 | GPIO1_0     |                 32 |
|      P8.26 | GPIO1_29    |                 61 |
|      P8.27 | GPIO2_22    |                 86 |
|      P8.28 | GPIO2_24    |                 88 |
|      P8.29 | GPIO2_23    |                 87 |
|      P8.30 | GPIO2_25    |                 89 |
|      P8.31 | GPIO0_10    |                 10 |
|      P8.32 | GPIO0_11    |                 11 |
|      P8.33 | GPIO0_9     |                  9 |
|      P8.34 | GPIO2_17    |                 81 |
|      P8.35 | GPIO0_8     |                  8 |
|      P8.36 | GPIO2_16    |                 80 |
|      P8.37 | GPIO2_14    |                 78 |
|      P8.38 | GPIO2_15    |                 79 |
|      P8.39 | GPIO2_12    |                 76 |
|      P8.40 | GPIO2_13    |                 77 |
|      P8.41 | GPIO2_10    |                 74 |
|      P8.42 | GPIO2_11    |                 75 |

> **Note:** Several P8 pins have alternate peripheral functions. Verify
> the active pinmux and board revision before using a pin as GPIO.

---

# 5. P9 GPIO Pin Map

Common GPIO-capable P9 header pins:

| Header Pin | AM335x GPIO | Legacy GPIO Number |
| ---------: | ----------- | -----------------: |
|      P9.11 | GPIO0_30    |                 30 |
|      P9.12 | GPIO1_28    |                 60 |
|      P9.13 | GPIO0_31    |                 31 |
|      P9.14 | GPIO1_18    |                 50 |
|      P9.15 | GPIO1_16    |                 48 |
|      P9.16 | GPIO1_19    |                 51 |
|      P9.17 | GPIO0_5     |                  5 |
|      P9.18 | GPIO0_4     |                  4 |
|      P9.19 | GPIO0_13    |                 13 |
|      P9.20 | GPIO0_12    |                 12 |
|      P9.21 | GPIO0_3     |                  3 |
|      P9.22 | GPIO0_2     |                  2 |
|      P9.23 | GPIO1_17    |                 49 |
|      P9.24 | GPIO0_15    |                 15 |
|      P9.25 | GPIO3_21    |                117 |
|      P9.26 | GPIO0_14    |                 14 |
|      P9.27 | GPIO3_19    |                115 |
|      P9.28 | GPIO3_17    |                113 |
|      P9.29 | GPIO3_16    |                112 |
|      P9.30 | GPIO3_16    |                112 |
|      P9.31 | GPIO3_14    |                110 |
|      P9.41 | GPIO0_20    |                 20 |
|      P9.42 | GPIO0_7     |                  7 |

> **Important:** P9 pins are heavily multiplexed. For example,
> P9.19/P9.20 can be used for I2C, while other configurations may use
> those pins for different functions. Do not configure the same pins for
> multiple peripherals simultaneously.

---

# 6. GPIO Input

GPIO input is used to read a digital signal.

Example push-button circuit:

```text
                  +3.3V
                    |
                  Button
                    |
                    +-------- GPIO INPUT
                    |
                 Pull-down
                    |
                   GND
```

Operation:

```text
Button Released → GPIO = 0

Button Pressed  → GPIO = 1
```

---

# 7. GPIO Output

GPIO output can control LEDs, relays, enable signals, and other digital
hardware.

Example:

```text
GPIO OUTPUT
     |
    330Ω
     |
    LED
     |
    GND
```

Operation:

```text
GPIO = HIGH → LED ON

GPIO = LOW  → LED OFF
```

---

# 8. GPIO Interrupt

GPIO can also be configured to generate interrupts.

Example:

```text
Push Button
     |
     v
GPIO Input
     |
     v
GPIO Interrupt
     |
     v
Linux IRQ Subsystem
     |
     v
GPIO Driver
     |
     v
Application / Kernel Work
```

Typical interrupt triggers:

```text
Rising Edge
Falling Edge
Both Edges
```

---

# 9. GPIO Device Tree

GPIO configuration is normally described using Device Tree.

Example concept:

```dts
my_gpio_device {
    compatible = "mycompany,my-gpio-device";

    control-gpios = <&gpio1 28 GPIO_ACTIVE_HIGH>;
};
```

The GPIO controller is referenced through:

```text
&gpio0
&gpio1
&gpio2
&gpio3
```

Example:

```dts
control-gpios = <&gpio1 28 GPIO_ACTIVE_HIGH>;
```

This means:

```text
GPIO Controller : GPIO1
GPIO Offset     : 28
Polarity        : Active High
```

---

# 10. GPIO Device Tree Flow

```text
bbb-gpio.dts
      |
      v
bbb-gpio.dtsi
      |
      v
Device Tree Compiler
      |
      v
       DTB
      |
      v
Linux Kernel
      |
      v
GPIO Controller
      |
      v
GPIO Driver
      |
      v
GPIO Subsystem
```

Project files:

```text
beaglebone-black/
└── device-tree/
    └── gpio/
        ├── bbb-gpio.dts
        ├── bbb-gpio.dtsi
        └── README.md
```

---

# 11. GPIO Linux Character Device

Modern Linux uses the GPIO character-device interface.

Check GPIO chips:

```bash
gpiodetect
```

Example:

```text
gpiochip0 [gpio-0-31] (32 lines)
gpiochip1 [gpio-32-63] (32 lines)
gpiochip2 [gpio-64-95] (32 lines)
gpiochip3 [gpio-96-127] (32 lines)
```

Check lines:

```bash
gpioinfo
```

Example:

```text
gpiochip1 - 32 lines:
        line 28: "GPIO1_28" unused input active-high
```

---

# 12. GPIO Output Test

Using `gpioset`:

```bash
sudo gpioset gpiochip1 28=1
```

Set LOW:

```bash
sudo gpioset gpiochip1 28=0
```

> The exact `gpiochip` and line number depend on the kernel's GPIO
> controller registration and should be verified using `gpiodetect` and
> `gpioinfo`.

---

# 13. GPIO Input Test

Using `gpioget`:

```bash
sudo gpioget gpiochip1 28
```

Example:

```text
1
```

or:

```text
0
```

---

# 14. GPIO Event Test

For edge detection:

```bash
gpiomon gpiochip1 28
```

Pressing a button connected to the GPIO may generate:

```text
event: FALLING EDGE
event: RISING EDGE
```

This is useful for testing GPIO interrupt functionality.

---

# 15. GPIO Driver Flow

For a custom kernel driver:

```text
                 User Application
                        |
                        v
                 ioctl/read/write
                        |
                        v
                  GPIO Driver
                        |
                        v
                  GPIO Framework
                        |
                        v
                GPIO Controller
                        |
                        v
                  AM335x Hardware
                        |
                        v
                     GPIO Pin
```

For a Device Tree based driver:

```text
Device Tree
     |
     v
GPIO Descriptor
     |
     v
gpiod_get()
     |
     v
GPIO Controller
     |
     v
GPIO Hardware
```

---

# 16. GPIO Active-Low

GPIO polarity can be defined in Device Tree.

Active-high:

```dts
enable-gpios = <&gpio1 28 GPIO_ACTIVE_HIGH>;
```

Active-low:

```dts
enable-gpios = <&gpio1 28 GPIO_ACTIVE_LOW>;
```

Example:

```text
GPIO_ACTIVE_HIGH

GPIO = 1 → Device ON
GPIO = 0 → Device OFF
```

```text
GPIO_ACTIVE_LOW

GPIO = 0 → Device ON
GPIO = 1 → Device OFF
```

---

# 17. GPIO Pull-Up / Pull-Down

The AM335x pinmux can configure appropriate pull-up/pull-down behavior.

Conceptually:

```text
GPIO Input
    |
    +---- Pull-up
    |
    +---- Pull-down
```

For a button:

```text
             3.3V
              |
            Button
              |
              +-------- GPIO
              |
           Pull-down
              |
             GND
```

The pull configuration must be selected through the pinmux/Device Tree
configuration appropriate to the board.

---

# 18. GPIO Pinmux

A physical header pin can have multiple functions.

Example concept:

```text
                  P9.19
                    |
        +-----------+-----------+
        |           |           |
       GPIO       I2C         CAN
        |           |           |
        +-----------+-----------+
                    |
                 PINMUX
                    |
               Device Tree
```

Only one selected mux function should control the pin at a time.

---

# 19. GPIO Testing with LED

Recommended simple hardware test:

```text
             BeagleBone Black

P9 GPIO
   |
   |
  330Ω
   |
   +---->|----+
        LED   |
              |
             GND
```

Test sequence:

```text
1. Configure GPIO as output
2. Set GPIO HIGH
3. Verify LED ON
4. Set GPIO LOW
5. Verify LED OFF
6. Repeat continuously
```

---

# 20. GPIO Testing with Button

```text
              +3.3V
                |
              Button
                |
                +---------- P9 GPIO
                |
             Pull-down
                |
               GND
```

Test sequence:

```text
1. Configure GPIO as input
2. Read GPIO
3. Press button
4. Read GPIO again
5. Verify state transition
6. Enable interrupt
7. Verify edge event
```

---

# 21. GPIO Debugging

Check GPIO controller:

```bash
gpiodetect
```

Check GPIO lines:

```bash
gpioinfo
```

Check kernel messages:

```bash
dmesg | grep -i gpio
```

Check pin configuration when available:

```bash
sudo grep -i gpio /sys/kernel/debug/pinctrl/*/pinmux-pins
```

Check pin configuration:

```bash
sudo cat /sys/kernel/debug/pinctrl/*/pinmux-pins
```

> Access to `/sys/kernel/debug/pinctrl` requires the kernel debugfs and
> pinctrl debug information to be enabled.

---

# 22. Common GPIO Problems

### GPIO is not detected

Check:

```bash
gpiodetect
```

Then:

```bash
gpioinfo
```

---

### GPIO is already in use

`gpioinfo` may show:

```text
"some-device" [used]
```

Possible causes:

```text
1. Device Tree peripheral enabled
2. GPIO already claimed by another driver
3. Pin configured for alternate peripheral
4. GPIO used by onboard hardware
```

---

### GPIO output does not change

Check:

```text
1. Pinmux
2. Device Tree
3. GPIO number/line
4. Direction
5. GPIO ownership
6. Hardware wiring
7. Ground connection
```

---

# 23. GPIO Driver Debug Flow

```text
             GPIO NOT WORKING
                    |
                    v
             Check gpiodetect
                    |
                    v
              Check gpioinfo
                    |
                    v
             Check Device Tree
                    |
                    v
               Check Pinmux
                    |
                    v
             Check Driver Probe
                    |
                    v
                Check dmesg
                    |
                    v
              Check Hardware
                    |
                    v
             Check Signal with
             Multimeter/Scope
```

---

# 24. GPIO Test Checklist

```text
[ ] GPIO controller detected
[ ] GPIO line detected
[ ] Correct header pin selected
[ ] Correct GPIO controller selected
[ ] Correct GPIO offset selected
[ ] Pinmux configured
[ ] Device Tree configured
[ ] GPIO driver loaded
[ ] GPIO input tested
[ ] GPIO output tested
[ ] LED test completed
[ ] Button test completed
[ ] Pull-up/pull-down tested
[ ] Interrupt tested
[ ] Rising edge tested
[ ] Falling edge tested
[ ] GPIO ownership verified
[ ] Kernel logs checked
```

---

# 25. GPIO Pin Quick Reference

```text
+------------+----------------+----------------+
| Header     | AM335x GPIO    | Legacy GPIO    |
+------------+----------------+----------------+
| P8.3       | GPIO1_6        | 38             |
| P8.4       | GPIO1_7        | 39             |
| P8.11      | GPIO1_13       | 45             |
| P8.12      | GPIO1_12       | 44             |
| P8.13      | GPIO0_23       | 23             |
| P8.14      | GPIO0_26       | 26             |
| P8.15      | GPIO1_15       | 47             |
| P8.16      | GPIO1_14       | 46             |
| P8.17      | GPIO0_27       | 27             |
| P8.18      | GPIO2_1        | 65             |
| P8.19      | GPIO0_22       | 22             |
| P8.21      | GPIO0_14       | 14             |
| P8.25      | GPIO1_0        | 32             |
| P8.26      | GPIO1_29       | 61             |
+------------+----------------+----------------+

+------------+----------------+----------------+
| Header     | AM335x GPIO    | Legacy GPIO    |
+------------+----------------+----------------+
| P9.11      | GPIO0_30       | 30             |
| P9.12      | GPIO1_28       | 60             |
| P9.13      | GPIO0_31       | 31             |
| P9.14      | GPIO1_18       | 50             |
| P9.15      | GPIO1_16       | 48             |
| P9.16      | GPIO1_19       | 51             |
| P9.17      | GPIO0_5        | 5              |
| P9.18      | GPIO0_4        | 4              |
| P9.21      | GPIO0_3        | 3              |
| P9.22      | GPIO0_2        | 2              |
| P9.23      | GPIO1_17       | 49             |
| P9.24      | GPIO0_15       | 15             |
| P9.26      | GPIO0_14       | 14             |
| P9.41      | GPIO0_20       | 20             |
| P9.42      | GPIO0_7        | 7              |
+------------+----------------+----------------+
```

---

# 26. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   └── pinout/
│       └── gpio_pin_map.md
│
├── device-tree/
│   └── gpio/
│       ├── bbb-gpio.dts
│       ├── bbb-gpio.dtsi
│       └── README.md
│
├── drivers/
│   └── gpio/
│       ├── README.md
│       └── ...
│
└── tests/
    └── gpio/
        ├── gpio_input_test.sh
        ├── gpio_output_test.sh
        └── gpio_interrupt_test.sh
```

---

# 27. Complete GPIO Architecture

```text
                    BeagleBone Black
                           |
                           v
                     P8 / P9 Pins
                           |
                           v
                      Pinmux
                           |
                           v
                     Device Tree
                           |
                           v
                  AM335x GPIO Bank
                  +------+------+------+
                  |      |      |      |
                GPIO0  GPIO1  GPIO2  GPIO3
                  |      |      |      |
                  +------+------+------+
                           |
                           v
                    Linux GPIO Core
                           |
                           v
                 GPIO Character Device
                           |
                           v
                    User Application
                           |
              +------------+------------+
              |                         |
             LED                      Button
              |                         |
           OUTPUT                     INPUT
```

---

## Project File

```text
beaglebone-black/hardware/pinout/gpio_pin_map.md
```

