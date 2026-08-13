Yes. For the `pwm` section, keep exactly the same structure as your ADC, GPIO, I²C, SPI and UART modules.

```text
beaglebone-black/device-tree/pwm/
├── bbb-pwm.dts
├── bbb-pwm.dtsi
└── README.md
```

Below are the complete files.

---

# 1. `bbb-pwm.dts`

```dts
/*
 * BeagleBone Black PWM Device Tree Test
 *
 * Platform : BeagleBone Black
 * SoC      : TI AM335x
 *
 * Purpose:
 *   Enable PWM hardware for testing and driver development.
 */

/dts-v1/;

/include/ "am335x-boneblack.dts"
/include/ "bbb-pwm.dtsi"

/ {
	model = "TI AM335x BeagleBone Black - PWM Test";
	compatible = "ti,am335x-bone-black", "ti,am335x";
};
```

---

# 2. `bbb-pwm.dtsi`

For the first version, enable the AM335x PWM-related hardware through the existing board Device Tree.

```dts
/*
 * BeagleBone Black PWM configuration
 *
 * AM335x PWM functionality is provided through the
 * eHRPWM / ECAP / EHRPWM subsystem depending on the
 * selected peripheral.
 *
 * Exact controller names and pinmux configuration must
 * match the kernel Device Tree being used.
 */

/*
 * Example eHRPWM controller.
 *
 * Verify the exact node names in your kernel Device Tree
 * before using additional PWM instances.
 */

&epwm0 {
	status = "okay";
};

/*
 * Additional PWM controllers can be enabled after verifying
 * their node names in the target kernel Device Tree.
 *
 * Example:
 *
 * &epwm1 {
 *     status = "okay";
 * };
 *
 * &epwm2 {
 *     status = "okay";
 * };
 */
```

### Important

For BeagleBone Black, PWM is closely related to **pin multiplexing**. Simply setting:

```dts
status = "okay";
```

doesn't necessarily make a PWM signal appear on a header pin.

The overall flow is:

```text
PWM Controller
      |
      v
Pin Multiplexer
      |
      v
Physical Header Pin
      |
      v
PWM Signal
```

So we will later add the **exact AM335x pinmux configuration** based on the physical PWM pin you select.

---

# 3. `README.md`

````markdown
# BeagleBone Black PWM Device Tree

## 1. Overview

This directory contains the Device Tree configuration used for PWM
development and testing on the BeagleBone Black.

PWM is commonly used for:

- LED brightness control
- Motor control
- Servo control
- Fan control
- Buzzer generation
- Backlight control
- Power control
- Hardware timing

The PWM section of this project demonstrates the complete Linux PWM
development flow:

```text
Device Tree
     |
     v
PWM Controller
     |
     v
Pin Multiplexing
     |
     v
Linux PWM Subsystem
     |
     v
PWM Driver
     |
     v
Hardware
````

---

# 2. Directory Structure

```text
pwm/
├── bbb-pwm.dts
├── bbb-pwm.dtsi
└── README.md
```

| File           | Purpose                       |
| -------------- | ----------------------------- |
| `bbb-pwm.dts`  | Main PWM Device Tree file     |
| `bbb-pwm.dtsi` | PWM controller configuration  |
| `README.md`    | PWM Device Tree documentation |

---

# 3. Hardware Platform

```text
Board : BeagleBone Black
SoC   : TI AM335x
CPU   : ARM Cortex-A8
```

The AM335x provides hardware PWM functionality through its PWM
subsystems.

Depending on the selected peripheral, PWM functionality can involve
eHRPWM and related PWM hardware blocks.

---

# 4. What Is PWM?

PWM stands for:

```text
Pulse Width Modulation
```

A PWM signal is a periodic digital waveform whose duty cycle can be
controlled.

Example:

```text
HIGH      ┌───────┐       ┌───────┐
          │       │       │       │
LOW  ─────┘       └───────┴───────┘
          <------ Period ------>

          <--- ON --->
          <-------- OFF -------->
