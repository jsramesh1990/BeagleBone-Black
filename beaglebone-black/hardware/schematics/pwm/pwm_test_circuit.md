# `pwm_test_circuit.md`

````markdown
# BeagleBone Black PWM Test Circuit

## 1. Overview

This document describes a hardware test circuit for validating PWM
(Pulse Width Modulation) functionality on the BeagleBone Black.

PWM can be used to control:

- LED brightness
- Motor speed
- Servo position
- Buzzer tone
- Fan speed
- Power control
- Other actuator devices

The complete software-to-hardware path is:

```text
User Application
       |
       v
Linux PWM Framework
       |
       v
PWM Controller Driver
       |
       v
AM335x PWM Hardware
       |
       v
PWM Output Pin
       |
       +---------> LED
       |
       +---------> Servo
       |
       +---------> Motor Driver
````

---

# 2. PWM Test Objectives

This test validates:

* PWM pin configuration
* PWM Device Tree configuration
* Linux PWM framework
* PWM period
* PWM duty cycle
* PWM enable/disable
* PWM frequency
* LED brightness control
* Servo PWM signal generation
* Motor-control PWM generation

---

# 3. Required Components

For a basic LED PWM test:

```text
1 × BeagleBone Black
1 × LED
1 × 330 Ω resistor
Breadboard
Jumper wires
```

For a servo test:

```text
1 × BeagleBone Black
1 × 3.3 V-compatible servo signal interface
External servo power supply as required
Jumper wires
```

For a motor test:

```text
1 × BeagleBone Black
1 × DC motor
1 × Motor driver
External motor power supply
Jumper wires
```

> Do not connect a motor directly to a BeagleBone Black GPIO/PWM pin.

---

# 4. PWM LED Test Circuit

The simplest PWM test uses an LED.

```text
             BeagleBone Black

                  PWM
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
PWM OUTPUT
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

The PWM signal rapidly switches the LED ON and OFF.

Changing the duty cycle changes the average LED brightness.

---

# 5. PWM Duty Cycle

PWM consists of:

```text
HIGH time
+
LOW time
=
PWM period
```

Example:

```text
      HIGH        LOW
       |           |
       v           v

       ┌───────┐
       │       │
───────┘       └────────
       <------->
         Period
```

Duty cycle:

```text
Duty Cycle (%) =
    HIGH Time / Period × 100
```

---

# 6. Duty Cycle Examples

### 0% Duty Cycle

```text
LOW LOW LOW LOW LOW
```

LED:

```text
OFF
```

### 25% Duty Cycle

```text
 ┌─┐        ┌─┐
 │ │        │ │
─┘ └────────┘ └────
```

LED:

```text
Low brightness
```

### 50% Duty Cycle

```text
 ┌────┐    ┌────┐
 │    │    │    │
─┘    └────┘    └──
```

LED:

```text
Medium brightness
```

### 75% Duty Cycle

```text
 ┌──────┐  ┌──────┐
 │      │  │      │
─┘      └──┘      └─
```

LED:

```text
High brightness
```

### 100% Duty Cycle

```text
HIGH HIGH HIGH HIGH
```

LED:

```text
Fully ON
```

---

# 7. PWM Frequency

PWM frequency is determined by:

```text
Frequency = 1 / Period
```

Example:

```text
Period = 1 ms

Frequency = 1 / 0.001
          = 1000 Hz
          = 1 kHz
```

For LED brightness control, a PWM frequency in the hundreds of Hz to
several kHz range is commonly suitable.

The appropriate frequency depends on the application.

---

# 8. Complete LED PWM Circuit

```text
                         BeagleBone Black
                        +----------------+
                        |                |
                        | PWM OUTPUT     |
                        +-------+--------+
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

---

# 9. PWM Architecture

```text
                 User Application
                        |
                        v
                 Linux PWM API
                        |
                        v
                 PWM Framework
                        |
                        v
                PWM Controller
                        |
                        v
                 AM335x PWM HW
                        |
                        v
                   Pinmux/PWM
                        |
                        v
                  Header Pin
                        |
                        v
                      LED
```

---

# 10. Device Tree

The PWM controller and pinmux must be configured through Device Tree.

Project files:

```text
beaglebone-black/
└── device-tree/
    └── pwm/
        ├── bbb-pwm.dts
        ├── bbb-pwm.dtsi
        └── README.md
```

Conceptual flow:

```text
bbb-pwm.dts
      |
      v
Pinmux Configuration
      |
      v
PWM Controller
      |
      v
Linux PWM Driver
      |
      v
PWM Framework
      |
      v
