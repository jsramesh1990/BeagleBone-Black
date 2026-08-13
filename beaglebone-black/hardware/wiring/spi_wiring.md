# `spi_wiring.md`

````markdown
# BeagleBone Black SPI Wiring

## 1. Overview

This document describes the hardware wiring required to test SPI
(Serial Peripheral Interface) communication on the BeagleBone Black.

SPI is a synchronous serial communication protocol commonly used for:

- ADCs
- DACs
- Flash memory
- EEPROMs
- Displays
- IMUs
- Sensors
- CAN controllers
- Ethernet controllers
- RF modules
- Motor-control peripherals

Basic SPI flow:

```text
User Application
       |
       v
Linux SPI Framework
       |
       v
SPI Controller Driver
       |
       v
AM335x SPI Controller
       |
       v
SPI Bus
       |
       +--------> SPI Peripheral
````

---

# 2. SPI Signals

A typical SPI bus uses four main signals:

```text
MOSI → Master Out / Slave In
MISO → Master In / Slave Out
SCLK → Serial Clock
CS   → Chip Select
```

Typical connection:

```text
              BeagleBone Black
                  SPI Master
                      |
       +--------------+--------------+
       |              |              |
      MOSI           MISO           SCLK
       |              |              |
       v              v              v
   +--------------------------------------+
   |             SPI Device              |
   |                                      |
   | MOSI          MISO          SCLK     |
   +--------------------------------------+
                      |
                     CS
                      ^
                      |
              BeagleBone GPIO/SPI CS
```

---

# 3. Required Components

For a basic SPI test:

```text
1 × BeagleBone Black
1 × SPI peripheral
Jumper wires
Breadboard
Logic analyzer or oscilloscope (recommended)
```

Example SPI peripherals:

```text
SPI ADC
SPI DAC
SPI Flash
SPI Display
SPI IMU
SPI EEPROM
SPI CAN controller
```

---

# 4. SPI Pin Selection

Before wiring the SPI device, check:

```text
hardware/pinout/spi_pin_map.md
```

Use this file to identify:

```text
SPI controller
MOSI pin
MISO pin
SCLK pin
CS pin
Header location
Pinmux mode
```

Do not assume that every header pin supports SPI.

The AM335x pins are multiplexed and must be configured for the required
SPI function.

---

# 5. Basic SPI Wiring

Typical connection:

```text
BeagleBone Black       SPI Peripheral
-----------------      --------------

MOSI ----------------> MOSI

MISO <---------------- MISO

SCLK ----------------> SCLK

CS   ----------------> CS

GND  ----------------> GND

3.3V ----------------> VCC
```

Complete:

```text
                  BeagleBone Black
                       SPI Master
                          |
        +-----------------+-----------------+
        |        |        |        |        |
       MOSI     MISO     SCLK      CS       GND
        |        |        |        |        |
        v        v        v        v        v
   +---------------------------------------------+
   |                SPI DEVICE                  |
   |                                             |
   | MOSI     MISO     SCLK      CS       GND   |
   +---------------------------------------------+
```

---

# 6. SPI Wiring Table

| BeagleBone Black   | SPI Device |
| ------------------ | ---------- |
| MOSI               | MOSI / SDI |
| MISO               | MISO / SDO |
| SCLK               | SCLK / CLK |
| CS                 | CS / SS    |
| GND                | GND        |
| Appropriate supply | VCC        |

The exact header pins must be taken from:

```text
hardware/pinout/spi_pin_map.md
```

---

# 7. SPI Signal Direction

SPI is generally full-duplex.

```text
MOSI:

BeagleBone ------------------> SPI Device


MISO:

BeagleBone <------------------ SPI Device


SCLK:

BeagleBone ------------------> SPI Device


CS:

BeagleBone ------------------> SPI Device
```

The BeagleBone acts as the SPI master in this project.

---

# 8. SPI Master and Slave

Typical architecture:

```text
                MASTER
           BeagleBone Black
                  |
       +----------+----------+
       |          |          |
      MOSI       MISO       SCLK
       |          |          |
       +----------+----------+
                  |
                 CS
                  |
                  v
               SLAVE
             SPI Device
