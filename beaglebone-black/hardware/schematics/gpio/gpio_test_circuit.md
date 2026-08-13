# `gpio_test_circuit.md`

````markdown
# BeagleBone Black GPIO Test Circuit

## 1. Overview

This document describes a simple hardware test circuit for validating
GPIO input and output functionality on the BeagleBone Black.

The test verifies the complete path from Linux user space to the physical
GPIO pin.

```text
Linux Application
       |
       v
GPIO Character Device
       |
       v
Linux GPIO Framework
       |
       v
AM335x GPIO Controller
       |
       v
BeagleBone Black Header
       |
       v
LED / Push Button
````

---

## 2. GPIO Test Objectives

The GPIO test covers:

* GPIO output
* GPIO input
* LED control
* Push-button detection
* GPIO direction configuration
* GPIO state reading
* Device Tree pinmux
* Linux GPIO framework
* GPIO interrupt testing

---

## 3. Required Components

```text
1 × BeagleBone Black
1 × LED
1 × 330 Ω resistor
1 × Push button
1 × 10 kΩ resistor
Breadboard
Jumper wires
```

---

# 4. GPIO Output Test

The simplest GPIO output test uses an LED.

```text
BBB GPIO
   |
   |
  330 Ω
   |
   |
  LED
   |
   |
  GND
```

Complete circuit:

```text
             BeagleBone Black

                 GPIO
                   |
                   |
                330 Ω
                   |
                   |
                 LED
                   |
                   |
                  GND
```

When GPIO is HIGH:

```text
GPIO = 1
   |
   v
LED ON
```

When GPIO is LOW:

```text
GPIO = 0
   |
   v
LED OFF
```

---

# 5. LED Polarity

An LED has two terminals:

```text
Anode (+)
Cathode (-)
```

Connect:

```text
GPIO → Resistor → LED Anode
LED Cathode → GND
```

Example:

```text
GPIO ----[330Ω]---->|---- GND
                    LED
```

The resistor limits LED current.

> Always use a current-limiting resistor with a discrete LED.

---

# 6. GPIO Input Test

A push button can be used to test GPIO input.

```text
             3.3 V
               |
               |
            Push Button
               |
               +----------> GPIO INPUT
               |
             10 kΩ
               |
              GND
```

In this configuration:

```text
Button released → GPIO = LOW

Button pressed  → GPIO = HIGH
```

---

# 7. Complete GPIO Input Circuit

```text
                 3.3 V
                   |
                   |
                +--+--+
                |Button|
                +--+--+
                   |
                   |
                   +------------ GPIO INPUT
                   |
                 10 kΩ
                   |
                  GND
```

The resistor acts as a pull-down resistor.

---

# 8. Combined GPIO Test Circuit

The project can test both input and output simultaneously.

```text
                         BeagleBone Black
                       +-------------------+
                       |                   |
                       | GPIO OUTPUT ------+----[330Ω]---->|---- GND
                       |                   |                LED
                       |                   |
                       | GPIO INPUT <------+---- Push Button
                       |                   |         |
                       |                   |       3.3V
                       |                   |
                       +-------------------+
```

A more complete representation:

```text
                 GPIO OUTPUT
                      |
                    330Ω
                      |
                     LED
                      |
                     GND


                 GPIO INPUT
                      |
                      +---------+
                      |         |
                   Button     10kΩ
                      |         |
                    3.3V       GND
```

---

# 9. GPIO Pin Selection

The exact GPIO header pin should be selected from:

```text
hardware/pinout/gpio_pin_map.md
```

Before wiring the circuit:

```text
1. Select GPIO
2. Check P8/P9 pin
3. Check pinmux
4. Check that the pin is not used by another peripheral
5. Verify voltage level
6. Connect circuit
```

---

# 10. GPIO Voltage

BeagleBone Black GPIOs operate in the 3.3 V I/O domain.

Do not connect a 5 V signal directly to a GPIO input.

```text
Correct:

3.3 V → GPIO


Incorrect:

5 V → GPIO
```

Always verify the electrical requirements of the connected circuit.

---

# 11. GPIO Test Architecture

```text
                 User Application
                        |
                        v
                GPIO Character API
                        |
                        v
                 Linux GPIO Layer
                        |
                        v
                GPIO Controller
                        |
                        v
                   Pinmux
                        |
                        v
                  Header Pin
                   /       \
                  /         \
                 v           v
               LED         Button
             OUTPUT         INPUT
```

---

# 12. Device Tree

GPIO pin configuration is controlled through Device Tree and pinctrl.

Project files:

```text
beaglebone-black/
└── device-tree/
    └── gpio/
        ├── bbb-gpio.dts
        ├── bbb-gpio.dtsi
        └── README.md
```

Conceptual flow:

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
Linux GPIO Framework
      |
      v
User Space
```

---

# 13. Modern Linux GPIO Interface

For modern Linux systems, prefer the GPIO character-device interface
rather than the old sysfs GPIO interface.

