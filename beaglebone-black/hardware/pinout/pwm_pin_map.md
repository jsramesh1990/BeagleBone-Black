# BeagleBone Black PWM Pin Map

## 1. Overview

The **BeagleBone Black** uses the TI AM335x processor, which provides
hardware PWM through the **eHRPWM** and **eCAP** peripherals.

PWM can be used for:

* LED brightness control
* Motor speed control
* Servo control
* Fan control
* Buzzer/tone generation
* Power regulation
* Backlight control
* Hardware timing

The basic architecture is:

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
AM335x PWM Hardware
       |
       v
PWM Pin
       |
       v
External Device
```

---

# 2. PWM Architecture

The AM335x provides PWM functionality through peripherals such as:

```text
eHRPWM
eCAP
```

The **eHRPWM** peripheral provides two PWM outputs:

```text
eHRPWMxA
eHRPWMxB
```

Conceptually:

```text
              AM335x
                |
       +--------+--------+
       |                 |
    eHRPWM0           eHRPWM1
       |                 |
    +--+--+           +--+--+
    |     |           |     |
   A      B          A      B
```

---

# 3. Common BeagleBone Black PWM Pins

Common PWM-capable expansion-header pins include:

| Header Pin | PWM Function | Peripheral |
| ---------- | ------------ | ---------- |
| P8.13      | EHRPWM2B     | eHRPWM2    |
| P8.19      | EHRPWM2A     | eHRPWM2    |
| P9.14      | EHRPWM1A     | eHRPWM1    |
| P9.16      | EHRPWM1B     | eHRPWM1    |
| P9.21      | EHRPWM0B     | eHRPWM0    |
| P9.22      | EHRPWM0A     | eHRPWM0    |
| P9.29      | EHRPWM0B     | eHRPWM0    |
| P9.31      | EHRPWM0A     | eHRPWM0    |
| P8.34      | EHRPWM1A     | eHRPWM1    |
| P8.36      | EHRPWM1B     | eHRPWM1    |

> **Important:** PWM pins are multiplexed with other peripheral
> functions. The actual pinmux configuration in the running Device Tree
> must be checked before using a pin.

---

# 4. Recommended PWM Pins

For a simple PWM project, commonly used header pins include:

```text
P9.14 → eHRPWM1A
P9.16 → eHRPWM1B
P9.21 → eHRPWM0B
P9.22 → eHRPWM0A
```

Quick reference:

```text
+-------------+-------------+
| BBB Pin     | PWM Signal  |
+-------------+-------------+
| P9.14       | EHRPWM1A    |
| P9.16       | EHRPWM1B    |
| P9.21       | EHRPWM0B    |
| P9.22       | EHRPWM0A    |
+-------------+-------------+
```

---

# 5. PWM Pin Concept

Example:

```text
P9.14
  |
  v
EHRPWM1A
  |
  v
PWM Hardware
  |
  v
Linux PWM Framework
  |
  v
Application
```

The PWM signal looks like:

```text
HIGH ────┐      ┌───────┐      ┌───────
         │      │       │      │
LOW      └──────┘       └──────┘
         <---- Period ---->
```

---

# 6. PWM Frequency

PWM frequency is determined by the period.

Formula:

```text
Frequency = 1 / Period
```

For example:

```text
Period = 1 ms

Frequency = 1 / 0.001
          = 1000 Hz
```

Therefore:

```text
1 ms period → 1 kHz PWM
```

---

# 7. PWM Duty Cycle

Duty cycle determines how long the signal remains HIGH during one
period.

Formula:

```text
Duty Cycle (%) =
        (ON Time / Total Period) × 100
```

Example:

```text
Period = 1 ms
ON Time = 0.5 ms

Duty Cycle = 50%
```

Waveform:

```text
50% Duty Cycle

HIGH ────┐      ┌───────
         │      │
LOW      └──────┘
         50%  50%
```

---

# 8. PWM Examples

### 25% Duty Cycle

```text
HIGH ──┐          ┌────────
       │          │
LOW    └──────────┘
       25%        75%
```

### 50% Duty Cycle

```text
HIGH ─────┐      ┌──────
          │      │
LOW       └──────┘
          50%
```

### 75% Duty Cycle

```text
HIGH ─────────┐  ┌────────
              │  │
LOW           └──┘
              75%
```

---

# 9. PWM LED Brightness Control

PWM can control LED brightness.

```text
              BeagleBone Black

