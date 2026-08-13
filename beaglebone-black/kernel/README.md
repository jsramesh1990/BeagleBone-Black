# `beaglebone-black/kernel/README.md`

````markdown
# BeagleBone Black Linux Kernel

## Overview

This directory contains the Linux kernel configuration and kernel
patches used for the BeagleBone Black device-driver development project.

The kernel is the central software layer between the BeagleBone Black
hardware and Linux user space.

```text
+-----------------------------+
|       User Applications     |
+-----------------------------+
              |
              v
+-----------------------------+
|        Linux User Space     |
|  /dev  /sys  /proc  ioctl   |
+-----------------------------+
              |
              v
+-----------------------------+
|        Linux Kernel         |
|                             |
| GPIO | ADC | I2C | SPI      |
| UART | PWM | CAN | USB      |
+-----------------------------+
              |
              v
+-----------------------------+
|      BeagleBone Black       |
|       AM335x Hardware       |
+-----------------------------+
````

---

# Directory Structure

```text
kernel/
│
├── config/
│   └── README.md
│
├── patches/
│   └── README.md
│
└── README.md
```

---

# 1. `config/`

The `config` directory contains kernel configuration information
required to enable the Linux subsystems and drivers used by this
project.

Important configuration areas include:

```text
CONFIG_GPIO
CONFIG_I2C
CONFIG_SPI
CONFIG_SERIAL
CONFIG_PWM
CONFIG_CAN
CONFIG_ADC
```

The exact configuration symbols depend on the Linux kernel version and
the driver implementation being used.

---

## Kernel Configuration Flow

```text
Linux Kernel Source
        |
        v
make menuconfig
        |
        v
Kernel Configuration
        |
        v
.config
        |
        v
Kernel Build
        |
        v
zImage / Image
        |
        v
BeagleBone Black
```

---

# 2. `patches/`

The `patches` directory contains project-specific Linux kernel patches.

Patches may be used for:

* Device-driver modifications
* Device Tree changes
* Kernel configuration changes
* Hardware-specific fixes
* Bug fixes
* Board support changes
* Driver enhancements
* Debugging changes

Example:

```text
patches/
├── 0001-enable-bbb-peripherals.patch
├── 0002-add-custom-gpio-driver.patch
├── 0003-add-custom-i2c-device.patch
└── 0004-add-custom-spi-device.patch
```

Patch numbering should follow the order in which patches are applied.

---

# Kernel Development Flow

The overall kernel development process is:

```text
Kernel Source
     |
     v
Kernel Configuration
     |
     v
Device Tree
     |
     v
Kernel Patches
     |
     v
Kernel Compilation
     |
     v
Kernel Installation
     |
     v
BeagleBone Black Boot
     |
     v
Driver Initialization
     |
     v
Hardware Testing
```

---

# Device Driver Integration

This project uses the Linux kernel to integrate the BeagleBone Black
peripherals.

```text
                 Linux Kernel
                      |
        +-------------+-------------+
        |             |             |
        v             v             v
      GPIO           I2C           SPI
        |             |             |
        v             v             v
      Driver        Driver        Driver
        |             |             |
        +-------------+-------------+
                      |
                      v
                 Hardware
```

Additional interfaces:

```text
ADC
CAN
PWM
UART
```

---

# Kernel Configuration Areas

The project should verify the required kernel subsystems.

## GPIO

GPIO support is required for:

```text
LED
Button
Reset
Interrupt
Digital Input
Digital Output
```

---

## I2C

I2C support is required for:

```text
Sensors
EEPROM
RTC
PMIC
Temperature Sensors
```

---

## SPI

SPI support is required for:

```text
SPI Flash
ADC
Displays
Sensors
External Controllers
```

---

## UART

UART support is required for:

```text
Debug Console
GPS
Bluetooth
Modem
External MCU
```

---

## PWM

PWM support is required for:

```text
LED brightness
Motor control
Fan control
Buzzer
Servo
```

---

## CAN

CAN support is required for:

```text
Automotive communication
Industrial control
CAN sensors
CAN controllers
```

---

## ADC

ADC support is required for:

```text
Analog sensors
Voltage measurement
Potentiometers
Analog monitoring
```

---

# Kernel and Device Tree Relationship

Device Tree describes the hardware to the Linux kernel.

```text
                    Device Tree
                         |
                         v
                 Hardware Description
                         |
                         v
                  Kernel Subsystem
                         |
                         v
                    Driver Probe
                         |
                         v
                     Hardware
