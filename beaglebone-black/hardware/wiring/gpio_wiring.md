# `gpio_wiring.md`

````markdown
# BeagleBone Black GPIO Wiring

## 1. Overview

This document describes the hardware wiring required to test GPIO
input and output functionality on the BeagleBone Black.

GPIOs are commonly used for:

- LED control
- Push buttons
- Digital sensors
- Relays
- Interrupt signals
- Status indicators
- Chip-select signals
- Hardware enable/reset signals
- Embedded device control

The basic GPIO flow is:

```text
User Application
       |
       v
Linux GPIO Framework
       |
       v
GPIO Controller Driver
       |
       v
AM335x GPIO Controller
       |
       v
GPIO Pin
       |
       +------> LED / Sensor / Switch
````

---

# 2. GPIO Input and Output

A GPIO can generally be configured as:

```text
GPIO OUTPUT
     |
     +----> Control LED / Relay / Enable

GPIO INPUT
     |
     +----> Read Button / Sensor / Interrupt
```

---

# 3. Required Components

For a basic GPIO test:

```text
1 × BeagleBone Black
1 × LED
1 × 220 Ω – 1 kΩ resistor
1 × Push button
10 kΩ resistor for external pull-up/pull-down if required
Jumper wires
Breadboard
```

For an initial output test, an LED and current-limiting resistor are
sufficient.

---

# 4. GPIO Pin Selection

Before wiring any GPIO:

```text
hardware/pinout/gpio_pin_map.md
```

Use the project's GPIO pin map to select:

```text
GPIO number
Header
Physical pin
Pinmux mode
```

Do not assume that every header pin is a GPIO by default.

Some pins may be assigned to:

```text
UART
I2C
SPI
PWM
ADC
CAN
eMMC
HDMI
Other peripherals
```

The pin must be configured for GPIO operation before using it as a
GPIO.

---

# 5. GPIO Output – LED

The simplest GPIO output test is controlling an LED.

Circuit:

```text
BeagleBone GPIO
      |
      |
     [R]
   220Ω–1kΩ
      |
      |
     LED
      |
      |
     GND
```

More detailed:

```text
GPIO OUTPUT
     |
     v
   Resistor
     |
     v
    LED
     |
     v
    GND
```

The resistor limits the LED current.

---

# 6. LED Wiring

Typical LED terminals:

```text
Anode (+)
Cathode (-)
```

Recommended connection:

```text
GPIO ---- Resistor ---- LED Anode
                         LED Cathode ---- GND
```

Example:

```text
BBB GPIO
   |
   |
  470Ω
   |
   |
   +---->|---- GND
        LED
```

The LED orientation matters.

If the LED does not illuminate, verify the polarity.

---

# 7. GPIO Output Table

| Component   | BeagleBone Black  |
| ----------- | ----------------- |
| GPIO        | Selected GPIO pin |
| Resistor    | 220 Ω – 1 kΩ      |
| LED Anode   | Resistor output   |
| LED Cathode | GND               |

The exact GPIO/header pin should be selected from:

```text
hardware/pinout/gpio_pin_map.md
```

---

# 8. GPIO HIGH / LOW

When configured as an output:

```text
GPIO LOW
   |
   v
0 V

GPIO HIGH
   |
   v
Logic HIGH
```

Conceptually:

```text
GPIO = LOW
    |
    +---- LED OFF


GPIO = HIGH
    |
    +---- LED ON
```

The actual output voltage and current limits must follow the AM335x and
BeagleBone Black hardware specifications.

---

# 9. GPIO Input – Push Button

A GPIO input can be tested using a push button.

Basic circuit:

```text
             3.3 V
               |
               |
             Button
               |
               +--------> GPIO INPUT
               |
             Pull-down
               |
              GND
```

When the button is not pressed:

```text
GPIO = LOW
```

When the button is pressed:

```text
GPIO = HIGH
```

---

# 10. Push Button Wiring

Example:

```text
3.3 V
  |
  |
[ Push Button ]
  |
  +-----------> GPIO INPUT
  |
 [10 kΩ]
  |
 GND