PWM P9.14
    |
    |
   330Ω
    |
   LED
    |
   GND
```

Changing duty cycle:

```text
10%  → Dim
25%  → Low brightness
50%  → Medium brightness
75%  → High brightness
100% → Maximum
```

---

# 10. PWM Motor Control

PWM can control a motor through an appropriate motor driver.

**Do not connect a motor directly to a BeagleBone GPIO/PWM pin.**

```text
BeagleBone
    |
 PWM Signal
    |
    v
+-------------+
| Motor       |
| Driver      |
+-------------+
    |
    v
  Motor
```

Architecture:

```text
PWM
 |
 v
Motor Driver
 |
 v
Motor
```

Duty cycle controls the effective motor power/speed.

---

# 11. PWM Servo Control

A servo can be controlled using PWM pulses.

Typical servo control:

```text
PWM Frequency ≈ 50 Hz
Period ≈ 20 ms
```

Pulse width determines the servo position.

Conceptually:

```text
20 ms period

|<-------------------- 20 ms -------------------->|

┌──────┐
│      │
┘      └───────────────────────────────────────────
 1 ms          → Position 1

┌──────────┐
│          │
┘          └───────────────────────────────────────
 1.5 ms        → Center

┌──────────────┐
│              │
┘              └───────────────────────────────────
 2 ms           → Position 3
```

Exact pulse widths depend on the servo.

---

# 12. PWM Device Tree

PWM peripherals are enabled through Device Tree.

Conceptually:

```dts
&ehrpwm1 {
    status = "okay";
};
```

A device consuming PWM may reference it using:

```dts
pwm = <&ehrpwm1 0 1000000 0>;
```

The exact binding depends on the consuming device and kernel version.

Project files:

```text
beaglebone-black/
└── device-tree/
    └── pwm/
        ├── bbb-pwm.dts
        ├── bbb-pwm.dtsi
        └── README.md
```

---

# 13. PWM Pinmux

The physical pin must be configured for the required PWM function.

Example:

```text
P9.14
  |
  v
Pinmux
  |
  v
EHRPWM1A
  |
  v
PWM Controller
```

Conceptually:

```text
P9.14
  |
  +---- GPIO
  |
  +---- EHRPWM1A
  |
  +---- Alternate Function
```

Device Tree selects the required function.

---

# 14. PWM Linux Framework

Linux provides a generic PWM framework.

Architecture:

```text
User Application
       |
       v
PWM Consumer
       |
       v
Linux PWM Framework
       |
       v
AM335x PWM Driver
       |
       v
eHRPWM Hardware
       |
       v
Physical Pin
```

---

# 15. PWM Sysfs Interface

Depending on the kernel version and configuration, PWM may be exposed
through the legacy sysfs interface.

Check:

```bash
ls /sys/class/pwm/
```

Possible output:

```text
pwmchip0
pwmchip1
pwmchip2
```

Inspect:

```bash
ls /sys/class/pwm/pwmchip0/
```

Possible entries:

```text
device
export
npwm
period
polarity
power
uevent
unexport
```

> Modern Linux systems increasingly prefer newer kernel interfaces and
> Device Tree based PWM consumers. Sysfs PWM is still useful for
> understanding and testing older BSPs.

---

# 16. Export PWM

On systems where the PWM sysfs interface is enabled:

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

---

# 17. Configure PWM Period

Example:

```bash
echo 1000000 | sudo tee \
/sys/class/pwm/pwmchip0/pwm0/period
```

The value is in nanoseconds.

```text
1,000,000 ns
= 1 ms
= 1 kHz
```

---

# 18. Configure PWM Duty Cycle

For 50% duty cycle:

```bash
echo 500000 | sudo tee \
/sys/class/pwm/pwmchip0/pwm0/duty_cycle
```

Because:

```text
Period     = 1,000,000 ns
Duty cycle =   500,000 ns

Duty = 50%
```

---

# 19. Enable PWM

```bash
echo 1 | sudo tee \
/sys/class/pwm/pwmchip0/pwm0/enable
```

Disable:

```bash
echo 0 | sudo tee \
/sys/class/pwm/pwmchip0/pwm0/enable
```

---

# 20. PWM Test Example

Complete example using sysfs:

```bash
PWM=/sys/class/pwm/pwmchip0/pwm0

echo 0 | sudo tee /sys/class/pwm/pwmchip0/export