```

The two main parameters are:

```text
Period
Duty Cycle
```

---

# 5. PWM Period

The period determines how long one complete PWM cycle takes.

Example:

```text
Frequency = 1 kHz

Period = 1 / 1000
       = 1 ms
```

Therefore:

```text
1 PWM cycle = 1 ms
```

---

# 6. PWM Frequency

PWM frequency is:

```text
Frequency = 1 / Period
```

Example:

```text
Period = 20 ms

Frequency = 1 / 0.020
          = 50 Hz
```

50 Hz is commonly used for servo applications.

---

# 7. PWM Duty Cycle

Duty cycle represents the percentage of time the signal stays HIGH.

Formula:

```text
Duty Cycle (%) =
    HIGH Time / Period × 100
```

Example:

```text
Period = 10 ms
HIGH   = 5 ms

Duty Cycle = 5 / 10 × 100
           = 50%
```

---

# 8. PWM Waveforms

## 25% Duty Cycle

```text
HIGH  ┌───┐           ┌───┐
      │   │           │   │
LOW   ┘   └───────────┘   └────
```

```text
Duty Cycle = 25%
```

---

## 50% Duty Cycle

```text
HIGH  ┌──────┐       ┌──────┐
      │      │       │      │
LOW   ┘      └───────┘      └────
```

```text
Duty Cycle = 50%
```

---

## 75% Duty Cycle

```text
HIGH  ┌───────────┐   ┌───────────┐
      │           │   │           │
LOW   ┘           └───┘           └──
```

```text
Duty Cycle = 75%
```

---

# 9. PWM Architecture

```text
                    USER SPACE
                        |
                        v
                 PWM Test Program
                        |
                        v
                 Linux PWM API
                        |
                        v
                  PWM Subsystem
                        |
                        v
                  PWM Controller
                        |
                        v
                     AM335x
                        |
                        v
                   Pinmux
                        |
                        v
                  Physical Pin
                        |
                        v
              +---------+---------+
              |         |         |
              v         v         v
             LED      Servo      Motor
```

---

# 10. Device Tree Flow

```text
bbb-pwm.dts
      |
      v
bbb-pwm.dtsi
      |
      v
Device Tree Compiler
      |
      v
bbb-pwm.dtb
      |
      v
Bootloader
      |
      v
Linux Kernel
      |
      v
PWM Controller
      |
      v
Linux PWM Subsystem
      |
      v
PWM Consumer / Driver
```

---

# 11. PWM Controller

The AM335x contains hardware blocks capable of generating PWM.

Conceptually:

```text
AM335x
 |
 +-- PWM Controller 0
 |
 +-- PWM Controller 1
 |
 +-- PWM Controller 2
 |
 +-- Additional PWM-related peripherals
```

The exact Device Tree node names depend on the Linux kernel version
and BeagleBone Black Device Tree being used.

---

# 12. Device Tree Configuration

The configuration is located in:

```text
bbb-pwm.dtsi
```

Example:

```dts
&epwm0 {
    status = "okay";
};
```

This enables the corresponding PWM controller node if the label
exists in the base Device Tree.

---

# 13. Pin Multiplexing

AM335x pins are multiplexed.

A physical pin can have multiple possible functions:

```text
                  AM335x PIN
                      |
        +-------------+-------------+
        |             |             |
        v             v             v
       GPIO          UART          PWM
```

For PWM operation, the selected pin must be configured for the
appropriate PWM function.

Therefore:

```text
PWM Controller Enabled
        +
Correct Pinmux
        =
PWM Signal on Header
```

---

# 14. Why Pinmux Is Important

Consider:

```text
PWM Controller
      |
      v
Internal PWM Signal
      |
      X
      |
   Wrong Pinmux
```

The PWM may be running internally but not appear on the physical
header.

Correct configuration:

```text
PWM Controller
      |
      v
Pinmux
      |
      v
Physical Header Pin
      |
      v