Useful tools from `libgpiod` include:

```text
gpiodetect
gpioinfo
gpioget
gpioset
gpiomon
```

Install on Debian/Ubuntu:

```bash
sudo apt update
sudo apt install gpiod
```

---

# 14. Detect GPIO Controllers

Run:

```bash
gpiodetect
```

Example:

```text
gpiochip0 [gpio...] (32 lines)
gpiochip1 [gpio...] (32 lines)
```

The exact GPIO chip names and number of lines depend on the kernel and
board configuration.

---

# 15. GPIO Information

Run:

```bash
gpioinfo
```

This displays information about GPIO lines.

Example:

```text
gpiochip0 - 32 lines:
        line   0: "..." unused input active-high
        line   1: "..." unused output active-high
```

Use this to identify whether a GPIO line is already being used.

---

# 16. GPIO Output Test

A GPIO can be driven using `gpioset`.

Generic example:

```bash
gpioset gpiochip0 20=1
```

This sets a GPIO line HIGH.

Set LOW:

```bash
gpioset gpiochip0 20=0
```

The exact chip and line number must be determined from the actual
BeagleBone Black GPIO mapping.

---

# 17. GPIO Input Test

Read a GPIO using:

```bash
gpioget gpiochip0 20
```

Example:

```text
0
```

or:

```text
1
```

The result depends on the push-button state.

---

# 18. GPIO Button Monitoring

Use:

```bash
gpiomon gpiochip0 20
```

This can be used to monitor GPIO edge events.

Conceptually:

```text
Button
   |
   v
GPIO Input
   |
   v
Edge Detection
   |
   v
Linux GPIO
   |
   v
gpiomon
```

---

# 19. GPIO Output Test Flow

```text
Terminal
   |
   v
gpioset
   |
   v
GPIO Character Device
   |
   v
Linux GPIO Framework
   |
   v
AM335x GPIO Controller
   |
   v
GPIO Pin
   |
   v
LED
```

Test:

```bash
gpioset gpiochipX LINE=1
```

Expected:

```text
LED ON
```

Then:

```bash
gpioset gpiochipX LINE=0
```

Expected:

```text
LED OFF
```

Replace `gpiochipX` and `LINE` with the actual values for the selected
GPIO.

---

# 20. GPIO Input Test Flow

```text
Push Button
     |
     v
GPIO Pin
     |
     v
AM335x GPIO Controller
     |
     v
Linux GPIO Framework
     |
     v
gpioget / gpiomon
     |
     v
Terminal
```

Example:

```bash
gpioget gpiochipX LINE
```

Expected:

```text
Button released → 0
Button pressed  → 1
```

---

# 21. GPIO Interrupt Test

A button can also be used to generate GPIO edge events.

```text
Button
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
User Application
```

Monitor events:

```bash
gpiomon gpiochipX LINE
```

Press and release the button and observe the generated events.

---

# 22. Pull-Up Configuration

Instead of a pull-down, the GPIO can use a pull-up arrangement.

```text
             3.3 V
               |
              10kΩ
               |
               +---------- GPIO INPUT
               |
             Button
               |
              GND
```

Now:

```text
Button released → GPIO = HIGH

Button pressed  → GPIO = LOW
```

This is called an **active-low input**.

---

# 23. Active-High vs Active-Low

### Active High

```text
Button pressed
      |
      v
GPIO = 1
```

### Active Low

```text
Button pressed
      |
      v
GPIO = 0
```

The polarity should be documented in the Device Tree and driver design.

---

# 24. GPIO Pinmux

The selected physical pin must be configured for GPIO functionality.

Conceptually:

```text
Physical Header Pin
        |
        v
      Pinmux
        |
        +---- GPIO
        +---- UART
        +---- SPI
        +---- I2C
        +---- PWM
```

Only the required function should be selected.

Check:

```text
hardware/pinout/gpio_pin_map.md
```

before configuring a GPIO.

---

# 25. GPIO Pin Conflict

A GPIO may also support another peripheral function.

For example:

```text
P9.xx
  |
  +---- GPIO
  |
  +---- UART
  |
  +---- SPI
```

If that pin is configured for UART, it should not simultaneously be
treated as a normal GPIO for the same function.

Always check:

```text
[ ] Pinmux
[ ] Device Tree
[ ] Existing peripherals
[ ] Pin conflicts
```

---

# 26. Hardware Verification

Before software testing:

```text
[ ] Correct GPIO pin selected
[ ] Correct P8/P9 pin verified
[ ] LED polarity verified
[ ] LED resistor connected
[ ] Button connected
[ ] Pull-up/pull-down resistor connected
[ ] GND connected
[ ] 3.3 V verified
[ ] No 5 V GPIO signal
```

---

# 27. LED Test Procedure

### Step 1

Connect:

```text
GPIO → 330 Ω → LED → GND
```

### Step 2

Boot Linux.