PWM Output
```

---

# 11. PWM Pin Selection

Select the required PWM-capable header pin from:

```text
hardware/pinout/pwm_pin_map.md
```

Before connecting the circuit:

```text
1. Select PWM-capable pin
2. Verify P8/P9 header location
3. Check pinmux
4. Check that the pin is not being used by another peripheral
5. Verify voltage level
6. Connect the LED circuit
```

---

# 12. PWM Pinmux

A BeagleBone Black header pin can support multiple functions.

Conceptually:

```text
Header Pin
    |
    +---- GPIO
    |
    +---- UART
    |
    +---- SPI
    |
    +---- I2C
    |
    +---- PWM
```

The pin must be configured for the PWM function.

```text
Pinmux
   |
   v
PWM Mode
   |
   v
PWM Controller
```

---

# 13. Linux PWM Interface

Depending on the kernel version and board configuration, PWM may be
exposed through the Linux PWM framework.

Check the system for PWM-related interfaces:

```bash
ls /sys/class/pwm/
```

Example:

```text
pwmchip0
```

Check:

```bash
ls -l /sys/class/pwm/
```

The exact PWM chip and channel numbering depend on the kernel and
Device Tree configuration.

---

# 14. PWM Sysfs Test

On systems where the legacy PWM sysfs interface is enabled, a PWM
channel can be tested using:

```bash
cd /sys/class/pwm/pwmchip0
```

Check channels:

```bash
ls
```

Export a channel:

```bash
echo 0 > export
```

Then:

```bash
cd pwm0
```

---

# 15. Configure PWM Period

Example:

```bash
echo 1000000 > period
```

The period is specified in nanoseconds in the legacy sysfs interface.

```text
1,000,000 ns
      |
      v
1 ms
```

Therefore:

```text
Frequency = 1 kHz
```

---

# 16. Configure PWM Duty Cycle

For 50% duty cycle:

```bash
echo 500000 > duty_cycle
```

Configuration:

```text
Period     = 1,000,000 ns
Duty Cycle =   500,000 ns
```

Therefore:

```text
Duty Cycle = 50%
```

---

# 17. Enable PWM

After configuring period and duty cycle:

```bash
echo 1 > enable
```

The PWM output should now be active.

For an LED:

```text
PWM enabled
     |
     v
LED brightness visible
```

---

# 18. Disable PWM

Disable the PWM channel:

```bash
echo 0 > enable
```

The PWM output is disabled.

---

# 19. PWM Test Example

Example:

```bash
cd /sys/class/pwm/pwmchip0

echo 0 > export

cd pwm0

echo 1000000 > period
echo 500000 > duty_cycle
echo 1 > enable
```

Expected:

```text
PWM Frequency = 1 kHz
Duty Cycle    = 50%
```

The LED should appear approximately half brightness compared with
100% duty cycle, subject to LED characteristics and human perception.

---

# 20. PWM Duty-Cycle Test

### 10%

```bash
echo 100000 > duty_cycle
```

### 25%

```bash
echo 250000 > duty_cycle
```

### 50%

```bash
echo 500000 > duty_cycle
```

### 75%

```bash
echo 750000 > duty_cycle
```

### 100%

```bash
echo 1000000 > duty_cycle
```

For:

```text
period = 1000000 ns
```

the corresponding duty cycles are:

```text
100000 ns   → 10%
250000 ns   → 25%
500000 ns   → 50%
750000 ns   → 75%
1000000 ns  → 100%
```

---

# 21. PWM Waveform

For 50% duty cycle:

```text
Voltage

HIGH       ┌────────┐          ┌────────┐
           │        │          │        │
LOW  ──────┘        └──────────┘        └──────

           <-------- Period -------->
```

For 25%:

```text
HIGH       ┌───┐             ┌───┐
           │   │             │   │
LOW  ──────┘   └─────────────┘   └────────
```

For 75%:

```text
HIGH       ┌────────────┐    ┌────────────┐
           │            │    │            │
LOW  ──────┘            └────┘            └────
```

---

# 22. Oscilloscope Test

Connect an oscilloscope:

```text
Oscilloscope CH1
        |
        v
PWM OUTPUT
        |
        v
BeagleBone PWM Pin
```

Ground:

```text
Oscilloscope GND
       |
       v
BBB GND
```

Observe:

```text
Frequency
Period
Duty Cycle
Voltage level
Rise time
Fall time
```

---

# 23. Expected Oscilloscope Signal

For:

```text
Frequency = 1 kHz
Duty Cycle = 50%
```

Expected:

```text
        1 ms