```

The master normally controls:

```text
Clock
Chip Select
Transfer timing
```

The slave responds when selected.

---

# 9. SPI Multiple Devices

Multiple SPI devices can share:

```text
MOSI
MISO
SCLK
```

Each device normally requires a separate chip-select signal.

```text
                  BeagleBone
                      |
       +--------------+--------------+
       |              |              |
      MOSI           MISO           SCLK
       |              |              |
       +--------------+--------------+
                      |
          +-----------+-----------+
          |           |           |
         CS0         CS1         CS2
          |           |           |
          v           v           v
       Device 0    Device 1    Device 2
```

---

# 10. Multiple SPI Device Wiring

```text
                 MOSI --------------------+
                 MISO --------------------+
                 SCLK --------------------+
                                          |
              +---------------------------+
              |
       +------+-------+
       |              |
      CS0            CS1
       |              |
       v              v
   SPI Device 0   SPI Device 1
```

Example:

```text
CS0 → SPI Flash
CS1 → SPI ADC
CS2 → SPI Display
```

Each device can use the same MOSI/MISO/SCLK bus but has its own CS.

---

# 11. Chip Select

Chip Select determines which SPI device is active.

Conceptually:

```text
CS0 = LOW
CS1 = HIGH
CS2 = HIGH

       |
       v

Only Device 0 communicates
```

Another transaction:

```text
CS0 = HIGH
CS1 = LOW
CS2 = HIGH

       |
       v

Only Device 1 communicates
```

---

# 12. SPI Chip Select Polarity

Most SPI devices use active-low chip select:

```text
CS = 0 → Device selected

CS = 1 → Device not selected
```

Always verify the device datasheet.

Some devices may support different chip-select behavior.

---

# 13. SPI Clock

The SPI master generates SCLK.

Example:

```text
SCLK

HIGH       +---+   +---+   +---+
           |   |   |   |   |   |
LOW  ------+   +---+   +---+   +---
```

Each clock cycle transfers data according to the configured SPI mode.

---

# 14. SPI Clock Frequency

Common SPI frequencies range from low kHz values to many MHz depending
on the controller, board, wiring, and peripheral.

Examples:

```text
1 MHz
5 MHz
10 MHz
20 MHz
```

Do not assume that a device supports an arbitrary SPI clock.

Always check the peripheral datasheet.

---

# 15. SPI Mode

SPI has four standard modes based on:

```text
CPOL → Clock Polarity
CPHA → Clock Phase
```

| SPI Mode | CPOL | CPHA |
| -------- | ---: | ---: |
| Mode 0   |    0 |    0 |
| Mode 1   |    0 |    1 |
| Mode 2   |    1 |    0 |
| Mode 3   |    1 |    1 |

The SPI device datasheet specifies which mode should be used.

---

# 16. SPI Mode 0

Conceptually:

```text
CPOL = 0
CPHA = 0

Clock idle = LOW
Data sampled on first clock edge
```

Waveform:

```text
SCLK  __|‾|__|‾|__|‾|__
         ^    ^    ^
       Sample
```

The exact sampling edge should be confirmed from the device datasheet.

---

# 17. SPI Full-Duplex Transfer

SPI can transmit and receive simultaneously.

```text
Master                    Slave

MOSI  ------------------> Data

MISO  <------------------ Data

SCLK  ------------------> Clock

CS    ------------------> Select
```

Example:

```text
Master sends:

0x9A

Slave sends:

0x35
```

Both transfers happen during the same clocked transaction.

---

# 18. SPI Device Tree

Project Device Tree files:

```text
beaglebone-black/
└── device-tree/
    └── spi/
        ├── bbb-spi.dts
        ├── bbb-spi.dtsi
        └── README.md
```

Conceptual flow:

```text
bbb-spi.dts
      |
      v
Pinmux Configuration
      |
      v
SPI Controller
      |
      v
Linux SPI Controller Driver
      |
      v
Linux SPI Framework
      |
      v
SPI Device Driver
      |
      v