```

The resistor keeps the GPIO input at a defined LOW state when the
button is released.

---

# 11. Pull-Up Configuration

An alternative is to use a pull-up resistor.

```text
3.3 V
  |
 [10 kΩ]
  |
  +-----------> GPIO INPUT
  |
[Button]
  |
 GND
```

Behavior:

```text
Button Released
      |
      v
GPIO = HIGH


Button Pressed
      |
      v
GPIO = LOW
```

This is called an **active-low** button configuration.

---

# 12. Internal Pull-Up / Pull-Down

Depending on the GPIO/pinmux configuration, an internal pull-up or
pull-down may be available.

Conceptually:

```text
GPIO
 |
 +---- Internal Pull-Up
 |
 +---- Internal Pull-Down
```

When using an internal pull resistor:

```text
External resistor
      |
      X
```

may not be required.

The exact pull configuration should be verified in the Device Tree
and SoC documentation.

---

# 13. GPIO Input and Output Together

A useful GPIO test uses:

```text
Push Button ---> GPIO INPUT
                      |
                      v
                Linux Application
                      |
                      v
                 GPIO OUTPUT
                      |
                      v
                     LED
```

Complete circuit:

```text
             +3.3 V
                |
             Button
                |
                +---------- GPIO INPUT
                |
              10 kΩ
                |
               GND


GPIO OUTPUT
     |
    470Ω
     |
    LED
     |
    GND
```

---

# 14. GPIO Test Concept

```text
Button Pressed
      |
      v
GPIO INPUT = HIGH
      |
      v
Linux Application
      |
      v
GPIO OUTPUT = HIGH
      |
      v
LED ON
```

When released:

```text
Button Released
      |
      v
GPIO INPUT = LOW
      |
      v
Linux Application
      |
      v
GPIO OUTPUT = LOW
      |
      v
LED OFF
```

---

# 15. GPIO Device Tree

Project Device Tree files:

```text
beaglebone-black/
└── device-tree/
    └── gpio/
        ├── bbb-gpio.dts
        ├── bbb-gpio.dtsi
        └── README.md
```

Conceptual Device Tree flow:

```text
bbb-gpio.dts
      |
      v
Pinmux Configuration
      |
      v
GPIO Controller
      |
      v
Linux GPIO Driver
      |
      v
GPIO Userspace / Kernel Driver
```

---

# 16. Pinmux Configuration

The AM335x pins are multiplexed.

A physical pin may support several functions:

```text
                    +--> GPIO
                    |
Physical Pin -------+--> UART
                    |
                    +--> SPI
                    |
                    +--> I2C
                    |
                    +--> PWM
```

Therefore, before using a pin as GPIO:

```text
Check pinmux
       |
       v
Configure GPIO mode
       |
       v
Use GPIO
```

---

# 17. GPIO Numbering

Be careful about the difference between:

```text
Physical header pin
GPIO controller/bank
GPIO line number
Linux GPIO identifier
```

Example concept:

```text
P8_XX
   |
   v
GPIOx_y
   |
   v
Linux GPIO line
```

Always use:

```text
hardware/pinout/gpio_pin_map.md
```

as the project's reference.

---

# 18. Modern Linux GPIO Interface

Modern Linux systems use the GPIO character-device interface.

Check GPIO chips:

```bash
gpioinfo
```

Example:

```text
gpiochip0
gpiochip1
gpiochip2
```

The exact GPIO chip numbering depends on the kernel and board
configuration.

---

# 19. Install GPIO Tools

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install gpiod
```

Check GPIO devices:

```bash
gpiodetect
```

Show GPIO lines:

```bash
gpioinfo
```

---

# 20. GPIO Output Test

Using `gpioset`:

```bash
gpioset gpiochip0 10=1
```

Set LOW:

```bash
gpioset gpiochip0 10=0
```

The exact chip and line number must be replaced with the GPIO selected
for this project.

Check:

```bash
gpioinfo
```

before testing.

---

# 21. GPIO Input Test

Using `gpioget`:

```bash
gpioget gpiochip0 10
```

Example:

```text
0
```

or:

```text
1
```

The returned value depends on the button state and whether the circuit
uses active-high or active-low logic.

---

# 22. GPIO Event / Interrupt Test

GPIO inputs can generate events.

Example:

```bash
gpiomon gpiochip0 10
```

Press the button.

The system should report GPIO transitions.

Conceptually:

```text
Button
   |
   v
GPIO Edge
   |
   v
Interrupt/Event
   |
   v
Linux GPIO Framework
   |
   v
Application
```

---

# 23. Rising and Falling Edge

A GPIO input can detect:

```text
Rising Edge
LOW  ---> HIGH
```

and:

```text
Falling Edge
HIGH ---> LOW
```

Example:

```text
        HIGH
          |
          |       +---------
          |       |
          |       |
LOW -------+-------+
          ^
          |
      Rising edge
```

---

# 24. GPIO Interrupt Flow

```text
External Signal
      |
      v
GPIO Pin
      |
      v
GPIO Controller
      |
      v
Interrupt
      |
      v
Linux Kernel
      |
      v
GPIO Driver
      |
      v
User / Kernel Application
```

For a kernel driver, the GPIO interrupt can be handled using Linux
interrupt APIs.

---

# 25. GPIO Test Circuit

Complete basic circuit:

```text
                         BeagleBone Black
                         +-------------+

3.3 V ---- Push Button -+---- GPIO INPUT
                         |
                        10kΩ
                         |
                        GND


GPIO OUTPUT ------------ 470Ω -------->|------ GND
                                      LED
```

---

# 26. GPIO Output Test Procedure

## Step 1 — Power Off

Disconnect power before changing wiring.

## Step 2 — Select GPIO

Check:

```text
hardware/pinout/gpio_pin_map.md
```

## Step 3 — Connect LED

```text
GPIO → Resistor → LED → GND
```

## Step 4 — Boot Linux

## Step 5 — Check GPIO

```bash
gpioinfo
```

## Step 6 — Set GPIO HIGH

```bash
gpioset gpiochipX LINE=1
```

## Step 7 — Verify LED

The LED should turn on.

## Step 8 — Set GPIO LOW

```bash
gpioset gpiochipX LINE=0
```

The LED should turn off.

---

# 27. GPIO Input Test Procedure

## Step 1 — Connect Button

```text
3.3 V
  |
Button
  |
GPIO INPUT
  |
10 kΩ
  |
GND
```

## Step 2 — Check GPIO

```bash
gpioinfo
```

## Step 3 — Read GPIO

```bash
gpioget gpiochipX LINE
```

## Step 4 — Press Button

Expected:

```text
GPIO = 1
```

## Step 5 — Release Button

Expected:

```text
GPIO = 0
```

---

# 28. GPIO Interrupt Test

Run:

```bash
gpiomon gpiochipX LINE
```

Press and release the button.

Expected behavior:

```text
Button Press
     |
     v
Rising Edge
     |
     v
GPIO Event


Button Release
     |
     v
Falling Edge
     |
     v
GPIO Event
```

---

# 29. GPIO LED + Button Test

A complete functional test can connect:

```text
Button → GPIO Input
              |
              v
          Application
              |
              v
        GPIO Output
              |
              v
             LED
```

Application behavior:

```text
IF button == PRESSED
        |
        +----> LED ON
ELSE
        |
        +----> LED OFF
```

---

# 30. GPIO Debugging

If the LED does not turn on:

```text
GPIO Failure
     |
     +--> Check GPIO pin
     |
     +--> Check pinmux
     |
     +--> Check GPIO line
     |
     +--> Check LED polarity
     |
     +--> Check resistor
     |
     +--> Check GND
     |
     +--> Check voltage
```

If button input does not change:

```text
GPIO Input Failure
       |
       +--> Check button wiring
       |
       +--> Check pull-up/pull-down
       |
       +--> Check GPIO line
       |
       +--> Check pinmux
       |
       +--> Check 3.3 V
       |
       +--> Check GND
```