<---------------->

       ┌───────┐
       │       │
───────┘       └───────
       0.5 ms
```

The measured waveform should approximately match the configured
period and duty cycle.

---

# 24. PWM Servo Test

PWM can also control a servo.

Basic circuit:

```text
                  BeagleBone Black
                  +--------------+
                  |              |
PWM SIGNAL ------>| Servo Signal |
                  |              |
GND ------------->| Servo GND    |
                  +--------------+
                         |
                         v
                       Servo
```

Servo power should normally come from an appropriate external supply,
not directly from the BeagleBone Black's 3.3 V GPIO rail unless the
servo and board power design explicitly support it.

Connect grounds appropriately so the PWM signal has a valid reference.

---

# 25. Servo PWM Concept

A typical hobby servo uses a pulse period around:

```text
20 ms
```

which corresponds to:

```text
50 Hz
```

The pulse width determines the commanded position.

Conceptually:

```text
Position A:

┌─┐
│ │
┘ └────────────────────
  ~1 ms


Position Center:

┌──┐
│  │
┘  └───────────────────
   ~1.5 ms


Position B:

┌───┐
│   │
┘   └──────────────────
    ~2 ms
```

The exact pulse-width range depends on the servo.

---

# 26. PWM Motor Control

A motor must not be connected directly to the PWM pin.

Correct architecture:

```text
BeagleBone PWM
      |
      v
Motor Driver
      |
      v
DC Motor
      |
      v
External Power Supply
```

Example:

```text
                   PWM
                    |
                    v
             +-------------+
             | Motor       |
             | Driver      |
             +------+------+
                    |
                    v
                  Motor
                    |
              External Supply
```

The motor driver handles the required current and voltage.

---

# 27. Motor PWM Circuit

```text
                BeagleBone Black

                   PWM OUT
                      |
                      v
              +---------------+
              | Motor Driver   |
              +-------+-------+
                      |
                      |
                    Motor
                      |
                      |
               External Supply
```

Ground/reference connections must follow the motor-driver
manufacturer's wiring requirements.

---

# 28. PWM Fan Control

PWM can also be used for fan-speed control.

```text
BeagleBone PWM
      |
      v
Fan Driver / Controller
      |
      v
PWM Fan
```

Do not drive a high-current fan directly from the BeagleBone PWM pin.

---

# 29. PWM Debugging

If there is no PWM output:

```text
PWM Not Working
      |
      +--> Check PWM-capable pin
      |
      +--> Check pinmux
      |
      +--> Check Device Tree
      |
      +--> Check PWM driver
      |
      +--> Check PWM framework
      |
      +--> Check pwmchip
      |
      +--> Check channel
      |
      +--> Check period
      |
      +--> Check duty cycle
      |
      +--> Check enable
```

---

# 30. Check Kernel Logs

Run:

```bash
dmesg | grep -i pwm
```

Also check:

```bash
dmesg | grep -i pinctrl
```

And:

```bash
ls /sys/class/pwm/
```

---

# 31. PWM Device Tree Debugging

Verify:

```text
[ ] PWM controller enabled
[ ] Correct PWM channel
[ ] Correct pinmux
[ ] Correct header pin
[ ] No peripheral conflict
[ ] PWM driver loaded
[ ] PWM framework registered
[ ] Correct period
[ ] Correct duty cycle
```

---

# 32. PWM Pin Conflict

A pin configured for PWM cannot simultaneously be used for another
function on the same pin.

Example:

```text
PWM Pin
   |
   +---- PWM
   |
   +---- GPIO
   |
   +---- Other Peripheral
```

Device Tree pinmux determines the active function.

---

# 33. PWM Hardware Verification

Before software testing:

```text
[ ] PWM pin verified
[ ] LED resistor connected
[ ] LED polarity checked
[ ] GND connected
[ ] PWM voltage level verified
[ ] No 5 V signal connected to PWM pin
[ ] Motor/servo not connected directly to PWM pin
```

---

# 34. PWM LED Test Procedure

### Step 1

Connect:

```text
PWM → 330 Ω → LED → GND
```

### Step 2

Boot Linux.

### Step 3

Check PWM:

```bash
ls /sys/class/pwm/
```

### Step 4

Export the appropriate PWM channel where supported:

```bash
echo 0 > /sys/class/pwm/pwmchip0/export
```

### Step 5

Configure period:

```bash
echo 1000000 > /sys/class/pwm/pwmchip0/pwm0/period
```

### Step 6

Configure duty cycle:

```bash
echo 500000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
```

### Step 7

Enable:

```bash
echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable
```

### Step 8

Observe:

```text
LED brightness ≈ 50%
```

---

# 35. Change LED Brightness

Increase brightness:

```bash
echo 750000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
```

Expected:

```text
LED → brighter
```

Decrease brightness:

```bash
echo 250000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
```

Expected:

```text
LED → dimmer
```

---

# 36. PWM Test Script

Example:

```bash
#!/bin/bash

