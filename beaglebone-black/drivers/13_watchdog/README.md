# BeagleBone Black Watchdog Demo

## Overview

This project demonstrates the Linux Watchdog framework using a
platform driver.

A watchdog timer is used to detect software failures. If software
stops responding and fails to service the watchdog, the hardware
watchdog can reset the system.

The Linux watchdog architecture is:

```text
Application
     |
     v
/dev/watchdog0
     |
     v
Linux Watchdog Framework
     |
     v
Watchdog Driver
     |
     v
Hardware Watchdog Timer
     |
     v
System Reset
Directory Structure
13_watchdog/
├── watchdog_demo.c
├── watchdog_demo.h
├── Makefile
└── README.md
Watchdog Flow
Device Tree
     |
     v
Platform Device
     |
     v
watchdog_demo_probe()
     |
     v
watchdog_device
     |
     v
Linux Watchdog Framework
     |
     v
/dev/watchdog0
     |
     +----------+
     |          |
     v          v
   Start      Ping
     |          |
     +----------+
          |
          v
   Watchdog Timer
          |
          v
      Timeout
          |
          v
       Reset
Device Tree

The example driver expects:

compatible = "bbb,watchdog-demo";

Example:

watchdog_demo {
    compatible = "bbb,watchdog-demo";
    status = "okay";
};

The actual hardware watchdog should be connected to the correct
watchdog hardware driver in a production system.

Build
make

Expected module:

watchdog_demo.ko

Verify:

ls -l watchdog_demo.ko
Load Driver
sudo insmod watchdog_demo.ko

Check:

lsmod | grep watchdog_demo

Check kernel messages:

dmesg | grep -i watchdog
Watchdog Device

Check:

ls -l /dev/watchdog*

Typical output:

/dev/watchdog0

Check sysfs:

ls -l /sys/class/watchdog/

Read driver identity:

cat /sys/class/watchdog/watchdog0/identity
Watchdog Timeout

The default timeout is:

10 seconds

Supported range:

Minimum: 1 second
Maximum: 120 seconds

The timeout can be changed through the Linux watchdog interface.

Using watchdogctl

If available:

watchdogctl

Check available watchdog options:

watchdogctl --help
Using hwclock

hwclock is not used for watchdog testing.

The watchdog device is normally accessed through:

/dev/watchdog0
Watchdog Keepalive

A watchdog must be periodically serviced.

Conceptually:

Application
    |
    v
Watchdog ping
    |
    v
Timer restarted
    |
    v
Continue running

If the application stops servicing the watchdog:

Application
    |
    X
No watchdog ping
    |
    v
Timer expires
    |
    v
Watchdog reset
Watchdog Operations

The driver implements:

.start
.stop
.ping
.set_timeout

These operations are registered through:

struct watchdog_ops
Watchdog Start

When the watchdog is started:

/dev/watchdog0
       |
       v
watchdog framework
       |
       v
bbb_watchdog_start()
       |
       v
Hardware watchdog starts
Watchdog Ping

The watchdog is periodically serviced:

Ping
 |
 v
Timer reset
 |
 v
Continue execution

The driver implements this through:

bbb_watchdog_ping()
Timeout Configuration

The timeout is changed through:

bbb_watchdog_set_timeout()

Example concept:

Timeout = 10 seconds

Application must ping before:

10 seconds
    |
    v
Watchdog expiration
Debugging

Check driver:

lsmod | grep watchdog_demo

Check watchdog:

ls -l /dev/watchdog*

Check kernel logs:

dmesg | grep -i watchdog

Check watchdog class:

ls /sys/class/watchdog/
Remove Driver
sudo rmmod watchdog_demo

Check:

lsmod | grep watchdog_demo
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

Logs:

make logs

Test:

make test
Production Architecture

For an actual BeagleBone Black system, the preferred architecture is:

                  Application
                       |
                       v
                Watchdog Service
                       |
                       v
                 /dev/watchdog0
                       |
                       v
             Linux Watchdog Framework
                       |
                       v
               AM335x Watchdog Driver
                       |
                       v
                Hardware Watchdog
                       |
              +--------+--------+
              |                 |
           Ping OK          No Ping
              |                 |
              v                 v
          Continue           Timeout
                                |
                                v
                             RESET

The BeagleBone Black/AM335x already has hardware watchdog
functionality, so a production system should normally use the
existing SoC watchdog driver rather than replacing it with a
software-only demo driver.

Important Note

This example demonstrates the Linux watchdog framework API and
driver structure.

The start, stop, ping, and set_timeout functions here do not
program actual AM335x watchdog registers.

For a real hardware watchdog driver, these callbacks must access the
actual watchdog hardware registers and correctly implement:

Watchdog start
Watchdog stop
Watchdog reset
Timeout configuration
Keepalive
Clock configuration
System reset behavior
Suspend/resume handling
Device Tree configuration

Therefore this project is intended primarily for Linux kernel
watchdog framework learning and driver architecture.


### Final structure

```text
beaglebone-black/
└── drivers/
    └── 13_watchdog/
        ├── watchdog_demo.c
        ├── watchdog_demo.h
        ├── Makefile
        └── README.md

Driver flow to remember for interviews:

Device Tree
    ↓
Platform Driver
    ↓
probe()
    ↓
struct watchdog_device
    ↓
watchdog_register_device()
    ↓
/dev/watchdog0
    ↓
Start → Ping → Timeout
    ↓
Hardware Reset