---

# 31. Common GPIO Problems

## Problem: GPIO line is unavailable

Possible causes:

```text
[ ] Pin is assigned to another peripheral
[ ] Device Tree configuration
[ ] GPIO already requested
[ ] Incorrect GPIO chip
[ ] Incorrect GPIO line
```

## Problem: LED is always ON

Check:

```text
[ ] GPIO polarity
[ ] LED wiring
[ ] GPIO default state
[ ] Pull-up configuration
```

## Problem: Button input floats

Use:

```text
Pull-up
```

or:

```text
Pull-down
```

so that the GPIO always has a defined logic state.

---

# 32. GPIO Safety

Do not connect arbitrary external voltages directly to a GPIO.

Before connecting an external device:

```text
Check:
    |
    +--> Logic voltage
    |
    +--> Input/output direction
    |
    +--> Maximum current
    |
    +--> Pull-up/pull-down
    |
    +--> Electrical compatibility
```

Never assume a GPIO can directly drive:

```text
Relay coil
Motor
High-current LED
Large load
5 V logic
12 V logic
```

Use an appropriate driver/transistor/interface circuit where required.

---

# 33. GPIO With Transistor/Relay

For higher-current loads:

```text
GPIO
 |
 v
Resistor
 |
 v
Transistor
 |
 v
Load
 |
 v
Supply
```

Do not drive a relay coil directly from a GPIO.

For inductive loads, provide appropriate protection such as a flyback
diode where applicable.

---

# 34. GPIO Wiring Checklist

```text
[ ] GPIO pin selected
[ ] Header pin verified
[ ] Pinmux verified
[ ] GPIO mode configured
[ ] Correct voltage level
[ ] LED resistor installed
[ ] LED polarity verified
[ ] Button wiring verified
[ ] Pull-up/pull-down configured
[ ] GND connected
[ ] GPIO driver available
[ ] gpioinfo verified
[ ] GPIO output tested
[ ] GPIO input tested
[ ] GPIO interrupt tested
```

---

# 35. GPIO Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── gpio_pin_map.md
│   │
│   └── wiring/
│       └── gpio_wiring.md
│
├── hardware/
│   └── schematics/
│       └── gpio/
│           └── gpio_test_circuit.md
│
├── device-tree/
│   ├── gpio/
│   │   ├── bbb-gpio.dts
│   │   ├── bbb-gpio.dtsi
│   │   └── README.md
│   │
│   └── overlays/
│       └── bbb-gpio-overlay.dts
│
├── drivers/
│   └── gpio/
│       └── README.md
│
└── tests/
    └── gpio/
        ├── gpio_output_test.sh
        ├── gpio_input_test.sh
        ├── gpio_interrupt_test.sh
        └── README.md
```

---

# 36. Complete GPIO Bring-Up

```text
                    GPIO Hardware
                         |
                         v
                     GPIO Pin
                         |
                         v
                   AM335x GPIO
                         |
                         v
                  GPIO Controller
                         |
                         v
                    Linux GPIO
                         |
              +----------+----------+
              |                     |
              v                     v
         GPIO Output           GPIO Input
              |                     |
              v                     v
             LED                  Button
              |                     |
              +----------+----------+
                         |
                         v
                   User Application
```

---

# 37. Final Test Objective

The objective of this wiring test is to validate the complete GPIO
path:

```text
Hardware Signal
      ↓
GPIO Pin
      ↓
AM335x GPIO Controller
      ↓
Linux GPIO Driver
      ↓
GPIO Character Device
      ↓
User Application
      ↓
LED / Button / Sensor
```

The recommended first tests are:

```text
1. GPIO output → LED ON/OFF
2. GPIO input → Button HIGH/LOW
3. GPIO interrupt → Button edge detection
4. GPIO input → Application-controlled LED
```

**Important:** Always verify the selected pin's voltage level, pinmux,
GPIO number/line, and electrical limits before connecting external
hardware.

````

**File location:**

```text
beaglebone-black/hardware/wiring/gpio_wiring.md
````

