# `02_hardware_setup.md`

Create this file:

```text
beaglebone-black/docs/02_hardware_setup.md
```

````markdown
# 02 - BeagleBone Black Hardware Setup

## Table of Contents

- [1. Overview](#1-overview)
- [2. Hardware Requirements](#2-hardware-requirements)
- [3. BeagleBone Black Overview](#3-beaglebone-black-overview)
- [4. AM335x SoC](#4-am335x-soc)
- [5. Hardware Architecture](#5-hardware-architecture)
- [6. Power Supply](#6-power-supply)
- [7. Boot Modes](#7-boot-modes)
- [8. Serial Console](#8-serial-console)
- [9. Ethernet Connection](#9-ethernet-connection)
- [10. USB Connection](#10-usb-connection)
- [11. microSD Card](#11-microsd-card)
- [12. P8 and P9 Headers](#12-p8-and-p9-headers)
- [13. GPIO Hardware](#13-gpio-hardware)
- [14. UART Hardware](#14-uart-hardware)
- [15. I2C Hardware](#15-i2c-hardware)
- [16. SPI Hardware](#16-spi-hardware)
- [17. PWM Hardware](#17-pwm-hardware)
- [18. ADC Hardware](#18-adc-hardware)
- [19. CAN Hardware](#19-can-hardware)
- [20. Pin Multiplexing](#20-pin-multiplexing)
- [21. External Hardware](#21-external-hardware)
- [22. Recommended Test Hardware](#22-recommended-test-hardware)
- [23. Initial Board Bring-Up](#23-initial-board-bring-up)
- [24. Verify Linux](#24-verify-linux)
- [25. Verify CPU and Memory](#25-verify-cpu-and-memory)
- [26. Verify Device Tree](#26-verify-device-tree)
- [27. Verify GPIO](#27-verify-gpio)
- [28. Verify UART](#28-verify-uart)
- [29. Verify I2C](#29-verify-i2c)
- [30. Verify SPI](#30-verify-spi)
- [31. Verify PWM](#31-verify-pwm)
- [32. Verify ADC](#32-verify-adc)
- [33. Verify CAN](#33-verify-can)
- [34. Hardware Debugging](#34-hardware-debugging)
- [35. Safety Precautions](#35-safety-precautions)
- [36. Complete Hardware Setup](#36-complete-hardware-setup)
- [37. Next Step](#37-next-step)

---

# 1. Overview

This document describes the hardware setup required for the
**BeagleBone Black Complete Linux Device Driver Project**.

The project uses the BeagleBone Black as the target Embedded Linux
platform and validates multiple hardware interfaces:

```text
GPIO
UART
I2C
SPI
PWM
ADC
CAN
Ethernet
USB
````

The objective is to create a hardware environment where each
peripheral can be configured, accessed, tested and debugged from
Linux.

---

# 2. Hardware Requirements

## 2.1 Main Board

| Component         | Description                         |
| ----------------- | ----------------------------------- |
| Development Board | BeagleBone Black                    |
| SoC               | TI Sitara AM335x                    |
| CPU               | ARM Cortex-A8                       |
| RAM               | DDR3                                |
| Storage           | eMMC / microSD                      |
| Ethernet          | 10/100 Ethernet                     |
| USB               | USB Host + USB Client               |
| GPIO              | Available through expansion headers |
| UART              | Multiple UART controllers           |
| I2C               | Multiple I2C controllers            |
| SPI               | Multiple SPI controllers            |
| PWM               | Multiple PWM outputs                |
| ADC               | Analog inputs                       |
| CAN               | CAN controller support              |

---

# 3. BeagleBone Black Overview

The BeagleBone Black is an ARM-based Embedded Linux development
platform built around the TI AM335x SoC.

The board exposes many SoC peripherals through the P8 and P9
expansion headers.

Conceptually:

```text
                   BeagleBone Black
                          |
          +---------------+---------------+
          |               |               |
          v               v               v
        AM335x          DDR3           eMMC
          |
          +--------------------------------+
          |                                |
          v                                v
       Peripherals                     Interfaces
          |                                |
    +-----+------+                 +-------+-------+
    |     |      |                 |       |       |
   GPIO  UART   I2C               USB   Ethernet  HDMI
    |
    +---- SPI
    |
    +---- PWM
    |
    +---- ADC
    |
    +---- CAN
```

---

# 4. AM335x SoC

The AM335x provides the processing and peripheral resources used by
this project.

Important blocks include:

```text
+------------------------------------------------------+
|                    AM335x SoC                        |
|                                                      |
|  ARM Cortex-A8                                       |
|                                                      |
|  GPIO                                                |
|  UART                                                |
|  I2C                                                 |
|  SPI                                                 |
|  PWM / Timers                                        |
|  ADC                                                 |
|  CAN                                                 |
|  Ethernet                                            |
|  USB                                                 |
|  DMA                                                 |
|  Interrupt Controller                                |
|  Clock / Power Management                            |
|                                                      |
+------------------------------------------------------+
```

The Linux kernel exposes these hardware blocks through different
subsystems.

---

# 5. Hardware Architecture

The complete hardware-to-software architecture is:

```text
+------------------------------------------------------+
|                  Physical Hardware                   |
|                                                      |
| Sensors | LEDs | Switches | Motors | Displays       |
+---------------------------+--------------------------+
                            |
                            v
+------------------------------------------------------+
|                    BBB Headers                      |
|                                                      |
| P8 / P9                                               |
+---------------------------+--------------------------+
                            |
                            v
+------------------------------------------------------+
|                     AM335x                           |
|                                                      |
| GPIO | UART | I2C | SPI | PWM | ADC | CAN           |
+---------------------------+--------------------------+
                            |
                            v
+------------------------------------------------------+
|                  Linux Kernel                        |
|                                                      |
| Device Tree | Subsystems | Drivers | IRQ | DMA      |
+---------------------------+--------------------------+
                            |
                            v
+------------------------------------------------------+
|                    User Space                        |
|                                                      |
| Test Applications | Utilities | Scripts              |
+------------------------------------------------------+
```

---

# 6. Power Supply

The board requires a suitable regulated power source.

Typical options include:

```text
5V DC Power Supply
```

or

```text
USB Power
```

For a complete peripheral testing setup, a dedicated regulated power
source is preferable.

## Important

Do not apply arbitrary voltage to the expansion headers.

The GPIO pins are **not general-purpose 5 V logic inputs**.

Always verify:

```text
Voltage Level
Current Limit
Pin Function
Pin Direction
```

before connecting external hardware.

---

# 7. Boot Modes

The board can boot from different sources depending on the hardware
configuration and board revision.

Typical boot sources include:

```text
eMMC
microSD
USB
Serial / network-assisted development
```

For this project, microSD boot is useful during kernel and Device
Tree development because it allows easy recovery and testing.

Conceptual boot flow:

```text
Power ON
   |
   v
Boot ROM
   |
   v
SPL
   |
   v
U-Boot
   |
   v
Kernel
   |
   v
Device Tree
   |
   v
Root Filesystem
   |
   v
Linux
```

---

# 8. Serial Console

A serial console is one of the most important tools for Embedded
Linux development.

Use a **3.3 V TTL UART interface** appropriate for the board.

Typical connection:

```text
USB-UART Adapter       BeagleBone Black

GND  ----------------> GND
RX   <---------------- TX
TX   ----------------> RX
```

Do not connect a traditional RS-232 voltage-level interface directly
to the BBB UART pins.

---

## 8.1 Linux Host

Identify the serial device:

```bash
ls /dev/ttyUSB*
```

or:

```bash
ls /dev/ttyACM*
```

Example:

```text
/dev/ttyUSB0
```

---

## 8.2 Using Screen

Install:

```bash
sudo apt install screen
```

Connect:

```bash
screen /dev/ttyUSB0 115200
```

Typical serial configuration:

```text
Baud Rate : 115200
Data Bits : 8
Parity    : None
Stop Bits : 1
Flow Ctrl : None
```

---

# 9. Ethernet Connection

Ethernet is useful for:

```text
SSH
File Transfer
Kernel Deployment
Driver Testing
Remote Debugging
Git
Network Driver Testing
```

Connect:

```text
BeagleBone Black
       |
       | Ethernet
       v
Network Switch / Router
       |
       v
Host PC
```

Check the interface:

```bash
ip link
```

Check IP address:

```bash
ip addr
```

Example:

```bash
ip addr show eth0
```

Test connectivity:

```bash
ping <host-ip>
```

---

# 10. USB Connection

The BeagleBone Black provides USB connectivity for:

```text
USB Storage
USB Serial
USB Keyboard
USB Mouse
USB Ethernet
USB Wi-Fi
USB Debugging
```

Check USB devices:

```bash
lsusb
```

Check kernel messages:

```bash
dmesg | tail -50
```

When a USB device is connected:

```text
USB Device
    |
    v
USB Host Controller
    |
    v
USB Core
    |
    v
USB Driver
    |
    v
User Space
```

---

# 11. microSD Card

A microSD card can be used for:

```text
Bootloader
Kernel
Device Tree
Root Filesystem
Test Images
```

Typical layout:

```text
microSD
 |
 +-- Bootloader
 |
 +-- Kernel
 |
 +-- DTB
 |
 +-- RootFS
```

During development, keeping a known-good boot image is strongly
recommended.

---

# 12. P8 and P9 Headers

The BeagleBone Black exposes many SoC signals through the expansion
headers.

```text
+-------------------------+
|       BeagleBone        |
|                         |
|   P8            P9      |
|   |              |      |
|   |              |      |
|   +--------------+      |
|                         |
+-------------------------+
```

The headers provide combinations of:

```text
GPIO
UART
I2C
SPI
PWM
ADC
Power
Ground
```

The exact pin mapping depends on the board revision and the selected
pin multiplexing configuration.

---

# 13. GPIO Hardware

GPIO is used for digital input and output.

Typical hardware:

```text
+----------------+
| GPIO Pin       |
+-------+--------+
        |
        +---- LED
        |
        +---- Switch
        |
        +---- Sensor
```

Example:

```text
GPIO Output
     |
     v
   LED
     |
    GND
```

For input:

```text
Switch
   |
   v
GPIO Input
```

GPIO testing should include:

```text
Output HIGH
Output LOW
Input HIGH
Input LOW
Interrupt
```

---

# 14. UART Hardware

UART provides asynchronous serial communication.

```text
BBB UART TX  -------->  External RX

BBB UART RX  <--------  External TX

BBB GND      ---------  External GND
```

Typical applications:

```text
GPS
Modem
MCU
Debug Console
Sensor
Industrial Device
```

Testing can be performed using:

```text
USB-UART Adapter
Serial Terminal
Logic Analyzer
```

---

# 15. I2C Hardware

I2C requires:

```text
SDA
SCL
GND
```

Typical connection:

```text
BBB                  I2C Sensor
---                  -----------

SDA  --------------> SDA
SCL  --------------> SCL
GND  --------------> GND
```

I2C normally requires pull-up resistors on SDA and SCL.

Conceptual bus:

```text
             +------ Sensor 1
             |
SDA ---------+------ Sensor 2
             |
             +------ EEPROM

             +------ Sensor 1
             |
SCL ---------+------ Sensor 2
             |
             +------ EEPROM
```

---

# 16. SPI Hardware

SPI typically uses:

```text
SCLK
MOSI
MISO
CS
GND
```

Connection:

```text
BBB                  SPI Device

SCLK  -------------> SCLK
MOSI  -------------> MOSI
MISO  <------------- MISO
CS    -------------> CS
GND   -------------- GND
```

SPI is commonly used for:

```text
Flash
Display
ADC
DAC
IMU
Sensors
```

---

# 17. PWM Hardware

PWM produces a periodic digital waveform.

```text
PWM Pin
   |
   +---- LED
   |
   +---- Servo
   |
   +---- Motor Driver
```

Example waveform:

```text
HIGH      +----+          +----+
          |    |          |    |
LOW  -----+    +----------+    +--------

          <---- Period ---->

Duty Cycle = HIGH time / Period
```

PWM parameters:

```text
Period
Frequency
Duty Cycle
Polarity
Enable
```

---

# 18. ADC Hardware

ADC converts an analog voltage into a digital value.

```text
Analog Sensor
      |
      v
   Voltage
      |
      v
     ADC
      |
      v
 Digital Value
      |
      v
    Linux
```

Typical applications:

```text
Potentiometer
Temperature Sensor
Voltage Monitoring
Current Monitoring
Analog Sensor
```

## Important

ADC inputs have specific voltage limits.

Never connect a voltage source without checking the AM335x ADC input
requirements.

---

# 19. CAN Hardware

CAN requires a CAN controller and normally an external CAN
transceiver for physical bus communication.

Conceptual setup:

```text
+----------------+        +----------------+
| BeagleBone     |        | CAN Node 2     |
|                |        |                |
| CAN Controller |        | CAN Controller |
+-------+--------+        +-------+--------+
        |                         |
        v                         v
   CAN Transceiver          CAN Transceiver
        |                         |
        +-----------+-------------+
                    |
                  CAN Bus
```

A practical CAN setup normally includes:

```text
CAN Controller
CAN Transceiver
CAN_H
CAN_L
GND
Proper Bus Termination
```

CAN testing tools:

```bash
candump
cansend
cangen
```

---

# 20. Pin Multiplexing

One of the most important concepts on the BeagleBone Black is
**pin multiplexing**.

A physical pin can support different functions.

Conceptually:

```text
             Physical Pin
                  |
        +---------+---------+
        |         |         |
        v         v         v
       GPIO      UART      SPI
```

The selected function is controlled by the SoC pinmux configuration.

Therefore:

```text
Peripheral Enabled
        +
Correct Pinmux
        +
Correct Device Tree
        =
Working Peripheral
```

A driver may be perfectly implemented but the peripheral can still
fail if the pinmux is incorrect.

---

# 21. External Hardware

To test all peripherals, connect suitable external devices.

Recommended setup:

```text
GPIO
 |
 +---- LED
 +---- Push Button

UART
 |
 +---- USB-UART Adapter

I2C
 |
 +---- EEPROM
 +---- Temperature Sensor

SPI
 |
 +---- SPI Sensor
 +---- SPI Flash

PWM
 |
 +---- LED
 +---- Servo / Motor Driver

ADC
 |
 +---- Potentiometer
 +---- Analog Sensor

CAN
 |
 +---- CAN Transceiver
 +---- CAN Analyzer / Second CAN Node
```

---

# 22. Recommended Test Hardware

| Peripheral | Recommended Hardware    |
| ---------- | ----------------------- |
| GPIO       | LED + Push Button       |
| UART       | USB-UART Adapter        |
| I2C        | EEPROM / Sensor         |
| SPI        | SPI Flash / Sensor      |
| PWM        | LED / Servo             |
| ADC        | Potentiometer           |
| CAN        | CAN Transceiver         |
| Interrupt  | Push Button             |
| DMA        | High-rate UART/SPI test |
| Debugging  | Logic Analyzer          |
| Timing     | Oscilloscope            |
| Network    | Ethernet Switch         |

---

# 23. Initial Board Bring-Up

After connecting power and serial console:

```text
Power ON
   |
   v
U-Boot
   |
   v
Linux Kernel
   |
   v
Login Prompt
```

Verify the console:

```text
BeagleBone Black
Linux
login:
```

Login to the board.

---

# 24. Verify Linux

Check kernel version:

```bash
uname -a
```

Check kernel release:

```bash
uname -r
```

Example:

```text
Linux beaglebone 6.x.x ...
```

Check architecture:

```bash
uname -m
```

Expected architecture:

```text
armv7l
```

---

# 25. Verify CPU and Memory

CPU information:

```bash
cat /proc/cpuinfo
```

Memory:

```bash
free -h
```

Detailed memory:

```bash
cat /proc/meminfo
```

Check uptime:

```bash
uptime
```

---

# 26. Verify Device Tree

The live Device Tree is normally available under:

```bash
/sys/firmware/devicetree/base/
```

List nodes:

```bash
ls /sys/firmware/devicetree/base/
```

Check compatible information:

```bash
find /sys/firmware/devicetree/base/ -name compatible -print
```

Inspect a property:

```bash
cat /sys/firmware/devicetree/base/model
```

The exact contents depend on the kernel and Device Tree used.

---

# 27. Verify GPIO

First inspect available GPIO-related interfaces:

```bash
ls /sys/class/gpio/
```

On newer kernels, GPIO character-device interfaces are preferred.

Check GPIO chips:

```bash
gpiodetect
```

List GPIO lines:

```bash
gpioinfo
```

These commands are provided by the `libgpiod` tools when installed.

Example:

```bash
sudo apt install gpiod
```

Then:

```bash
gpiodetect
gpioinfo
```

---

# 28. Verify UART

List serial devices:

```bash
ls /dev/tty*
```

Filter:

```bash
ls /dev/ttyS*
```

Check kernel messages:

```bash
dmesg | grep -i tty
```

Check serial devices:

```bash
ls -l /dev/ttyS*
```

A UART may appear as a `ttyS*` device depending on the kernel
configuration and driver.

---

# 29. Verify I2C

Install I2C tools:

```bash
sudo apt install i2c-tools
```

List I2C adapters:

```bash
i2cdetect -l
```

Example:

```text
i2c-0
i2c-1
```

Scan an adapter:

```bash
sudo i2cdetect -y 1
```

Example result:

```text
     0 1 2 3 4 5 6 7 8 9 a b c d e f
00: -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

An address appears only when a responding device is detected.

---

# 30. Verify SPI

Check SPI-related kernel messages:

```bash
dmesg | grep -i spi
```

Check device nodes:

```bash
ls -l /dev/spidev*
```

Possible result:

```text
/dev/spidev0.0
```

If `spidev` is not present, investigate:

```text
Device Tree
SPI controller
SPI device node
Kernel configuration
Pinmux
Driver
```

---

# 31. Verify PWM

Inspect PWM interfaces:

```bash
ls /sys/class/pwm/
```

On systems using the modern PWM framework, inspect:

```bash
find /sys/class/pwm/ -maxdepth 2 -type f
```

Check kernel messages:

```bash
dmesg | grep -i pwm
```

The exact sysfs interface depends on the kernel version.

---

# 32. Verify ADC

ADC support is normally exposed through the Linux IIO subsystem.

Check:

```bash
ls /sys/bus/iio/devices/
```

Example:

```text
iio:device0
```

Inspect:

```bash
ls /sys/bus/iio/devices/iio:device0/
```

Look for channel files such as:

```text
in_voltage*_raw
```

The exact channel naming depends on the kernel and Device Tree.

---

# 33. Verify CAN

Check CAN interfaces:

```bash
ip link
```

If CAN is configured, an interface such as:

```text
can0
```

may appear.

Configure bitrate:

```bash
sudo ip link set can0 type can bitrate 500000
```

Bring the interface up:

```bash
sudo ip link set can0 up
```

Check:

```bash
ip -details link show can0
```

Receive:

```bash
candump can0
```

Transmit:

```bash
cansend can0 123#11223344
```

A functional physical CAN network requires the appropriate
transceiver, wiring and termination.

---

# 34. Hardware Debugging

## 34.1 Check Kernel Logs

Always start with:

```bash
dmesg
```

Peripheral-specific:

```bash
dmesg | grep -i gpio
dmesg | grep -i uart
dmesg | grep -i i2c
dmesg | grep -i spi
dmesg | grep -i pwm
dmesg | grep -i adc
dmesg | grep -i can
```

---

## 34.2 Check Device Nodes

```bash
ls -l /dev/
```

Examples:

```text
/dev/ttyS*
/dev/spidev*
/dev/i2c-*
```

---

## 34.3 Check Kernel Modules

List loaded modules:

```bash
lsmod
```

Load a module:

```bash
sudo modprobe <module>
```

Remove a module:

```bash
sudo modprobe -r <module>
```

---

# 35. Logic Analyzer

A logic analyzer is extremely useful for:

```text
UART
I2C
SPI
GPIO
PWM
```

Example SPI debugging:

```text
SCLK  ───────────────────────
MOSI  ──▔▔──▁▁──▔▔──────────
MISO  ──▁▁──▔▔──▁▁──────────
CS    ────▁▁▁▁───────────────
```

Use it to verify:

```text
Clock
Data
Timing
Chip Select
Baud Rate
Protocol
```

---

# 36. Oscilloscope

An oscilloscope is useful for:

```text
PWM
UART
SPI
GPIO
Clock Signals
Analog Signals
```

For PWM, verify:

```text
Frequency
Period
Duty Cycle
Rise Time
Fall Time
```

For ADC testing, verify the analog input voltage with a multimeter
or oscilloscope before comparing it against the ADC reading.

---

# 37. Safety Precautions

## 37.1 Voltage

Never assume every BBB header signal accepts 5 V.

Always check:

```text
AM335x Electrical Specifications
Board Documentation
Pin Function
Device Tree Configuration
```

---

## 37.2 Current

Do not drive high-current loads directly from GPIO.

For:

```text
Motor
Relay
Large LED
Fan
Solenoid
```

use an appropriate driver circuit.

Example:

```text
GPIO
 |
 v
Transistor / MOSFET Driver
 |
 v
Load
```

---

## 37.3 Ground

For external digital devices:

```text
BBB GND
   |
   +---- Sensor GND
   +---- UART GND
   +---- SPI GND
   +---- I2C GND
```

A common reference ground is normally required.

---

# 38. Complete Hardware Setup

The recommended project hardware setup is:

```text
                         +------------------+
                         |   Host Ubuntu    |
                         |                  |
                         | SSH / Git / Build|
                         +--------+---------+
                                  |
                              Ethernet
                                  |
                                  v
+----------------------------------------------------------------+
|                     BeagleBone Black                           |
|                                                                |
|                         AM335x                                 |
|                                                                |
| GPIO ---------> LED / Button                                  |
|                                                                |
| UART ---------> USB-UART                                      |
|                                                                |
| I2C ----------> Sensor / EEPROM                               |
|                                                                |
| SPI ----------> SPI Sensor / Flash                             |
|                                                                |
| PWM ----------> LED / Servo Driver                             |
|                                                                |
| ADC ----------> Potentiometer / Analog Sensor                 |
|                                                                |
| CAN ----------> CAN Transceiver                               |
|                                                                |
| USB ----------> USB Devices                                   |
|                                                                |
+----------------------------------------------------------------+
                                  |
                                  v
                         Logic Analyzer
                         Oscilloscope
                         CAN Analyzer
```

---

# 39. Recommended Development Sequence

Do not connect and debug every peripheral simultaneously.

Use this sequence:

```text
Step 1
  |
  v
Board Boot
  |
  v
Step 2
  |
  v
Serial Console
  |
  v
Step 3
  |
  v
Ethernet / SSH
  |
  v
Step 4
  |
  v
GPIO
  |
  v
Step 5
  |
  v
UART
  |
  v
Step 6
  |
  v
I2C
  |
  v
Step 7
  |
  v
SPI
  |
  v
Step 8
  |
  v
PWM
  |
  v
Step 9
  |
  v
ADC
  |
  v
Step 10
  |
  v
CAN
  |
  v
Step 11
  |
  v
Interrupts
  |
  v
Step 12
  |
  v
DMA
```

This makes debugging much easier because each layer is validated
before moving to the next one.

---

# 40. Hardware Bring-Up Checklist

## Board

* [ ] BeagleBone Black available
* [ ] Correct power supply available
* [ ] microSD card available
* [ ] Ethernet cable available
* [ ] USB cable available
* [ ] USB-UART adapter available

## Console

* [ ] UART TX connected
* [ ] UART RX connected
* [ ] GND connected
* [ ] Serial terminal working
* [ ] 115200 baud configured

## GPIO

* [ ] LED connected
* [ ] Button connected
* [ ] Correct voltage level verified
* [ ] GPIO output tested
* [ ] GPIO input tested
* [ ] GPIO interrupt tested

## I2C

* [ ] I2C sensor available
* [ ] SDA connected
* [ ] SCL connected
* [ ] GND connected
* [ ] Pull-ups verified
* [ ] I2C address detected

## SPI

* [ ] SPI peripheral available
* [ ] SCLK connected
* [ ] MOSI connected
* [ ] MISO connected
* [ ] CS connected
* [ ] GND connected
* [ ] SPI transaction verified

## PWM

* [ ] PWM output connected
* [ ] Frequency verified
* [ ] Duty cycle verified
* [ ] Oscilloscope/logic analyzer available

## ADC

* [ ] Analog source connected
* [ ] Input voltage verified
* [ ] ADC channel configured
* [ ] ADC value verified

## CAN

* [ ] CAN transceiver available
* [ ] CAN_H connected
* [ ] CAN_L connected
* [ ] GND connected
* [ ] Bus termination verified
* [ ] CAN bitrate configured
* [ ] `can0` available
* [ ] `candump` tested
* [ ] `cansend` tested

---

# 41. Hardware-to-Driver Mapping

| Hardware | Device Tree         | Kernel Subsystem | Driver/Test |
| -------- | ------------------- | ---------------- | ----------- |
| GPIO     | `device-tree/gpio/` | GPIO             | GPIO test   |
| UART     | `device-tree/uart/` | TTY/Serial       | UART test   |
| I2C      | `device-tree/i2c/`  | I2C              | I2C test    |
| SPI      | `device-tree/spi/`  | SPI              | SPI test    |
| PWM      | `device-tree/pwm/`  | PWM              | PWM test    |
| ADC      | `device-tree/adc/`  | IIO              | ADC test    |
| CAN      | `device-tree/can/`  | SocketCAN        | CAN test    |

---

# 42. Hardware Setup to Driver Flow

Once the hardware is connected, the development flow becomes:

```text
                  HARDWARE
                     |
                     v
                Pin Mapping
                     |
                     v
                 Pinmux
                     |
                     v
               Device Tree
                     |
                     v
             Kernel Configuration
                     |
                     v
             Kernel Subsystem
                     |
                     v
                  Driver
                     |
                     v
                 Probe()
                     |
                     v
             Hardware Init
                     |
                     v
             Device Interface
                     |
                     v
              User Application
                     |
                     v
              Physical Testing
```

---

# 43. Project Hardware Directory

The hardware documentation can be organized as:

```text
hardware/
├── README.md
├── pinout/
│   ├── p8.md
│   └── p9.md
├── gpio/
├── uart/
├── i2c/
├── spi/
├── pwm/
├── adc/
├── can/
└── schematics/
```

Optional hardware diagrams can be added later.

---

# 44. Final Hardware Architecture

```text
                         HOST PC
                           |
             +-------------+-------------+
             |                           |
          Ethernet                     USB
             |                           |
             v                           v
      +---------------------------------------+
      |          BeagleBone Black             |
      |                                       |
      |              AM335x                   |
      |                                       |
      |  +-------+  +-------+  +-------+     |
      |  | GPIO  |  | UART  |  |  I2C  |     |
      |  +---+---+  +---+---+  +---+---+     |
      |      |          |          |           |
      |      v          v          v           |
      |     LED       UART       Sensor        |
      |                                       |
      |  +-------+  +-------+  +-------+     |
      |  |  SPI  |  |  PWM  |  |  ADC  |     |
      |  +---+---+  +---+---+  +---+---+     |
      |      |          |          |           |
      |      v          v          v           |
      |    Sensor      Motor     Analog        |
      |                                       |
      |              +-------+                |
      |              |  CAN  |                |
      |              +---+---+                |
      |                  |                    |
      +------------------+--------------------+
                         |
                         v
                   CAN Transceiver
                         |
                         v
                      CAN Bus
```

---

# 45. Final Verification

Before starting driver development, confirm:

```bash
uname -a
```

```bash
ip addr
```

```bash
ls /sys/firmware/devicetree/base/
```

```bash
dmesg | tail -50
```

```bash
ls /dev/
```

Then verify each subsystem:

```bash
# GPIO
gpiodetect

# UART
ls /dev/ttyS*

# I2C
i2cdetect -l

# SPI
ls /dev/spidev*

# PWM
ls /sys/class/pwm/

# ADC
ls /sys/bus/iio/devices/

# CAN
ip link
```

---

# 46. Next Step

After the physical hardware is verified, proceed to:

```text
02_hardware_setup.md
        |
        v
03_device_tree.md
        |
        v
04_kernel_build.md
        |
        v
05_driver_model.md
        |
        v
Peripheral Drivers
        |
        v
User-Space Testing
        |
        v
Hardware Validation
```

The next major step is to configure the **Device Tree for GPIO,
UART, I2C, SPI, PWM, ADC and CAN** and understand exactly how each
Device Tree node gets matched with its Linux driver.

```
```