PWM Output
```

---

# 15. Inspect Existing Device Tree

Search for PWM:

```bash
grep -R "epwm" arch/arm/boot/dts/
```

Search for ECAP:

```bash
grep -R "ecap" arch/arm/boot/dts/
```

Search pinmux:

```bash
grep -R "pwm" arch/arm/boot/dts/
```

Find BeagleBone Black Device Tree files:

```bash
find arch/arm/boot/dts -iname "*boneblack*"
```

---

# 16. Build Device Tree

Check Device Tree Compiler:

```bash
dtc --version
```

Compile:

```bash
dtc -I dts -O dtb \
    -o bbb-pwm.dtb \
    bbb-pwm.dts
```

Output:

```text
bbb-pwm.dtb
```

---

# 17. Kernel PWM Support

Check the running kernel:

```bash
grep CONFIG_PWM /boot/config-$(uname -r)
```

Depending on the kernel version, relevant options can include:

```text
CONFIG_PWM
CONFIG_PWM_SYSFS
```

The exact configuration options depend on the kernel version.

---

# 18. Linux PWM Subsystem

Linux provides a generic PWM subsystem.

Conceptually:

```text
PWM Hardware
     |
     v
PWM Controller Driver
     |
     v
Linux PWM Core
     |
     v
PWM Consumer Driver
```

This allows applications and drivers to use PWM without directly
programming hardware registers.

---

# 19. PWM Sysfs

Some older Linux kernels expose PWM through sysfs.

Check:

```bash
ls /sys/class/pwm/
```

Possible output:

```text
pwmchip0
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
power
subsystem
uevent
unexport
```

The exact interface depends on the Linux kernel version.

---

# 20. PWM Character Interface

Newer Linux kernel configurations may use PWM through kernel
interfaces rather than relying on the old sysfs interface.

For driver development, prefer the kernel PWM framework and the
appropriate kernel-version APIs.

Do not assume that a particular `/sys/class/pwm` interface exists on
every kernel.

---

# 21. PWM Export

On kernels supporting the legacy PWM sysfs interface, a PWM channel
can be exported.

Example:

```bash
echo 0 | sudo tee /sys/class/pwm/pwmchip0/export
```

Then:

```bash
ls /sys/class/pwm/pwmchip0/
```

You may see:

```text
pwm0
```

---

# 22. Configure PWM Period

Example:

```bash
echo 20000000 | sudo tee \
    /sys/class/pwm/pwmchip0/pwm0/period
```

The value is in nanoseconds.

Therefore:

```text
20,000,000 ns
=
20 ms
=
50 Hz
```

---

# 23. Configure Duty Cycle

For 50% duty cycle:

```bash
echo 10000000 | sudo tee \
    /sys/class/pwm/pwmchip0/pwm0/duty_cycle
```

Because:

```text
Period     = 20 ms
Duty Cycle = 10 ms

10 / 20 × 100
= 50%
```

---

# 24. Enable PWM

Example:

```bash
echo 1 | sudo tee \
    /sys/class/pwm/pwmchip0/pwm0/enable
```

Disable:

```bash
echo 0 | sudo tee \
    /sys/class/pwm/pwmchip0/pwm0/enable
```

Again, these commands apply only when the target kernel exposes the
legacy PWM sysfs interface.

---

# 25. LED Brightness Control

PWM can control LED brightness.

```text
PWM
 |
 v
LED
```

Example:

```text
Duty Cycle
    |
    +---- 10% -> Dim
    |
    +---- 50% -> Medium
    |
    +---- 90% -> Bright
```

Conceptually:

```text
PWM Duty Cycle ↑
       |
       v
Average LED Power ↑
       |
       v
Brightness ↑
```

---

# 26. Servo Motor Control

A typical hobby servo uses approximately:

```text
Frequency ≈ 50 Hz
Period    ≈ 20 ms
```

The pulse width determines the position.

Conceptually:

```text
1 ms  -> Position 0°
1.5ms -> Position 90°
2 ms  -> Position 180°
```

The exact relationship depends on the servo.

---

# 27. Servo PWM Flow

```text
User Application
      |
      v