```

Example:

```text
Device Tree
    |
    v
I2C Controller Enabled
    |
    v
I2C Driver
    |
    v
I2C Peripheral
```

---

# Kernel Build Flow

Typical kernel build flow:

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- <config>
```

Then:

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j$(nproc)
```

The exact commands depend on the selected kernel version, toolchain,
and build environment.

---

# Kernel Configuration

Start configuration using:

```bash
make menuconfig
```

For an ARM target:

```bash
make ARCH=arm menuconfig
```

Configuration is stored in:

```text
.config
```

A project configuration can be saved as:

```text
config/bbb_defconfig
```

---

# Configuration Management

Recommended structure:

```text
kernel/
│
├── config/
│   ├── README.md
│   └── bbb_defconfig
│
└── patches/
    ├── README.md
    └── *.patch
```

This makes the kernel configuration reproducible.

---

# Kernel Patching

Apply a patch using:

```bash
git apply 0001-example.patch
```

or:

```bash
git am 0001-example.patch
```

depending on how the patch was created.

Check the working tree:

```bash
git status
```

Review changes:

```bash
git diff
```

---

# Creating a Patch

After making a kernel change:

```bash
git diff > 0001-example.patch
```

For Git commits:

```bash
git format-patch -1 HEAD
```

For multiple commits:

```bash
git format-patch <base-commit>..HEAD
```

---

# Kernel Debugging

After booting the new kernel, check:

```bash
dmesg
```

For a particular subsystem:

```bash
dmesg | grep -i gpio
dmesg | grep -i i2c
dmesg | grep -i spi
dmesg | grep -i uart
dmesg | grep -i pwm
dmesg | grep -i can
```

Check the kernel version:

```bash
uname -a
```

or:

```bash
uname -r
```

---

# Driver Probe Flow

When Linux boots:

```text
Kernel Boot
     |
     v
Device Tree Parsed
     |
     v
Device Registered
     |
     v
Driver Matching
     |
     v
probe()
     |
     v
Hardware Initialization
     |
     v
Device Interface Created
     |
     v
User Application
```

A successful probe should normally be visible in kernel logs.

---

# Kernel Logs

Use:

```bash
dmesg
```

For live kernel messages:

```bash
dmesg -w
```

For a specific driver:

```bash
dmesg | grep -i <driver_name>
```

---

# Sysfs

Linux exposes hardware and driver information through:

```text
/sys
```

Examples:

```bash
ls /sys/class/
```

GPIO:

```bash
ls /sys/class/gpio/
```

PWM:

```bash
ls /sys/class/pwm/
```

I2C:

```bash
ls /sys/bus/i2c/devices/
```

SPI:

```bash
ls /sys/bus/spi/devices/
```

---

# Device Nodes

Some drivers expose devices through `/dev`.

Examples:

```text
/dev/tty*
/dev/i2c-*
/dev/spidev*
```

The exact device node depends on the driver and kernel configuration.

---

# Kernel Testing Flow

```text
             Kernel Build
                  |
                  v
             Boot Board
                  |
                  v
             Check dmesg
                  |
                  v
          Check Device Tree
                  |
                  v
         Check Driver Probe
                  |
                  v
          Check /dev or /sys
                  |
                  v
          Hardware Test
                  |
                  v
         Application Test
```

---

# Reproducible Kernel Build

The objective of this directory is to make kernel development
reproducible.

```text
Source Code
     +
Config
     +
Patches
     +
Device Tree
     |
     v
