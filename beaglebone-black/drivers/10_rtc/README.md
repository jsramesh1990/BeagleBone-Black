# BeagleBone Black RTC Driver

## Overview

This directory contains an example Linux kernel RTC driver for the
BeagleBone Black.

The driver uses the Linux **RTC (Real-Time Clock) framework**.

Features:

- Platform driver registration
- Device Tree matching
- RTC register mapping
- RTC time read
- RTC time set
- RTC alarm read
- RTC alarm configuration
- RTC alarm enable/disable
- Linux RTC framework integration
- Character device `/dev/rtcX`

---

## Directory Structure

```text
10_rtc/
├── rtc_driver.c
├── rtc_driver.h
├── Makefile
└── README.md
RTC Architecture
                    Device Tree
                         |
                         v
                  RTC Platform Device
                         |
                         v
                    rtc_probe()
                         |
                         v
                    RTC Registers
                         |
                         v
                 Linux RTC Framework
                         |
                         v
                    /dev/rtcX
                         |
             +-----------+-----------+
             |                       |
             v                       v
         Read Time               Set Time
             |                       |
             +-----------+-----------+
                         |
                         v
                    RTC Hardware
RTC Read Flow
User Space
    |
    v
/dev/rtc0
    |
    v
Linux RTC Framework
    |
    v
read_time()
    |
    v
RTC Registers
    |
    v
Seconds / Minutes / Hours
    |
    v
RTC Hardware
RTC Set-Time Flow
User Space
    |
    v
hwclock --set
    |
    v
RTC Framework
    |
    v
set_time()
    |
    v
RTC Registers
    |
    v
Hardware RTC
Device Tree

The example driver expects:

compatible = "bbb,rtc-test";

Example:

rtc_test {
    compatible = "bbb,rtc-test";
    reg = <0x00000000 0x1000>;
    status = "okay";
};

The actual RTC register address and size must match the target
hardware.

Build
make

Expected module:

rtc_driver.ko

Verify:

ls -l rtc_driver.ko
Load Driver
sudo insmod rtc_driver.ko

Check:

lsmod | grep rtc_driver

Check kernel logs:

dmesg | grep -i rtc
RTC Device

Check:

ls -l /dev/rtc*

Example:

/dev/rtc0

Check RTC class:

ls -l /sys/class/rtc/
Read Hardware Clock

If hwclock is available:

sudo hwclock -r

Example:

2026-08-13 10:30:00.123456+05:30
Set Hardware Clock

Set the RTC from the system clock:

sudo hwclock --systohc

Read it again:

sudo hwclock -r
Set RTC Manually

Example:

sudo hwclock --set --date="2026-08-13 10:30:00"

Read:

sudo hwclock -r
RTC Alarm

The example driver supports RTC alarm operations through the Linux
RTC framework.

Check RTC information:

cat /proc/driver/rtc

Example:

rtc_time        : 10:30:00
rtc_date        : 2026-08-13
alrm_time       : 10:35:00
alrm_date       : ****-**-**
alarm_IRQ       : yes
Driver Flow
Device Tree
     |
     v
platform_device
     |
     v
platform_driver
     |
     v
probe()
     |
     +---- Get Memory Resource
     |
     +---- ioremap()
     |
     +---- Allocate RTC Device
     |
     +---- Register RTC Device
     |
     v
Linux RTC Framework
Important RTC Operations

The driver implements:

.read_time
.set_time
.read_alarm
.set_alarm
.alarm_irq_enable

These operations are provided through:

struct rtc_class_ops
Useful Commands

List RTC devices:

ls /dev/rtc*

List RTC class devices:

ls /sys/class/rtc/

Read RTC:

sudo hwclock -r

Set system time to RTC:

sudo hwclock --hctosys

Set RTC from system time:

sudo hwclock --systohc

Show RTC information:

cat /proc/driver/rtc

Check kernel logs:

dmesg | grep -i rtc
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

Read:

make read

Test:

make test

Logs:

make logs
Important Note

The register offsets in rtc_driver.h are example placeholders.

A production BeagleBone Black/AM335x RTC driver must use the actual
RTC register map, clock configuration, power-management behavior,
interrupts, and Device Tree properties for the hardware.

Linux already provides an RTC subsystem and an AM335x RTC driver.
A production system should normally use the existing kernel RTC
driver instead of registering a second driver for the same RTC
hardware.

The architecture is:

Application
     |
     v
hwclock
     |
     v
/dev/rtc0
     |
     v
Linux RTC Framework
     |
     v
AM335x RTC Driver
     |
     v
RTC Hardware

### Final structure

```text
beaglebone-black/
└── drivers/
    └── 10_rtc/
        ├── rtc_driver.c
        ├── rtc_driver.h
        ├── Makefile
        └── README.md

Build and test:

cd beaglebone-black/drivers/10_rtc
make
make status
make load
make logs

One important distinction from your previous drivers: RTC is normally integrated through the Linux RTC framework, so /dev/rtc0 and hwclock are the normal user-space interfaces rather than creating a custom character device.