PWM Configuration
      |
      v
Period = 20 ms
      |
      v
Duty/Pulse Width
      |
      v
PWM Controller
      |
      v
Servo
```

---

# 28. Motor Control

PWM is commonly used for DC motor speed control.

```text
PWM
 |
 v
Motor Driver
 |
 v
DC Motor
```

Duty cycle controls the average applied power.

Example:

```text
20% -> Low speed
50% -> Medium speed
80% -> High speed
```

The actual motor speed depends on the motor, load, driver,
supply voltage and control system.

---

# 29. PWM and Motor Driver

Do not connect a motor directly to a BeagleBone GPIO/PWM pin.

Correct architecture:

```text
BeagleBone
    |
    | PWM
    v
Motor Driver
    |
    v
Motor
```

The motor driver provides the required current and voltage.

---

# 30. PWM Driver Architecture

A Linux PWM driver typically follows:

```text
Device Tree
     |
     v
PWM Controller
     |
     v
Driver Probe
     |
     v
PWM Chip Registration
     |
     v
Linux PWM Core
     |
     v
PWM Consumer
```

---

# 31. Driver Probe Flow

```text
Linux Boot
    |
    v
Device Tree Parsing
    |
    v
PWM Device Created
    |
    v
Driver Matching
    |
    v
probe()
    |
    v
PWM Hardware Initialization
    |
    v
PWM Chip Registration
```

---

# 32. PWM Consumer Driver

A device that needs PWM can request a PWM channel.

Conceptually:

```c
struct pwm_device *pwm;

pwm = devm_pwm_get(dev, NULL);
```

The exact API and behavior depend on the Linux kernel version.

---

# 33. PWM Configuration

Conceptually, a PWM configuration contains:

```text
Period
Duty Cycle
Polarity
Enable
```

Example:

```text
Period     = 20 ms
Duty Cycle = 1.5 ms
Polarity   = Normal
Enabled    = Yes
```

---

# 34. PWM Driver Flow

```text
Application
     |
     v
Consumer Driver
     |
     v
PWM API
     |
     v
Linux PWM Core
     |
     v
PWM Controller Driver
     |
     v
AM335x PWM Hardware
     |
     v
Physical Pin
```

---

# 35. PWM Testing

First verify PWM support:

```bash
ls /sys/class/pwm/
```

Then:

```bash
ls /sys/class/pwm/pwmchip0/
```

Check available channels:

```bash
cat /sys/class/pwm/pwmchip0/npwm
```

The actual `pwmchip` number may be different.

---

# 36. PWM Test Using Oscilloscope

The best way to verify PWM is with an oscilloscope or logic analyzer.

Connect:

```text
BeagleBone PWM
       |
       v
Oscilloscope CH1

BeagleBone GND
       |
       v
Oscilloscope GND
```

Measure:

```text
Frequency
Period
Duty Cycle
Voltage
```

---

# 37. Expected PWM Test

Configure:

```text
Frequency = 1 kHz
Duty Cycle = 50%
```

Expected:

```text
Period = 1 ms
HIGH    ≈ 0.5 ms
LOW     ≈ 0.5 ms
```

Oscilloscope:

```text
HIGH   ┌─────┐     ┌─────┐
       │     │     │     │
LOW ───┘     └─────┘     └─────
       <--- 1 ms --->
```

---

# 38. PWM Debugging

Check kernel messages:

```bash
dmesg | grep -i pwm
```

Check PWM devices:

```bash
ls /sys/class/pwm/
```

Check:

```bash
cat /sys/class/pwm/pwmchip0/npwm
```

Check Device Tree:

```bash
find /proc/device-tree -iname "*pwm*"
```

---

# 39. Device Tree Debugging

Check live Device Tree:

```bash
ls /proc/device-tree/
```

Search PWM:

```bash
find /proc/device-tree -iname "*pwm*"
```

Check kernel logs:

```bash
dmesg | grep -Ei "pwm|epwm|ecap"
```

---

# 40. Common Problem: PWM Not Visible

If no PWM appears:

```text
Check
 |
 +-- Device Tree
 |
 +-- PWM controller
 |
 +-- Pinmux
 |
 +-- Kernel PWM support
 |
 +-- PWM driver
 |
 +-- Physical pin