SPI Peripheral
```

---

# 19. SPI Device Tree Configuration

The Device Tree normally describes:

```text
SPI controller
SPI bus
Pinmux
Chip select
SPI frequency
SPI mode
SPI child device
Compatible string
Interrupt GPIO
```

Conceptually:

```text
SPI Controller
      |
      +---- SPI Device @ CS0
      |
      +---- SPI Device @ CS1
```

---

# 20. SPI Pinmux

The physical header pins must be configured for SPI.

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

The selected pin function must match the intended SPI configuration.

---

# 21. Check SPI Device

After booting Linux:

```bash
ls /dev/spidev*
```

Possible output:

```text
/dev/spidev0.0
/dev/spidev0.1
```

The exact device names depend on kernel configuration, Device Tree,
controller numbering, and enabled chip selects.

---

# 22. Check SPI Kernel Logs

Use:

```bash
dmesg | grep -i spi
```

You can also check:

```bash
dmesg | grep -i spidev
```

This can help determine whether the SPI controller and userspace SPI
interface have initialized.

---

# 23. Check SPI Controller

Depending on the kernel:

```bash
ls -l /sys/class/spi_master/
```

Possible output:

```text
spi0
spi1
```

The exact controller numbering depends on the board configuration.

---

# 24. Install SPI Tools

For userspace testing, a commonly used utility is `spidev_test`.

If available in your kernel/tools package:

```bash
spidev_test
```

or build the Linux SPI test utility from the kernel/tools source tree.

---

# 25. SPI Test Using spidev_test

Example:

```bash
sudo ./spidev_test -D /dev/spidev0.0
```

You can configure parameters such as:

```text
Device
Speed
Bits per word
Delay
SPI mode
Transfer length
```

Example:

```bash
sudo ./spidev_test \
    -D /dev/spidev0.0 \
    -s 1000000 \
    -v
```

This uses:

```text
Device = /dev/spidev0.0
Speed  = 1 MHz
```

Always verify that the selected device supports the configured speed and
mode.

---

# 26. SPI Loopback Test

A useful basic hardware test is SPI loopback.

Connect:

```text
MOSI --------+
             |
             +-------- MISO
```

Also connect:

```text
SCLK → SCLK
CS   → CS
GND  → GND
```

Circuit:

```text
              BeagleBone Black

             MOSI --------+
                           |
                           |
                           +------ MISO

             SCLK ---------------- SCLK

             CS ------------------ CS

             GND ----------------- GND
```

The transmitted data should be received back.

---

# 27. SPI Loopback Test Flow

```text
Application
    |
    v
SPI Driver
    |
    v
MOSI
    |
    v
Loopback Wire
    |
    v
MISO
    |
    v
SPI Driver
    |
    v
Application
```

Example:

```text
TX = 0xAA

RX = 0xAA
```

If:

```text
TX != RX
```

debug:

```text
MOSI
MISO
SCLK
CS
SPI mode
frequency
```

---

# 28. SPI Loopback Wiring Table

| Signal | Connection |
| ------ | ---------- |
| MOSI   | MISO       |
| MISO   | MOSI       |
| SCLK   | SCLK       |
| CS     | CS         |
| GND    | GND        |

Do not connect VCC to another output pin.

---

# 29. SPI Flash Wiring

Example SPI NOR Flash:

```text
BeagleBone          SPI Flash

MOSI  ------------> SI / MOSI

MISO  <------------ SO / MISO

SCLK  ------------> CLK

CS    ------------> CS#

GND   ------------> GND

VCC   ------------> VCC
```

Some Flash devices also have:

```text
WP#
HOLD#
RESET#
```

These must be handled according to the Flash datasheet and board
design.

---

# 30. SPI ADC Wiring

Example:

```text
BeagleBone          SPI ADC

MOSI  ------------> DIN

MISO  <------------ DOUT

SCLK  ------------> SCLK

CS    ------------> CS

GND   ------------> GND

