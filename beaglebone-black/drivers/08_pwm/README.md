# BeagleBone Black PWM Driver

## Overview

This directory contains an example Linux kernel PWM driver for the
BeagleBone Black.

The driver uses the Linux **PWM framework** instead of directly
programming PWM registers.

The driver demonstrates:

- Platform driver registration
- Device Tree matching
- Linux PWM framework
- PWM period configuration
- PWM duty-cycle configuration
- PWM enable/disable
- Sysfs attributes
- Mutex protection
- Kernel module build and loading

---

## Directory Structure

```text
08_pwm/
├── pwm_driver.c
├── pwm_driver.h
├── Makefile
└── README.md
Driver Architecture
                    Device Tree
                         |
                         v
                  PWM Platform Device
                         |
                         v
                    pwm_probe()
                         |
                         v
                  Linux PWM Core
                         |
                         v
                  PWM Controller
                         |
                  +------+------+
                  |             |
                  v             v
              Period        Duty Cycle
                  |             |
                  +------+------+
                         |
                         v
                    PWM Output
                         |
                         v
                    GPIO/PWM Pin
PWM Concepts
Period

The PWM period defines the total time for one complete PWM cycle.

Frequency = 1 / Period

For example:

Period = 1 ms
Frequency = 1 kHz
Duty Cycle

The duty cycle determines how long the PWM output remains HIGH
during one period.

For example:

Period     = 1 ms
Duty       = 500 us

Duty Cycle = 50%
PWM Waveform
50% Duty Cycle

HIGH  ┌───────┐       ┌───────┐
      │       │       │       │
      │       │       │       │
LOW   ┘       └───────┘       └───────

      <------ Period ------>

      <--- Duty --->
Device Tree

The example driver expects:

compatible = "bbb,pwm-test";

Example:

pwm_test {
    compatible = "bbb,pwm-test";
    status = "okay";
};

The actual PWM controller and pin configuration must be provided
according to the target BeagleBone Black Device Tree.

Build

Run:

make

Expected output:

pwm_driver.ko

Verify:

ls -l pwm_driver.ko
Load Driver
sudo insmod pwm_driver.ko

Check:

lsmod | grep pwm_driver

Check logs:

dmesg | grep -i pwm
Unload Driver
sudo rmmod pwm_driver
PWM Sysfs Interface

The driver creates attributes through the device associated with
the platform driver:

period
duty_cycle
enable

The values are represented in nanoseconds.

Example:

period = 20000000 ns
duty_cycle = 10000000 ns

This represents:

Period    = 20 ms
Duty      = 10 ms
Duty      = 50%
Frequency = 50 Hz
Configure PWM

Set period:

echo 1000000 > period

Set duty cycle:

echo 500000 > duty_cycle

Enable:

echo 1 > enable

Disable:

echo 0 > enable
PWM Flow
User Space
    |
    v
Sysfs
    |
    +---- period
    |
    +---- duty_cycle
    |
    +---- enable
    |
    v
pwm_driver.c
    |
    v
Linux PWM Framework
    |
    v
PWM Controller Driver
    |
    v
Hardware PWM
    |
    v
PWM Pin
Example: 1 kHz / 50%

Period:

1 ms = 1,000,000 ns

Duty cycle:

500 us = 500,000 ns

Configuration:

echo 1000000 > period
echo 500000 > duty_cycle
echo 1 > enable

Result:

Frequency = 1 kHz
Duty Cycle = 50%
Example: 1 kHz / 25%
echo 1000000 > period
echo 250000 > duty_cycle
echo 1 > enable

Result:

Frequency = 1 kHz
Duty Cycle = 25%
Debugging

Check PWM framework:

ls /sys/class/pwm/

Check loaded module:

lsmod | grep pwm_driver

Check kernel messages:

dmesg | grep -i pwm

Check platform devices:

ls /sys/bus/platform/devices/

Check driver:

ls /sys/bus/platform/drivers/
Makefile Commands

Build:

make

Clean:

make clean

Load:

make load

Unload:

make unload

Status:

make status

Information:

make info

Test:

make test

Logs:

make logs
Important Note

The BeagleBone Black already has Linux PWM controller support.

For a production driver, the preferred architecture is normally:

Application
     |
     v
PWM Sysfs / PWM Consumer
     |
     v
Linux PWM Framework
     |
     v
AM335x PWM Driver
     |
     v
eHRPWM / EHRPWM Hardware
     |
     v
PWM Pin

The register-level implementation should not duplicate an existing
kernel PWM controller driver.

The Device Tree pinmux, PWM controller, channel, clock and output pin
must match the actual BeagleBone Black hardware configuration.


### Final structure

```text
beaglebone-black/
└── drivers/
    └── 08_pwm/
        ├── pwm_driver.c
        ├── pwm_driver.h
        ├── Makefile
        └── README.md

This follows the same structure as your 05_spi, 06_uart, and 07_adc driver directories.