```

---

# 41. Common Problem: PWM Controller Enabled But No Output

This usually means the PWM controller is configured but the physical
pin is not correctly configured.

Check:

```text
PWM Controller
       |
       v
Pinmux
       |
       v
Header Pin
```

Both controller configuration and pinmux are required.

---

# 42. Common Problem: Wrong Frequency

Verify:

```text
Clock Source
PWM Period
Prescaler
Counter
```

The PWM frequency is derived from the hardware PWM clock and timing
configuration.

---

# 43. Common Problem: Wrong Duty Cycle

Check:

```text
Period
Duty Cycle
Polarity
```

For example:

```text
Period     = 20 ms
Duty Cycle = 10 ms

Duty = 50%
```

---

# 44. PWM Interrupts

Some PWM hardware can also generate interrupts.

Conceptual flow:

```text
PWM Controller
      |
      v
PWM Event
      |
      v
IRQ
      |
      v
Linux IRQ Subsystem
      |
      v
Interrupt Handler
```

This can be useful for timing and event-driven hardware control.

---

# 45. PWM Applications

This project can demonstrate PWM with:

```text
+----------------------+
| PWM Applications     |
+----------------------+
| LED Brightness       |
| Servo Motor          |
| DC Motor             |
| Fan Control          |
| Buzzer               |
| Backlight            |
| Power Control        |
+----------------------+
```

---

# 46. PWM Testing Matrix

| Test           | Description          | Status  |
| -------------- | -------------------- | ------- |
| PWM Controller | Verify controller    | Planned |
| Device Tree    | Verify PWM node      | Planned |
| Pinmux         | Verify PWM pin       | Planned |
| PWM Channel    | Verify channel       | Planned |
| Frequency      | Verify frequency     | Planned |
| Period         | Verify period        | Planned |
| Duty Cycle     | Verify duty cycle    | Planned |
| Enable         | Enable PWM output    | Planned |
| Disable        | Disable PWM output   | Planned |
| LED            | Brightness control   | Planned |
| Servo          | Position control     | Planned |
| Motor          | Speed control        | Planned |
| Oscilloscope   | Signal verification  | Planned |
| Stress Test    | Continuous operation | Planned |

---

# 47. User-Space PWM Test

Recommended test structure:

```text
user-space/
└── pwm_test/
    ├── Makefile
    ├── README.md
    └── pwm_test.c
```

The application can test:

```text
Set Period
Set Duty Cycle
Enable
Disable
```

---

# 48. PWM Driver Structure

The kernel driver can eventually be:

```text
drivers/
└── 05_pwm/
    ├── Makefile
    ├── README.md
    └── bbb_pwm_driver.c
```

The driver should demonstrate:

* Device Tree matching
* `probe()`
* `remove()`
* PWM framework
* PWM channel management
* Period configuration
* Duty-cycle configuration
* Enable/disable
* Error handling
* Kernel logging

---

# 49. Driver Integration

The PWM Device Tree connects to the driver:

```text
device-tree/pwm/
        |
        v
bbb-pwm.dts
        |
        v
bbb-pwm.dtsi
        |
        v
Linux Device Tree
        |
        v
PWM Controller
        |
        v
drivers/05_pwm/
        |
        v
PWM Driver
        |
        v
user-space/pwm_test/
        |
        v
Hardware Test
```

---

# 50. Complete PWM Flow

```text
                    BEAGLEBONE BLACK
                           |
                           v
                         AM335x
                           |
                           v
                    PWM Controller
                           |
                           v
                      Pinmux Setup
                           |
                           v
                    Physical PWM Pin
                           |
                           v
                    Linux PWM Core
                           |
                           v
                     PWM Consumer
                           |
              +------------+------------+
              |            |            |
              v            v            v
             LED          Servo        Motor