echo 1000000 | sudo tee $PWM/period

echo 500000 | sudo tee $PWM/duty_cycle

echo 1 | sudo tee $PWM/enable
```

This configures approximately:

```text
Frequency : 1 kHz
Duty Cycle: 50%
```

---

# 21. PWM Linux Commands

Check PWM controllers:

```bash
ls /sys/class/pwm/
```

Check number of PWM channels:

```bash
cat /sys/class/pwm/pwmchip0/npwm
```

Check current period:

```bash
cat /sys/class/pwm/pwmchip0/pwm0/period
```

Check duty cycle:

```bash
cat /sys/class/pwm/pwmchip0/pwm0/duty_cycle
```

Check enable status:

```bash
cat /sys/class/pwm/pwmchip0/pwm0/enable
```

---

# 22. PWM Debugging

Check kernel messages:

```bash
dmesg | grep -i pwm
```

Check pinmux:

```bash
sudo cat /sys/kernel/debug/pinctrl/*/pinmux-pins
```

Search PWM:

```bash
sudo cat /sys/kernel/debug/pinctrl/*/pinmux-pins | grep -i pwm
```

Check PWM devices:

```bash
find /sys/class/pwm/ -maxdepth 2 -type d
```

---

# 23. PWM Measurement

A logic analyzer or oscilloscope should be used to verify:

```text
1. Frequency
2. Duty cycle
3. Voltage level
4. Rising edge
5. Falling edge
```

Example:

```text
Expected:

Frequency = 1 kHz
Period    = 1 ms
Duty      = 50%
```

Oscilloscope:

```text
     ┌───────┐       ┌───────┐
     │       │       │       │
─────┘       └───────┘       └──────
     <---1 ms--->
```

---

# 24. PWM Driver Flow

```text
                 Linux Boot
                     |
                     v
                Device Tree
                     |
                     v
                 Pinmux
                     |
                     v
              PWM Controller
                     |
                     v
                PWM Driver
                     |
                     v
              Linux PWM Core
                     |
                     v
              PWM Consumer
                     |
                     v
              Physical Output
```

---

# 25. PWM Driver Responsibilities

The PWM driver controls:

```text
1. PWM period
2. PWM duty cycle
3. PWM polarity
4. PWM enable/disable
5. Clock configuration
6. Hardware registers
7. PWM channel selection
```

Typical kernel PWM concepts include:

```c
pwm_get()
pwm_apply_state()
pwm_enable()
pwm_disable()
```

Modern kernel drivers generally use the PWM state/configuration APIs.

---

# 26. PWM Register-Level Concept

The eHRPWM hardware contains registers for controlling the waveform.

Conceptually:

```text
             eHRPWM
        +---------------+
        | Clock         |
        |               |
        | Time Base     |
        |               |
        | Counter       |
        |               |
        | Compare       |
        |               |
        | Action        |
        | Qualifier     |
        +-------+-------+
                |
                v
             PWM OUT
```

The timer counter repeatedly counts through the configured period.

The compare value determines when the output changes state.

---

# 27. PWM Frequency and Counter

Conceptually:

```text
PWM Clock
    |
    v
Time Base Counter
    |
    +---- Period Register
    |
    +---- Compare Register
    |
    v
PWM Output
```

Example:

```text
Period Register → 1000 counts
Compare Register → 500 counts

Duty Cycle ≈ 50%
```

---

# 28. Multiple PWM Channels

PWM channels can be used independently.

Example:

```text
eHRPWM1
   |
   +---- eHRPWM1A → LED 1
   |
   +---- eHRPWM1B → LED 2
```

Or:

```text
eHRPWM0
   |
   +---- Channel A → Motor 1
   |
   +---- Channel B → Motor 2
```

The actual synchronization and configuration behavior depends on the
specific eHRPWM hardware configuration.

---

# 29. PWM and GPIO Conflict

A PWM-capable pin cannot normally be used simultaneously as a GPIO and
PWM output.

Example:

```text
P9.14
   |
   +---- GPIO
   |
   +---- EHRPWM1A
   |
   +---- Alternate Function
```

If Device Tree selects:

```text
EHRPWM1A
```

the pin is controlled by the PWM peripheral rather than the normal GPIO
function.

---

# 30. PWM Overlay

For projects using Device Tree overlays:

```text
beaglebone-black/
└── device-tree/
    └── overlays/
        ├── bbb-gpio-overlay.dts
        ├── bbb-i2c-overlay.dts
        ├── bbb-spi-overlay.dts
        ├── bbb-uart-overlay.dts
        └── README.md
