# `pwm_wiring.md`

````markdown
# BeagleBone Black PWM Wiring

## 1. Overview

This document describes the hardware wiring required to test PWM
(Pulse Width Modulation) functionality on the BeagleBone Black.

PWM is commonly used for:

- LED brightness control
- Motor speed control
- Servo control
- Fan speed control
- Buzzer/tone generation
- Power regulation
- Actuator control
- Embedded control applications

Basic PWM flow:

```text
User Application
       |
       v
Linux PWM Framework
       |
       v
PWM Driver
       |
       v
AM335x PWM Controller
       |
       v
PWM Output Pin
       |
       +------> LED / Servo / Motor Driver
````

---

# 2. PWM Concept

PWM generates a periodic digital waveform.

```text
HIGH       +------+          +------+
           |      |          |      |
           |      |          |      |
LOW  ------+      +----------+      +----------

           <---- Period ---->

Duty Cycle = HIGH time / Total period
```

For example:

```text
20% Duty Cycle

HIGH  +---+
      |   |
------+   +----------------------

50% Duty Cycle

HIGH  +-------+
      |       |
------+       +-------------------

80% Duty Cycle

HIGH  +-------------+
      |             |
------+             +--------------
```

---

# 3. Required Components

For a basic PWM test:

```text
1 × BeagleBone Black
1 × LED
1 × 220 Ω – 1 kΩ resistor
Jumper wires
Breadboard
```

For servo testing:

```text
1 × 3.3 V-compatible servo/control interface
External power supply if required
Jumper wires
```

For motor testing:

```text
1 × DC motor
1 × Motor driver
External motor power supply
Flyback/protection circuitry as required
```

Do not connect a motor directly to a BeagleBone GPIO/PWM pin.

---

# 4. PWM Pin Selection

Before wiring:

```text
hardware/pinout/pwm_pin_map.md
```

Use this file to identify:

```text
PWM controller
PWM channel
Header pin
Physical pin
Pinmux mode
```

The BeagleBone Black uses multiplexed pins, so the selected pin must be
configured for PWM functionality.

---

# 5. Basic PWM Wiring

For an LED:

```text
BeagleBone PWM
      |
      v
   Resistor
   220Ω–1kΩ
      |
      v
     LED
      |
      v
     GND
```

Complete circuit:

```text
PWM OUTPUT
    |
    |
   470Ω
    |
    |
   LED
    |
    |
   GND
```

---

# 6. PWM LED Wiring Table

| BeagleBone Black | Component |
| ---------------- | --------- |
| PWM Output       | Resistor  |
| Resistor         | LED Anode |
| LED Cathode      | GND       |

Use the exact PWM-capable header pin from:

```text
hardware/pinout/pwm_pin_map.md
```

---

# 7. LED Polarity

LED terminals:

```text
Anode   (+)
Cathode (-)
```

Recommended:

```text
PWM ---- Resistor ---- LED Anode
                       LED Cathode ---- GND
```

If the LED does not illuminate:

```text
Check LED polarity.
```

---

# 8. PWM Duty Cycle

Duty cycle determines how long the PWM signal remains HIGH during each
period.

```text
Duty Cycle = ON Time / Period × 100
```

Example:

```text
Period = 1 ms
ON Time = 0.5 ms

Duty Cycle = 0.5 / 1 × 100
           = 50%
```

---

# 9. PWM LED Brightness

PWM can control the perceived brightness of an LED.

```text
0% Duty Cycle
    |
    v
LED OFF


25% Duty Cycle
    |
    v
Low Brightness


50% Duty Cycle
    |
    v
Medium Brightness


75% Duty Cycle
    |
    v
High Brightness


100% Duty Cycle
    |
    v
Fully ON
```

Conceptually:

```text
0%:

________________________


25%:

__|‾|___________________


50%:

____|‾‾‾‾|_____________


75%:

______|‾‾‾‾‾‾‾|_______


100%:

‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
```

---

# 10. PWM Period

PWM frequency and period are related:

```text
Period = 1 / Frequency
```

Examples:

```text
100 Hz  → 10 ms period
1 kHz   → 1 ms period
10 kHz  → 100 µs period
```

The appropriate frequency depends on the application.

---

# 11. PWM Frequency Selection

Typical examples:

| Application    | Typical PWM Range           |
| -------------- | --------------------------- |
| LED brightness | Hundreds of Hz to kHz range |
| Buzzer         | Audio-frequency range       |
| DC motor       | Application dependent       |
| Servo          | Commonly around 50 Hz       |
| Power control  | Application dependent       |

These are application examples, not universal requirements. Always use
the peripheral's datasheet/specification.

---

# 12. PWM Device Tree

Project Device Tree files:

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
PWM Consumer
      |
      v
LED / Servo / Motor Driver
```