Reproducible Kernel Build
```

A new developer should be able to clone the repository and reproduce
the same kernel configuration and project-specific modifications.

---

# Kernel Version Tracking

Always record the kernel version used by the project.

Example:

```bash
uname -r
```

For the kernel source:

```bash
git describe --always --tags
```

Recommended documentation:

```text
Kernel Version:
Toolchain:
Architecture:
Defconfig:
Applied Patches:
Device Tree:
Build Environment:
```

---

# Recommended Project Structure

The complete BeagleBone Black kernel section can eventually look like:

```text
beaglebone-black/
│
├── kernel/
│   │
│   ├── README.md
│   │
│   ├── config/
│   │   ├── README.md
│   │   └── bbb_defconfig
│   │
│   └── patches/
│       ├── README.md
│       ├── 0001-*.patch
│       ├── 0002-*.patch
│       └── 0003-*.patch
│
├── device-tree/
│
├── drivers/
│
├── hardware/
│
└── tests/
```

---

# Kernel Layer in the Complete Project

The kernel directory connects the Device Tree, drivers, and hardware
documentation.

```text
                         Project
                            |
          +-----------------+-----------------+
          |                 |                 |
          v                 v                 v
      Hardware         Device Tree         Kernel
          |                 |                 |
          |                 v                 |
          |            Hardware Info         |
          |                 |                 |
          +-----------------+-----------------+
                            |
                            v
                       Device Driver
                            |
                            v
                     Linux Subsystem
                            |
                            v
                       User Space
```

---

# Final Objective

The `kernel/` directory provides the foundation for building and
customizing the Linux kernel used by the BeagleBone Black
device-driver project.

The main responsibilities are:

```text
1. Kernel configuration
2. Kernel subsystem enablement
3. Kernel patch management
4. Device Tree integration
5. Driver integration
6. Kernel compilation
7. Kernel deployment
8. Kernel debugging
9. Hardware validation
10. Reproducible builds
```

The final development flow is:

```text
Hardware
   |
   v
Device Tree
   |
   v
Kernel Configuration
   |
   v
Kernel Patches
   |
   v
Linux Driver
   |
   v
Kernel Build
   |
   v
BeagleBone Black
   |
   v
Driver Probe
   |
   v
/dev + /sys
   |
   v
Hardware Test
   |
   v
User Application
```

```
```
```

## What this configuration enables

Your project now has a kernel baseline for:

```text
+------------------------------------------------+
|              bbb_driver_defconfig              |
+------------------------------------------------+
|                                                |
|  GPIO       → GPIOLIB                          |
|  I2C        → I2C + I2C_CHARDEV               |
|  SPI        → SPI + SPI_MASTER + SPIDEV        |
|  UART       → 8250 Serial + Console            |
|  PWM        → PWM Framework                    |
|  CAN        → CAN + SocketCAN                  |
|  ADC        → IIO Framework                    |
|  DeviceTree → OF                               |
|  Debug      → DEBUG_FS + DYNAMIC_DEBUG         |
|  Modules    → Kernel Module Support            |
|                                                |
+------------------------------------------------+
```

## Project flow

```text
                    bbb_driver_defconfig
                             |
                             v
                    Linux Kernel Config
                             |
          +------------------+------------------+
          |        |         |        |         |
          v        v         v        v         v
        GPIO      I2C       SPI      UART      PWM
          |        |         |        |         |
          +--------+---------+--------+---------+
                             |
                       +-----+-----+
                       |           |
                       v           v
                      CAN         ADC
                       |           |
                       +-----+-----+
                             |
                             v
                       Device Tree
                             |
                             v
                         Drivers
                             |
                             v
                    BeagleBone Black
```

### Verify it against your kernel

After placing the file in:

```text
beaglebone-black/kernel/config/bbb_driver_defconfig
```

copy it into your kernel source:

```bash
cp beaglebone-black/kernel/config/bbb_driver_defconfig .config
```

Then run:

```bash
make ARCH=arm olddefconfig
```

Check for configuration problems:

```bash
make ARCH=arm menuconfig
```

And verify the important options:

```bash
grep -E 'CONFIG_(GPIOLIB|I2C|SPI|SERIAL|PWM|CAN|IIO)=' .config
```

For your project, the next logical file is **`kernel/config/fragments/gpio.cfg`**, followed by the I2C, SPI, UART, PWM, CAN, and ADC fragments.