VCC   ------------> VCC
```

The ADC may also have:

```text
CONVST
DRDY
IRQ
```

which may connect to GPIOs.

---

# 31. SPI Sensor Wiring

Typical sensor:

```text
Sensor
+----------------+
| VCC ---------- +---- 3.3V
| GND ---------- +---- GND
| MOSI --------- +---- BBB MOSI
| MISO --------- +---- BBB MISO
| SCLK --------- +---- BBB SCLK
| CS ----------- +---- BBB CS
+----------------+
```

Some sensors also provide:

```text
INT
RESET
```

which can be connected to suitable GPIO pins.

---

# 32. SPI Display Wiring

Example:

```text
BeagleBone              Display

MOSI -----------------> DIN

SCLK -----------------> CLK

CS   -----------------> CS

GPIO -----------------> DC

GPIO -----------------> RESET

GND  -----------------> GND

VCC  -----------------> VCC
```

Display controllers commonly use additional control signals such as
D/C and RESET.

---

# 33. SPI Interrupt

An SPI device may generate an interrupt when data is ready.

```text
SPI Device
    |
    | INT
    v
BeagleBone GPIO
    |
    v
Linux IRQ
    |
    v
SPI Device Driver
```

The SPI data itself still travels through:

```text
MOSI
MISO
SCLK
CS
```

---

# 34. SPI Voltage Compatibility

Before connecting an SPI peripheral, verify:

```text
[ ] VCC
[ ] I/O voltage
[ ] MOSI voltage
[ ] MISO voltage
[ ] SCLK voltage
[ ] CS voltage
```

Do not connect a 5 V SPI signal directly to a BeagleBone input unless
the hardware explicitly supports it.

If required, use an appropriate level shifter.

---

# 35. SPI Level Shifter

For different voltage domains:

```text
BeagleBone
  3.3 V SPI
      |
      v
+----------------+
| SPI Level      |
| Shifter        |
+----------------+
      |
      v
Peripheral
  1.8 V / 5 V
```

The level-shifting device must support the required SPI direction and
speed.

---

# 36. SPI Signal Integrity

At higher SPI frequencies, wiring quality becomes important.

Check:

```text
[ ] Short wires
[ ] Good ground connection
[ ] Proper signal routing
[ ] Appropriate voltage levels
[ ] Correct termination where required
[ ] Clean power supply
```

Long jumper wires can produce:

```text
Ringing
Overshoot
Undershoot
Crosstalk
Timing errors
```

---

# 37. SPI Logic Analyzer

A logic analyzer is highly recommended for SPI debugging.

Connect:

```text
Analyzer CH0 → MOSI
Analyzer CH1 → MISO
Analyzer CH2 → SCLK
Analyzer CH3 → CS
Analyzer GND → GND
```

Example:

```text
BeagleBone             Logic Analyzer

MOSI ----------------> CH0

MISO ----------------> CH1

SCLK ----------------> CH2

CS   ----------------> CH3

GND  ----------------> GND
```

---

# 38. Expected SPI Waveform

Conceptually:

```text
CS

HIGH  ----------------+
                      |
LOW                   +----------------------+
                      |                      |
                      +----------------------+


SCLK

      __|‾|__|‾|__|‾|__|‾|__|‾|__|‾|__


MOSI

      ___|‾‾|____|‾|____|‾‾‾|____________


MISO

      ______|‾|______|‾‾|____|‾|________
```

The exact waveform depends on:

```text
SPI mode
Data
Clock frequency
Device protocol
```

---

# 39. SPI Transaction

Typical transaction:

```text
CS LOW
   |
   v
Send command
   |
   v
Send register/address
   |
   v
Read/write data
   |
   v
CS HIGH
```

Example:

```text
CS ↓
    |
    +--> Command
    |
    +--> Address
    |
    +--> Data
    |
CS ↑
```

---

# 40. SPI Read Transaction

Typical read:

```text
Master
   |
   | Command
   v
Slave
   |
   | Data
   v
Master
```

Conceptually:

```text
CS LOW

MOSI: COMMAND → ADDRESS → DUMMY
MISO: ------- → ------- → DATA

CS HIGH
```

The exact transaction depends on the peripheral datasheet.

---

# 41. SPI Write Transaction

Typical write:

```text
CS LOW

MOSI:
COMMAND → ADDRESS → DATA

MISO:
STATUS / DON'T CARE