### Step 3

Check GPIO:

```bash
gpioinfo
```

### Step 4

Set GPIO HIGH:

```bash
gpioset gpiochipX LINE=1
```

### Step 5

Observe LED:

```text
LED → ON
```

### Step 6

Set GPIO LOW:

```bash
gpioset gpiochipX LINE=0
```

Expected:

```text
LED → OFF
```

---

# 28. Button Test Procedure

### Step 1

Connect:

```text
3.3V
 |
Button
 |
GPIO
 |
10kΩ
 |
GND
```

### Step 2

Boot Linux.

### Step 3

Check GPIO:

```bash
gpioinfo
```

### Step 4

Read GPIO:

```bash
gpioget gpiochipX LINE
```

### Step 5

Press button.

Run again:

```bash
gpioget gpiochipX LINE
```

Expected:

```text
Released → 0
Pressed  → 1
```

for the pull-down configuration.

---

# 29. GPIO Debugging

If the LED does not turn on:

```text
LED OFF
  |
  +--> Check GPIO number
  |
  +--> Check header pin
  |
  +--> Check pinmux
  |
  +--> Check Device Tree
  |
  +--> Check LED polarity
  |
  +--> Check resistor
  |
  +--> Check GND
  |
  +--> Check GPIO ownership
```

---

# 30. Button Debugging

If the button does not change the GPIO value:

```text
Button Not Detected
        |
        +--> Check GPIO pin
        |
        +--> Check button wiring
        |
        +--> Check pull-up/pull-down
        |
        +--> Check 3.3V
        |
        +--> Check GND
        |
        +--> Check pinmux
        |
        +--> Check GPIO ownership
```

---

# 31. Check Kernel Messages

Use:

```bash
dmesg | grep -i gpio
```

Also:

```bash
dmesg | grep -i pinctrl
```

For Device Tree/pinctrl debugging:

```bash
dmesg | grep -i pinctrl
```

---

# 32. GPIO Device Tree Flow

```text
                 bbb-gpio.dts
                       |
                       v
                  pinctrl node
                       |
                       v
                GPIO Controller
                       |
                       v
                 Linux GPIO Core
                       |
                       v
                GPIO Character Dev
                       |
                       v
              gpioget / gpioset
                       |
              +--------+--------+
              |                 |
              v                 v
           Button              LED
```

---

# 33. GPIO Driver Architecture

```text
+--------------------------------+
| User Application               |
+---------------+----------------+
                |
                v
+--------------------------------+
| GPIO Character Device          |
+---------------+----------------+
                |
                v
+--------------------------------+
| Linux GPIO Subsystem           |
+---------------+----------------+
                |
                v
+--------------------------------+
| AM335x GPIO Driver             |
+---------------+----------------+
                |
                v
+--------------------------------+
| AM335x GPIO Controller         |
+---------------+----------------+
                |
                v
+--------------------------------+
| BeagleBone Black GPIO Pin      |
+--------------------------------+
```

---

# 34. GPIO Test Checklist

```text
[ ] GPIO pin identified
[ ] P8/P9 header verified
[ ] Pinmux configured
[ ] Device Tree configured
[ ] GPIO controller detected
[ ] GPIO line identified
[ ] gpioinfo checked
[ ] LED circuit connected
[ ] LED output tested
[ ] Button circuit connected
[ ] GPIO input tested
[ ] Pull-up/pull-down verified
[ ] GPIO interrupt tested
[ ] Kernel logs checked
[ ] Pin conflicts checked
```

---

# 35. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── gpio_pin_map.md
│   │
│   └── schematics/
│       └── gpio/
│           └── gpio_test_circuit.md
│
├── device-tree/
│   └── gpio/
│       ├── bbb-gpio.dts
│       ├── bbb-gpio.dtsi
│       └── README.md
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
                  GPIO Pin Map
                       |
                       v
                 Test Circuit
                       |
                       v
                    Pinmux
                       |
                       v
                  Device Tree
                       |
                       v
                 GPIO Driver
                       |
                       v
               Linux GPIO Core
                       |
                       v
             Character Device API
                       |
              +--------+--------+
              |                 |
              v                 v
           gpioset           gpioget
              |                 |
              v                 v
             LED              Button
```

---

# 37. Final Objective

The purpose of this test circuit is to validate both GPIO output and
GPIO input on the BeagleBone Black.

The complete validation path is:

```text
GPIO Output:

Linux Application
      ↓
GPIO Character Device
      ↓
Linux GPIO Framework
      ↓
AM335x GPIO Controller
      ↓
GPIO Pin
      ↓
Resistor
      ↓
LED


GPIO Input:

Push Button
      ↓
GPIO Pin
      ↓
AM335x GPIO Controller
      ↓
Linux GPIO Framework
      ↓
GPIO Character Device
      ↓
User Application
```

This provides the hardware foundation for the GPIO portion of the
BeagleBone Black device-driver project.

```
```