---

# 13. PWM Pinmux

The physical header pin must be configured for its PWM function.

Conceptually:

```text
Physical Pin
     |
     +---- GPIO
     |
     +---- UART
     |
     +---- I2C
     |
     +---- SPI
     |
     +---- PWM
```

Only one active pin function should be selected for a pin at a time.

Therefore:

```text
Pinmux
   |
   v
PWM Mode
   |
   v
PWM Output
```

---

# 14. PWM Linux Interface

Depending on the Linux kernel version and configuration, PWM may be
exposed through the Linux PWM framework.

On systems using the legacy sysfs PWM interface, you may see:

```bash
ls /sys/class/pwm/
```

Possible output:

```text
pwmchip0
pwmchip1
```

The exact numbering depends on the kernel/device-tree configuration.

---

# 15. Check PWM Controllers

Run:

```bash
ls -l /sys/class/pwm/
```

Then:

```bash
ls -l /sys/class/pwm/pwmchip*/
```

You can inspect the controller:

```bash
cat /sys/class/pwm/pwmchip0/npwm
```

This indicates the number of PWM channels exposed by that controller.

---

# 16. Export PWM Channel

On systems supporting the sysfs PWM interface:

```bash
echo 0 | sudo tee /sys/class/pwm/pwmchip0/export
```

Check:

```bash
ls /sys/class/pwm/pwmchip0/
```

You may see:

```text
pwm0
```

The exact PWM chip/channel must be verified on the target board.

---

# 17. Configure PWM Period

Example:

```bash
echo 1000000 | sudo tee /sys/class/pwm/pwmchip0/pwm0/period
```

The value is normally expressed in nanoseconds.

Therefore:

```text
1000000 ns
=
1 ms
```

which corresponds to:

```text
1 kHz
```

---

# 18. Configure PWM Duty Cycle

For 50% duty cycle:

```bash
echo 500000 | sudo tee /sys/class/pwm/pwmchip0/pwm0/duty_cycle
```

If:

```text
Period = 1000000 ns
Duty   = 500000 ns
```

then:

```text
Duty Cycle = 50%
```

---

# 19. Enable PWM

After configuring the period and duty cycle:

```bash
echo 1 | sudo tee /sys/class/pwm/pwmchip0/pwm0/enable
```

The PWM output should now be active.

Disable:

```bash
echo 0 | sudo tee /sys/class/pwm/pwmchip0/pwm0/enable
```

---

# 20. Complete LED PWM Test

Example sequence:

```bash
echo 0 | sudo tee /sys/class/pwm/pwmchip0/export

echo 1000000 | sudo tee /sys/class/pwm/pwmchip0/pwm0/period

echo 500000 | sudo tee /sys/class/pwm/pwmchip0/pwm0/duty_cycle

echo 1 | sudo tee /sys/class/pwm/pwmchip0/pwm0/enable
```

Expected:

```text
PWM = 1 kHz
Duty = 50%

LED = approximately medium brightness
```

The exact visible brightness depends on the LED, resistor, optics, and
human perception.

---

# 21. Change Duty Cycle

25%:

```bash
echo 250000 | sudo tee /sys/class/pwm/pwmchip0/pwm0/duty_cycle
```

50%:

```bash
echo 500000 | sudo tee /sys/class/pwm/pwmchip0/pwm0/duty_cycle
```

75%:

```bash
echo 750000 | sudo tee /sys/class/pwm/pwmchip0/pwm0/duty_cycle
```

100%:

```bash
echo 1000000 | sudo tee /sys/class/pwm/pwmchip0/pwm0/duty_cycle
```

The duty cycle must not exceed the configured period.

---

# 22. PWM Servo Wiring

A servo generally has:

```text
VCC
GND
SIGNAL
```

Typical connection:

```text
BeagleBone PWM
       |
       v
    SIGNAL
       |
       v
     Servo

Servo GND ---------- BBB GND
Servo VCC ---------- Appropriate external supply
```

For many servos, use an external supply capable of providing the
required current.

Do not assume the BeagleBone 3.3 V rail can power the servo.

---

# 23. Servo Circuit

```text
                  +----------------+
                  |     Servo      |
                  |                |
BBB PWM ----------| SIGNAL         |
                  |                |
BBB GND ----------| GND            |
                  |                |
External Supply --| VCC            |
                  +----------------+
```

Important:

```text
External Supply GND
        |
        +-------- BBB GND
```

The grounds must share an appropriate reference.

---

# 24. Servo PWM Signal