CS HIGH
```

The exact transaction is device-specific.

---

# 42. SPI Driver Flow

For a Linux SPI device driver:

```text
Device Tree
     |
     v
SPI Controller
     |
     v
SPI Controller Driver
     |
     v
Linux SPI Core
     |
     v
SPI Device
     |
     v
SPI Client Driver
     |
     v
Hardware
```

The driver typically performs:

```text
probe()
   |
   +--> Configure device
   |
   +--> SPI transfers
   |
   +--> Register interface
   |
   +--> Handle interrupts
```

---

# 43. SPI Driver Communication

Conceptually:

```text
SPI Driver
    |
    +---- spi_write()
    |
    +---- spi_read()
    |
    +---- spi_write_then_read()
    |
    +---- spi_sync()
    |
    +---- spi_async()
```

The exact API depends on the Linux kernel driver implementation.

---

# 44. SPI Device Tree Overlay

Project overlay:

```text
beaglebone-black/
└── device-tree/
    └── overlays/
        └── bbb-spi-overlay.dts
```

Conceptual flow:

```text
SPI Overlay
     |
     v
Pinmux
     |
     v
SPI Controller
     |
     v
Chip Select
     |
     v
SPI Device
```

---

# 45. SPI Debugging Flow

```text
                    SPI Failure
                        |
                        v
                 /dev/spidev* exists?
                    /          \
                  NO            YES
                  |              |
                  v              v
          Check Device Tree   Run SPI test
          Check pinmux           |
          Check kernel            v
                            Device responds?
                              /       \
                            NO         YES
                            |           |
                            v           v
                      Check wiring   Test driver
                      Check CS       Test data
                      Check mode
                      Check clock
```

---

# 46. SPI Device Not Detected

Unlike I2C, SPI does not have a universal bus-discovery mechanism.

Therefore:

```text
i2cdetect
```

has no direct SPI equivalent.

The device must generally be known/configured through:

```text
Device Tree
SPI driver
spidev
Application
```

Then communication is tested using the device protocol.

---

# 47. `/dev/spidev*` Missing

Check:

```text
[ ] SPI controller enabled
[ ] SPI pinmux
[ ] Device Tree
[ ] SPI driver
[ ] spidev support
[ ] SPI chip-select configuration
```

Run:

```bash
ls /dev/spidev*
```

and:

```bash
dmesg | grep -i spi
```

---

# 48. SPI Communication Failure

Check:

```text
[ ] MOSI
[ ] MISO
[ ] SCLK
[ ] CS
[ ] GND
[ ] VCC
[ ] SPI mode
[ ] Clock frequency
[ ] Bits per word
[ ] CS polarity
[ ] Device command sequence
```

---

# 49. MOSI Works but MISO Does Not

Possible causes:

```text
[ ] MISO wiring problem
[ ] Incorrect device mode
[ ] Device not selected
[ ] Device not powered
[ ] Incorrect command
[ ] Device requires dummy clocks
[ ] Device is not configured for SPI
```

Use a logic analyzer to determine whether the slave is actually driving
MISO.

---

# 50. CS Problem

If CS never goes LOW:

```text
Check:
[ ] Device Tree
[ ] Chip-select configuration
[ ] Correct SPI device
[ ] Controller configuration
```

Expected:

```text
Idle:

CS = HIGH

Transaction:

CS = LOW
   |
   +--> Clock/data transfer
   |
CS = HIGH
```

---

# 51. Wrong SPI Mode

If the device responds with incorrect data:

```text
Check:

CPOL
CPHA
SPI mode
```

Example:

```text
Mode 0
Mode 1
Mode 2
Mode 3
```

The peripheral datasheet should be treated as the authoritative source.

---

# 52. Wrong Clock Frequency

If communication works at a low speed but fails at a high speed:

```text
Possible causes:

[ ] Peripheral maximum SPI clock exceeded
[ ] Wiring too long
[ ] Signal integrity problems
[ ] Incorrect voltage levels
[ ] Timing violation
```

Start at a conservative frequency and increase gradually.

---

# 53. SPI Test Procedure

## Step 1 — Select SPI Controller

Check:

```text
hardware/pinout/spi_pin_map.md
```

## Step 2 — Verify Pinmux

Configure:

```text
MOSI
MISO
SCLK
CS
```

## Step 3 — Wire Peripheral

```text
MOSI → MOSI
MISO → MISO
SCLK → SCLK
CS   → CS
GND  → GND
VCC  → VCC
```

## Step 4 — Boot Linux

## Step 5 — Check SPI Device

```bash
ls /dev/spidev*
```

## Step 6 — Check Logs

```bash
dmesg | grep -i spi
```

## Step 7 — Run Loopback

Connect:

```text
MOSI → MISO
```

## Step 8 — Test Real Device

Configure:

```text
SPI mode
Frequency
Bits per word
CS
```

## Step 9 — Verify Data

## Step 10 — Capture With Logic Analyzer

---

# 54. SPI Loopback Test Checklist

```text
[ ] MOSI connected to MISO
[ ] SCLK connected
[ ] CS connected
[ ] GND connected
[ ] SPI device exists
[ ] Correct SPI mode
[ ] Correct speed
[ ] TX data configured
[ ] RX data verified
```

Expected:

```text
TX = 0x55
RX = 0x55
```

or:

```text
TX = 0xAA
RX = 0xAA
```

---

# 55. SPI Wiring Checklist

```text
[ ] SPI controller selected
[ ] MOSI pin verified
[ ] MISO pin verified
[ ] SCLK pin verified
[ ] CS pin verified
[ ] Pinmux configured
[ ] VCC verified
[ ] GND connected
[ ] Voltage levels verified
[ ] SPI mode verified
[ ] SPI frequency verified
[ ] Chip-select polarity verified
[ ] /dev/spidev* checked
[ ] Loopback test completed
[ ] Real peripheral tested
[ ] Logic analyzer test completed
```

---

# 56. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── spi_pin_map.md
│   │
│   └── wiring/
│       └── spi_wiring.md
│
├── hardware/
│   └── schematics/
│       └── spi/
│           └── spi_test_circuit.md
│
├── device-tree/
│   ├── spi/
│   │   ├── bbb-spi.dts
│   │   ├── bbb-spi.dtsi
│   │   └── README.md
│   │
│   └── overlays/
│       └── bbb-spi-overlay.dts
│
├── drivers/
│   └── spi/
│       └── README.md
│
└── tests/
    └── spi/
        ├── spi_loopback_test.sh
        ├── spi_device_test.sh
        └── README.md
```

---

# 57. Complete SPI Hardware Flow

```text
                         User Application
                                |
                                v
                         Linux SPI Framework
                                |
                                v
                      SPI Controller Driver
                                |
                                v
                         AM335x SPI HW
                                |
                                v
                         Pin Multiplexer
                                |
          +---------------------+---------------------+
          |                     |                     |
         MOSI                  MISO                  SCLK
          |                     |                     |
          +---------------------+---------------------+
                                |
                         Chip Select (CS)
                                |
                                v
                         SPI Peripheral
```

---

# 58. Final Test Objective

The objective of this wiring test is to validate the complete SPI path:

```text
SPI Application
      ↓
Linux SPI Framework
      ↓
SPI Controller Driver
      ↓
AM335x SPI Hardware
      ↓
Pinmux
      ↓
MOSI / MISO / SCLK / CS
      ↓
SPI Peripheral
```

Recommended validation sequence:

```text
1. Verify SPI pin mapping
2. Verify Device Tree
3. Verify pinmux
4. Check SPI controller
5. Check /dev/spidev*
6. Perform SPI loopback test
7. Verify SPI mode
8. Verify SPI clock
9. Connect actual SPI peripheral
10. Verify CS
11. Verify MOSI/MISO data
12. Capture transaction with logic analyzer
13. Validate device driver
```

> **Important:** SPI electrical requirements vary between peripherals.
> Always verify the peripheral's supply voltage, I/O voltage, maximum
> clock frequency, SPI mode, chip-select polarity, and timing
> requirements before connecting it to the BeagleBone Black.

````

**File location:**

```text
beaglebone-black/hardware/wiring/spi_wiring.md
````