```

A PWM overlay can also be maintained if the BSP/bootloader configuration
supports Device Tree overlays.

Example project addition:

```text
bbb-pwm-overlay.dts
```

---

# 31. PWM Testing with LED

Recommended test:

```text
                  BeagleBone Black

PWM P9.14
    |
    |
   330Ω
    |
   LED
    |
   GND
```

Test:

```text
10% duty → LED dim
25% duty → LED brightness increases
50% duty → Medium brightness
75% duty → High brightness
90% duty → Very bright
```

---

# 32. PWM Testing with Oscilloscope

Connect:

```text
Oscilloscope CH1 → PWM pin
Oscilloscope GND → BBB GND
```

Configure:

```text
Period = 1 ms
Duty   = 50%
```

Expected:

```text
Frequency ≈ 1 kHz
Duty      ≈ 50%
```

---

# 33. PWM Debug Flow

```text
                 PWM NOT WORKING
                       |
                       v
                Check pwmchip
                       |
                       v
              Check PWM channel
                       |
                       v
               Check Device Tree
                       |
                       v
                 Check Pinmux
                       |
                       v
                Check PWM Driver
                       |
                       v
               Check Period/Duty
                       |
                       v
              Check Enable State
                       |
                       v
           Measure Physical Signal
                       |
                       v
                 Oscilloscope
```

---

# 34. Common PWM Problems

### PWM controller not visible

Check:

```bash
ls /sys/class/pwm/
```

Then:

```bash
dmesg | grep -i pwm
```

Possible causes:

```text
1. PWM node disabled
2. Driver not loaded
3. Incorrect Device Tree
4. Kernel configuration missing
```

---

### PWM pin has no signal

Check:

```text
1. Correct PWM channel
2. Correct pin
3. Pinmux
4. Device Tree
5. Period
6. Duty cycle
7. PWM enable
8. Hardware connection
```

---

### LED brightness does not change

Check:

```text
1. LED polarity
2. Resistor
3. PWM frequency
4. Duty cycle
5. Pinmux
6. Ground
7. Physical PWM waveform
```

---

# 35. PWM Testing Checklist

```text
[ ] PWM controller enabled
[ ] PWM driver loaded
[ ] PWM chip detected
[ ] PWM channel detected
[ ] Correct header pin selected
[ ] Pinmux configured
[ ] Device Tree configured
[ ] Period configured
[ ] Duty cycle configured
[ ] PWM enabled
[ ] LED test completed
[ ] Oscilloscope test completed
[ ] Frequency verified
[ ] Duty cycle verified
[ ] PWM polarity verified
[ ] Kernel logs checked
```

---

# 36. PWM Pin Quick Reference

```text
+-------------+-------------+-------------+
| BBB Pin     | PWM Signal  | Peripheral  |
+-------------+-------------+-------------+
| P8.13       | EHRPWM2B    | eHRPWM2     |
| P8.19       | EHRPWM2A    | eHRPWM2     |
| P9.14       | EHRPWM1A    | eHRPWM1     |
| P9.16       | EHRPWM1B    | eHRPWM1     |
| P9.21       | EHRPWM0B    | eHRPWM0     |
| P9.22       | EHRPWM0A    | eHRPWM0     |
+-------------+-------------+-------------+
```

---

# 37. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   └── pinout/
│       └── pwm_pin_map.md
│
├── device-tree/
│   └── pwm/
│       ├── bbb-pwm.dts
│       ├── bbb-pwm.dtsi
│       └── README.md
│
├── drivers/
│   └── pwm/
│       ├── README.md
│       └── ...
│
└── tests/
    └── pwm/
        ├── pwm_test.sh
        ├── pwm_led_test.sh
        └── pwm_servo_test.sh
```

---

# 38. Complete PWM Architecture

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
                     AM335x PWM HW
                            |
              +-------------+-------------+
              |                           |
              v                           v
         eHRPWM0                       eHRPWM1
          A / B                         A / B
              |                           |
              +-------------+-------------+
                            |
                            v
                     Linux PWM Framework
                            |
              +-------------+-------------+
              |             |             |
              v             v             v
             LED          Motor         Servo
```

## Project File

```text
beaglebone-black/hardware/pinout/pwm_pin_map.md
```