```

---

# 51. Development Checklist

* [ ] Identify PWM controller
* [ ] Identify PWM channel
* [ ] Identify physical header pin
* [ ] Verify pinmux
* [ ] Create `bbb-pwm.dtsi`
* [ ] Create `bbb-pwm.dts`
* [ ] Build Device Tree
* [ ] Deploy DTB
* [ ] Boot board
* [ ] Verify PWM controller
* [ ] Verify PWM channel
* [ ] Configure period
* [ ] Configure duty cycle
* [ ] Enable PWM
* [ ] Measure with oscilloscope
* [ ] Test LED brightness
* [ ] Test servo
* [ ] Test motor through driver
* [ ] Develop PWM driver
* [ ] Add user-space test
* [ ] Perform stress testing
* [ ] Document results

---

# 52. Repository Integration

The PWM module is part of the complete BeagleBone Black Linux driver
project:

```text
beaglebone-black/
|
+-- device-tree/
|   |
|   +-- adc/
|   +-- can/
|   +-- gpio/
|   +-- i2c/
|   +-- overlays/
|   +-- pwm/
|   +-- spi/
|   +-- uart/
|
+-- drivers/
|   |
|   +-- 01_char_driver/
|   +-- 02_gpio/
|   +-- 03_interrupt/
|   +-- 04_i2c/
|   +-- 05_pwm/
|   +-- ...
|
+-- user-space/
|   |
|   +-- gpio_test/
|   +-- i2c_test/
|   +-- pwm_test/
|   +-- ...
|
+-- tests/
|
+-- docs/
|
+-- scripts/
```

---

# 53. Final Architecture

```text
                         USER SPACE
                             |
                             v
                       PWM Test App
                             |
                             v
                      PWM Consumer
                             |
                             v
                       Linux PWM Core
                             |
                             v
                    PWM Controller Driver
                             |
                             v
                         AM335x
                             |
                             v
                         Pinmux
                             |
                             v
                       PWM Output
                             |
             +---------------+---------------+
             |               |               |
             v               v               v
            LED            Servo           Motor
```

---

# 54. Status

```text
Device Tree Configuration : In Development
PWM Controller             : In Development
Pinmux Configuration       : To Be Verified
PWM Channel                : Planned
PWM Frequency              : Planned
PWM Duty Cycle             : Planned
PWM Output                 : Planned
LED Test                   : Planned
Servo Test                 : Planned
Motor Test                 : Planned
Driver                     : In Development
User-Space Test            : Planned
Oscilloscope Validation    : Planned
Stress Test                : Planned
Documentation              : In Progress
```

---

# 55. Summary

This PWM module demonstrates the complete Linux PWM development flow:

```text
AM335x PWM Hardware
        ↓
PWM Controller
        ↓
Pin Multiplexing
        ↓
Device Tree
        ↓
Linux PWM Subsystem
        ↓
PWM Driver
        ↓
PWM Consumer
        ↓
LED / Servo / Motor
        ↓
Oscilloscope Validation
```

The PWM implementation will later connect to:

```text
device-tree/pwm/
        ↓
drivers/05_pwm/
        ↓
user-space/pwm_test/
        ↓
tests/
```

````

---

## Final directory

After adding the three files:

```text
beaglebone-black/device-tree/pwm/
├── bbb-pwm.dts
├── bbb-pwm.dtsi
└── README.md
````

You can check:

```bash
cd ~/beaglebone-black/device-tree/pwm
ls -lh
```

### One important point for this project

For your **complete BeagleBone Black driver GitHub project**, PWM should eventually demonstrate **three real use cases**:

```text
PWM
├── LED brightness
├── Servo control
└── Motor speed control
```

That gives you a strong interview explanation:

> **“I configured the AM335x PWM controller through Device Tree, handled pin multiplexing, integrated it with the Linux PWM framework, and validated period, frequency and duty-cycle control using LED, servo and motor applications.”**