PWMCHIP=/sys/class/pwm/pwmchip0
PWM=pwm0

echo 0 > ${PWMCHIP}/export

echo 1000000 > ${PWMCHIP}/${PWM}/period
echo 500000 > ${PWMCHIP}/${PWM}/duty_cycle
echo 1 > ${PWMCHIP}/${PWM}/enable

echo "PWM enabled"
echo "Period: 1 ms"
echo "Frequency: 1 kHz"
echo "Duty cycle: 50%"
```

The actual PWM chip/channel must match the board's configuration.

---

# 37. PWM Cleanup

Disable:

```bash
echo 0 > /sys/class/pwm/pwmchip0/pwm0/enable
```

Then unexport:

```bash
echo 0 > /sys/class/pwm/pwmchip0/unexport
```

---

# 38. PWM Driver Architecture

```text
+--------------------------------+
| User Application               |
+---------------+----------------+
                |
                v
+--------------------------------+
| Linux PWM Interface            |
+---------------+----------------+
                |
                v
+--------------------------------+
| Linux PWM Framework             |
+---------------+----------------+
                |
                v
+--------------------------------+
| AM335x PWM Driver              |
+---------------+----------------+
                |
                v
+--------------------------------+
| PWM Hardware                   |
+---------------+----------------+
                |
                v
+--------------------------------+
| Pinmux / Header Pin            |
+---------------+----------------+
                |
                v
             Device
```

---

# 39. PWM Test Flow

```text
PWM Hardware
      |
      v
PWM Pin Map
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
PWM Driver
      |
      v
Linux PWM Framework
      |
      v
PWM Channel
      |
      +----------+
      |          |
      v          v
    Period     Duty Cycle
      |          |
      +-----+----+
            |
            v
       PWM Output
            |
            v
          LED
```

---

# 40. PWM Test Checklist

```text
[ ] PWM-capable pin identified
[ ] Header pin verified
[ ] Pinmux configured
[ ] Device Tree configured
[ ] PWM controller enabled
[ ] PWM driver loaded
[ ] PWM chip detected
[ ] PWM channel identified
[ ] Period configured
[ ] Duty cycle configured
[ ] PWM enabled
[ ] LED brightness tested
[ ] Oscilloscope waveform tested
[ ] Frequency verified
[ ] Duty cycle verified
[ ] PWM disabled successfully
```

---

# 41. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── pwm_pin_map.md
│   │
│   └── schematics/
│       └── pwm/
│           └── pwm_test_circuit.md
│
├── device-tree/
│   └── pwm/
│       ├── bbb-pwm.dts
│       ├── bbb-pwm.dtsi
│       └── README.md
│
├── drivers/
│   └── pwm/
│       └── README.md
│
└── tests/
    └── pwm/
        ├── pwm_led_test.sh
        ├── pwm_servo_test.sh
        ├── pwm_motor_test.sh
        └── README.md
```

---

# 42. Complete PWM Bring-Up

```text
                     PWM Hardware
                          |
                          v
                     PWM Pin Map
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
                      PWM Driver
                          |
                          v
                   Linux PWM Core
                          |
                          v
                    PWM Channel
                          |
                +---------+---------+
                |                   |
                v                   v
             Period             Duty Cycle
                |                   |
                +---------+---------+
                          |
                          v
                     PWM Output
                          |
                +---------+---------+
                |                   |
                v                   v
               LED              Servo/Motor
```

---

# 43. Final Objective

The purpose of this test circuit is to validate the complete PWM path on
the BeagleBone Black:

```text
User Application
       ↓
Linux PWM Framework
       ↓
PWM Driver
       ↓
AM335x PWM Hardware
       ↓
Pinmux
       ↓
PWM Header Pin
       ↓
LED / Servo / Motor Driver
```

The LED test validates PWM generation and duty-cycle control.

The oscilloscope test validates the actual electrical PWM waveform.

The servo or motor-driver test validates PWM usage in a practical
embedded-control application.

> For the final driver project, validate PWM using both software
> configuration and an oscilloscope so that the configured period and
> duty cycle are verified on the physical pin.

````

**File location:**

```text
beaglebone-black/hardware/schematics/pwm/pwm_test_circuit.md
````