Many hobby servos use a control signal around:

```text
50 Hz
```

which corresponds to approximately:

```text
20 ms period
```

The pulse width determines the commanded position.

However, exact pulse widths and valid ranges vary by servo model.

Always use the servo datasheet.

---

# 25. Servo Example

Conceptually:

```text
20 ms period

|<------------------------------>|

+----+
|    |
|    |
+    +---------------------------
^
|
Control pulse
```

Different pulse widths can command different positions.

Do not assume that a particular pulse width corresponds to a particular
angle for every servo.

---

# 26. PWM Motor Control

Do not connect a DC motor directly to a BeagleBone PWM pin.

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
                BeagleBone
                    |
                  PWM
                    |
                    v
             +-------------+
             | Motor       |
             | Driver      |
             +-------------+
                    |
                    v
                 Motor
                    |
                    v
             External Supply
```

---

# 27. Why a Motor Driver Is Required

A motor can require significantly more current than a processor GPIO
or PWM output can provide.

The motor driver provides:

```text
PWM input interface
Current handling
Voltage handling
Motor switching
Protection features
```

Depending on the driver, additional control signals may include:

```text
PWM
ENABLE
DIR
BRAKE
FAULT
```

---

# 28. PWM Motor Speed Control

Conceptually:

```text
PWM Duty
    |
    +---- 20% ----> Low Speed
    |
    +---- 50% ----> Medium Speed
    |
    +---- 80% ----> High Speed
```

Actual motor speed is affected by:

```text
Motor characteristics
Load
Supply voltage
Driver
PWM frequency
Back EMF
Control algorithm
```

Therefore duty cycle is not necessarily directly proportional to RPM.

---

# 29. PWM + Direction Motor Interface

Typical motor-driver interface:

```text
BBB PWM --------> PWM
BBB GPIO -------> DIR
BBB GPIO -------> ENABLE

Motor Driver
     |
     v
   Motor
```

Conceptual behavior:

```text
DIR = 0
PWM = 50%
     |
     v
Motor rotates direction A


DIR = 1
PWM = 50%
     |
     v
Motor rotates direction B
```

---

# 30. PWM Buzzer

PWM can also generate tones.

```text
BBB PWM
   |
   v
Buzzer
   |
   v
GND
```

The frequency controls the tone:

```text
Frequency ↑
     |
     v
Higher pitch
```

Use an appropriate transistor/driver if the buzzer requires more current
than the PWM output can safely provide.

---

# 31. PWM Measurement

A logic analyzer or oscilloscope can verify:

```text
Frequency
Period
Duty cycle
Rise time
Fall time
Voltage level
```

Connect:

```text
PWM ---- Logic Analyzer CH1
GND ---- Logic Analyzer GND
```

---

# 32. Oscilloscope Verification

Expected waveform:

```text
Voltage

HIGH       +------+       +------+
           |      |       |      |
           |      |       |      |
LOW  ------+      +-------+      +------

           <--- Period --->

           <HIGH>
           <---->
```

Measure:

```text
Period
Frequency
HIGH time
LOW time
Duty cycle
```

---

# 33. PWM Debugging Flow

```text
                  PWM Failure
                      |
                      v
              PWM controller exists?
                 /           \
               NO             YES
               |               |
               v               v
        Check Device Tree   Check PWM pin
        Check Kernel        Check pinmux
                               |
                               v
                       PWM output present?
                          /          \
                        NO            YES
                        |              |
                        v              v
                  Check period      Check load
                  Check duty        Check wiring
                  Check enable      Check voltage
```

---

# 34. PWM Not Available

Check:

```text
[ ] PWM controller enabled
[ ] Device Tree configuration
[ ] Pinmux configuration
[ ] Kernel PWM support
[ ] Correct PWM chip
[ ] Correct PWM channel
[ ] Pin not assigned to another peripheral
```

Check:

```bash
ls /sys/class/pwm/
```

and:

```bash
dmesg | grep -i pwm
```

---

# 35. PWM Pin Has No Output

Check:

```text
[ ] Correct physical pin
[ ] Correct PWM channel
[ ] Pinmux set to PWM
[ ] PWM enabled
[ ] Period configured
[ ] Duty cycle configured
[ ] Ground connected
[ ] Oscilloscope/logic analyzer ground connected
```

---

# 36. LED Does Not Change Brightness

Check:

```text
[ ] LED polarity
[ ] Series resistor
[ ] PWM frequency
[ ] Duty cycle
[ ] PWM output pin
[ ] Pinmux
[ ] Ground
```

Also remember that perceived LED brightness is not perfectly linear
with duty cycle.

---

# 37. Servo Does Not Move

Check:

```text
[ ] Correct PWM pin
[ ] Correct frequency
[ ] Pulse width
[ ] Servo power supply
[ ] Common ground
[ ] Servo signal wiring
[ ] Servo-specific pulse limits
```

Never power a high-current servo from the BeagleBone board without
confirming that the supply is suitable.

---

# 38. PWM Device Tree Overlay

Project overlay:

```text
beaglebone-black/
└── device-tree/
    └── overlays/
        └── bbb-pwm-overlay.dts
```

Conceptual flow:

```text
PWM Overlay
     |
     v
Pinmux
     |
     v
PWM Controller
     |
     v
Linux PWM Framework
     |
     v
PWM Output
```

---

# 39. PWM Test Script

A basic test script can be placed under:

```text
tests/pwm/
```

Example structure:

```text
tests/
└── pwm/
    ├── pwm_led_test.sh
    ├── pwm_servo_test.sh
    └── README.md
```

The script should:

```text
1. Identify PWM controller
2. Select PWM channel
3. Configure period
4. Configure duty cycle
5. Enable PWM
6. Verify output
7. Disable PWM
```

---

# 40. Complete PWM Hardware Test

```text
                       BeagleBone Black
                              |
                         PWM Output
                              |
                              v
                           470 Ω
                              |
                              v
                             LED
                              |
                              v
                             GND
```

Test sequence:

```text
0% duty
   |
   v
LED OFF

25% duty
   |
   v
Low brightness

50% duty
   |
   v
Medium brightness

75% duty
   |
   v
High brightness

100% duty
   |
   v
Fully ON
```

---

# 41. PWM Driver Flow

For a Linux-based PWM application:

```text
User Application
       |
       v
PWM Interface
       |
       v
Linux PWM Framework
       |
       v
PWM Consumer
       |
       v
PWM Controller Driver
       |
       v
AM335x PWM Hardware
       |
       v
PWM Pin
```

---

# 42. PWM Wiring Checklist

```text
[ ] PWM-capable pin selected
[ ] Physical header pin verified
[ ] PWM channel verified
[ ] Pinmux configured
[ ] Device Tree configured
[ ] PWM controller enabled
[ ] LED resistor installed
[ ] LED polarity verified
[ ] GND connected
[ ] Voltage level verified
[ ] Period configured
[ ] Duty cycle configured
[ ] PWM enabled
[ ] Output measured
[ ] Frequency verified
[ ] Duty cycle verified
```

---

# 43. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── pwm_pin_map.md
│   │
│   └── wiring/
│       └── pwm_wiring.md
│
├── hardware/
│   └── schematics/
│       └── pwm/
│           └── pwm_test_circuit.md
│
├── device-tree/
│   ├── pwm/
│   │   ├── bbb-pwm.dts
│   │   ├── bbb-pwm.dtsi
│   │   └── README.md
│   │
│   └── overlays/
│       └── bbb-pwm-overlay.dts
│
├── drivers/
│   └── pwm/
│       └── README.md
│
└── tests/
    └── pwm/
        ├── pwm_led_test.sh
        ├── pwm_servo_test.sh
        └── README.md
```

---

# 44. Complete PWM Bring-Up

```text
                         PWM Application
                               |
                               v
                       Linux PWM Framework
                               |
                               v
                         PWM Controller
                               |
                               v
                          AM335x PWM
                               |
                               v
                         Pin Multiplexer
                               |
                               v
                           PWM Pin
                               |
                +--------------+--------------+
                |              |              |
                v              v              v
               LED           Servo       Motor Driver
                                             |
                                             v
                                           Motor
```

---

# 45. Final Test Objective

The objective of this wiring test is to validate the complete PWM path:

```text
PWM Configuration
      ↓
Device Tree / Pinmux
      ↓
Linux PWM Framework
      ↓
PWM Controller Driver
      ↓
AM335x PWM Hardware
      ↓
PWM Output Pin
      ↓
External Device
```

Recommended validation sequence:

```text
1. Verify PWM pin mapping
2. Verify pinmux
3. Verify Device Tree
4. Check PWM controller
5. Configure period
6. Configure duty cycle
7. Enable PWM
8. Connect LED/load
9. Measure frequency
10. Measure duty cycle
11. Test multiple duty cycles
12. Validate with oscilloscope/logic analyzer
```

> **Important:** Never connect a motor or other high-current load directly
> to a BeagleBone Black PWM/GPIO pin. Use an appropriate driver and
> external power supply. Always verify the electrical limits of the
> selected BeagleBone pin and the requirements of the connected device.

````

**File location:**

```text
beaglebone-black/hardware/wiring/pwm_wiring.md
````

