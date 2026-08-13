# `spi_test_circuit.md`

````markdown
# BeagleBone Black SPI Test Circuit

## 1. Overview

This document describes a hardware test circuit for validating SPI
communication on the BeagleBone Black.

SPI (Serial Peripheral Interface) is a synchronous serial communication
protocol commonly used with:

- ADCs
- DACs
- Flash memory
- Displays
- Sensors
- IMUs
- GPIO expanders
- Communication controllers

The complete path is:

```text
User Application
       |
       v
/dev/spidevX.Y
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
SPI Pins
       |
       v
SPI Slave Device
````

---

# 2. SPI Test Objectives

This test validates:

* SPI MOSI
* SPI MISO
* SPI CLK
* SPI CS
* SPI Device Tree
* SPI pinmux
* Linux SPI controller driver
* `/dev/spidev*`
* SPI mode
* SPI clock frequency
* SPI data transmission
* SPI data reception
* Full-duplex communication

---

# 3. Required Components

For a basic SPI test:

```text
1 × BeagleBone Black
1 × SPI sensor / EEPROM / ADC / DAC
Breadboard
Jumper wires
External power supply if required
Logic analyzer or oscilloscope
```

A simple SPI sensor or SPI ADC is recommended for a hardware test.

---

# 4. SPI Signals

SPI normally uses four main signals:

```text
MOSI = Master Out Slave In
MISO = Master In Slave Out
SCLK = Serial Clock
CS   = Chip Select
```

Basic architecture:

```text
                 BeagleBone Black
                    SPI Master
                       |
          +------------+------------+
          |            |            |
         MOSI         MISO         SCLK
          |            |            |
          v            v            v
       +-----------------------------+
       |         SPI Slave           |
       +-----------------------------+
                    ^
                    |
                   CS
                    |
                    |
              BeagleBone CS
```

---

# 5. Complete SPI Test Circuit

```text
             BeagleBone Black
             SPI Master
                  |
                  |
       +----------+----------+
       |          |          |
      MOSI       MISO       SCLK
       |          |          |
       |          |          |
       v          v          v
   +-----------------------------+
   |                             |
   |        SPI DEVICE           |
   |                             |
   | MOSI  <---------------------+
   | MISO  ---------------------->
   | SCLK  <---------------------+
   | CS    <---------------------+
   | VCC   <---------------------+---- 3.3V
   | GND   <---------------------+---- GND
   |                             |
   +-----------------------------+
```

---

# 6. SPI Wiring

Connect the BeagleBone Black SPI master to the SPI slave:

```text
BBB SPI        SPI Device
--------------------------
MOSI    -----> MOSI
MISO    <----- MISO
SCLK    -----> SCLK
CS      -----> CS
3.3V    -----> VCC
GND     -----> GND
```

The exact header pins should be obtained from:

```text
hardware/pinout/spi_pin_map.md
```

---

# 7. SPI Signal Direction

### MOSI

```text
Master
   |
   v
MOSI
   |
   v
Slave
```

MOSI carries data from the master to the slave.

### MISO

```text
Slave
   |
   v
MISO
   |
   v
Master
```

MISO carries data from the slave to the master.

### SCLK

```text
Master
   |
   v
SCLK
   |
   v
Slave
```

The master generates the SPI clock.

### CS

```text
Master
   |
   v
CS
   |
   v
Slave
```

Chip Select activates the selected slave.

---

# 8. SPI Power

Typical connection:

```text
BBB 3.3V  ----------> SPI Device VCC
BBB GND   ----------> SPI Device GND
```

Verify the SPI device's voltage requirements before connecting it.

Do not connect a 5 V SPI signal directly to a 3.3 V BeagleBone Black
I/O pin.

---

# 9. Multiple SPI Devices

SPI can support multiple slaves.

```text
                    MOSI
BBB ----------------+------------------+
                    |                  |
                    v                  v
                 Slave 1            Slave 2

                    MISO
BBB <---------------+------------------+

                    SCLK
BBB ----------------+------------------+
                    |                  |
                    v                  v
                 Slave 1            Slave 2

                    CS1               CS2
BBB ----------------|-----------------|
                    |                 |
                    v                 v
                 Slave 1            Slave 2
```

MOSI, MISO and SCLK can be shared.

Each slave normally requires its own chip-select.

---

# 10. SPI Architecture

```text
                    User Application
                           |
                           v
                       spidev
                           |
                           v
                    /dev/spidevX.Y
                           |
                           v
                    Linux SPI Core
                           |
                           v
                    SPI Controller
                           |
                           v
                    AM335x SPI HW
                           |
                           v
                    Pinmux / Pins
                           |
                           v
                      SPI Slave
```

---

# 11. Device Tree

The SPI controller and pinmux must be configured through Device Tree.

Project files:

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
Linux SPI Driver
      |
      v
SPI Framework
      |
      v
SPI Device
```

---

# 12. SPI Pin Selection

Select the SPI-capable pins from:

```text
hardware/pinout/spi_pin_map.md
```

Before wiring:

```text
1. Select SPI bus
2. Verify MOSI pin
3. Verify MISO pin
4. Verify SCLK pin
5. Verify CS pin
6. Check pinmux
7. Check peripheral conflicts
8. Verify voltage levels
```

---

# 13. Check SPI Device Node

After booting Linux:

```bash
ls /dev/spidev*
```

Example:

```text
/dev/spidev1.0
```

The exact device node depends on the configured SPI controller,
chip-select, kernel configuration, and Device Tree.

---

# 14. SPI Device Naming

A typical device name is:

```text
/dev/spidev1.0
```

Meaning conceptually:

```text
SPI Bus    = 1
Chip Select = 0
```

Another possible device could be:

```text
/dev/spidev1.1
```

which represents:

```text
SPI Bus    = 1
Chip Select = 1
```

---

# 15. Install SPI Test Utilities

A useful utility is `spidev_test`.

On many Linux distributions it is available as source code in the
Linux kernel tools/examples.

Typical workflow:

```bash
git clone https://github.com/torvalds/linux.git
cd linux/tools/spi
make
```

Then:

```bash
sudo ./spidev_test -D /dev/spidev1.0
```

The exact build location can vary with the kernel source tree.

---

# 16. SPI Loopback Test

A simple SPI software/hardware test can connect MOSI directly to MISO.

```text
BeagleBone Black

MOSI -------------------+
                        |
                        |
                        +---- MISO
```

This is called an SPI loopback.

Data transmitted on MOSI should return through MISO.

---

# 17. SPI Loopback Circuit

```text
             BeagleBone Black

              MOSI
                |
                |
                +----------------+
                                 |
                                 |
              MISO <-------------+
```

Other signals are still available for observation:

```text
SCLK → Oscilloscope / Logic Analyzer
CS   → Oscilloscope / Logic Analyzer
```

For a loopback test, the slave device is not required.

---

# 18. SPI Loopback Architecture

```text
                User Application
                       |
                       v
                  spidev
                       |
                       v
                 SPI Controller
                       |
                       v
                     MOSI
                       |
                       v
                  +--------+
                  | Wire   |
                  +--------+
                       |
                       v
                     MISO
                       |
                       v
                 SPI Controller
                       |
                       v
                User Application
```

---

# 19. SPI Loopback Test

Connect:

```text
MOSI ↔ MISO
```

Then run:

```bash
sudo ./spidev_test -D /dev/spidev1.0
```

A successful loopback should show transmitted data being received
back.

Example concept:

```text
TX:
00 01 02 03 04

RX:
00 01 02 03 04
```

The exact output depends on the `spidev_test` configuration.

---

# 20. SPI Mode

SPI supports four common modes.

```text
Mode 0
CPOL = 0
CPHA = 0

Mode 1
CPOL = 0
CPHA = 1

Mode 2
CPOL = 1
CPHA = 0

Mode 3
CPOL = 1
CPHA = 1
```

The SPI slave's datasheet determines which mode must be used.

---

# 21. SPI Clock

The SPI master generates SCLK.

Example:

```text
SPI Clock = 1 MHz
```

Conceptually:

```text
SCLK
 ┌─┐ ┌─┐ ┌─┐ ┌─┐
 │ │ │ │ │ │ │ │
─┘ └─┘ └─┘ └─┘ └──
```

The maximum supported clock depends on the controller, board,
interconnect, and SPI slave.

---

# 22. SPI Data Transfer

A typical transaction:

```text
CS LOW
   |
   v
Send Command
   |
   v
Send Address
   |
   v
Read / Write Data
   |
   v
CS HIGH
```

Conceptually:

```text
CS    ───────┐________________┌────
             |                |
SCLK         ┌─┐ ┌─┐ ┌─┐ ┌─┐
             | | | | | | | |
             └─┘ └─┘ └─┘ └─┘

MOSI         CMD ADDRESS DATA
```

The exact transaction depends on the SPI device.

---

# 23. SPI Register Read

Many SPI devices use a transaction similar to:

```text
CS LOW
   |
   v
Register Command
   |
   v
Register Address
   |
   v
Read Data
   |
   v
CS HIGH
```

Example concept:

```text
TX:
80 10 00

RX:
00 00 5A
```

The exact command and data format must be obtained from the device
datasheet.

---

# 24. SPI Register Write

Generic structure:

```text
CS LOW
   |
   v
Write Command
   |
   v
Register Address
   |
   v
Register Data
   |
   v
CS HIGH
```

Example:

```text
TX:
00 10 55
```

The exact protocol depends on the SPI slave.

---

# 25. SPI Logic Analyzer Test

Connect a logic analyzer to:

```text
MOSI
MISO
SCLK
CS
GND
```

Example:

```text
BeagleBone Black
       |
       +------ MOSI ------> Logic Analyzer CH1
       |
       +------ MISO ------> Logic Analyzer CH2
       |
       +------ SCLK ------> Logic Analyzer CH3
       |
       +------ CS --------> Logic Analyzer CH4
       |
       +------ GND -------> Logic Analyzer GND
```

Configure the analyzer for:

```text
Protocol = SPI
Clock polarity = device-specific
Clock phase = device-specific
Bit order = device-specific
```

---

# 26. Expected SPI Waveform

Typical SPI transaction:

```text
CS
────────┐                    ┌────────
        │                    │
        └────────────────────┘

SCLK
      ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐
──────┘ └─┘ └─┘ └─┘ └─┘ └──────

MOSI
      <----- DATA ---------->

MISO
      <----- DATA ---------->
```

The exact data sampling edge depends on the selected SPI mode.

---

# 27. SPI Test Procedure

## Step 1 — Power Off

Disconnect board power before changing wiring.

## Step 2 — Connect SPI Device

```text
MOSI → MOSI
MISO → MISO
SCLK → SCLK
CS   → CS
VCC  → appropriate supply
GND  → GND
```

## Step 3 — Verify Pin Mapping

Check:

```text
hardware/pinout/spi_pin_map.md
```

## Step 4 — Boot Linux

## Step 5 — Check Device Node

```bash
ls /dev/spidev*
```

## Step 6 — Perform Loopback Test

Connect:

```text
MOSI ↔ MISO
```

Then run:

```bash
sudo ./spidev_test -D /dev/spidev1.0
```

## Step 7 — Connect Actual SPI Device

Remove the loopback wire and connect the SPI slave.

## Step 8 — Test Device Transaction

Use the appropriate SPI application or driver.

---

# 28. SPI Device Driver Flow

For a real SPI peripheral:

```text
Device Tree
     |
     v
SPI Device Node
     |
     v
SPI Core
     |
     v
SPI Controller Driver
     |
     v
SPI Bus
     |
     v
SPI Slave
     |
     v
Peripheral Driver
     |
     v
User Application
```

---

# 29. SPI Kernel Driver Architecture

```text
+--------------------------------+
| User Application               |
+---------------+----------------+
                |
                v
+--------------------------------+
| Device Driver / spidev         |
+---------------+----------------+
                |
                v
+--------------------------------+
| Linux SPI Core                 |
+---------------+----------------+
                |
                v
+--------------------------------+
| AM335x SPI Controller Driver   |
+---------------+----------------+
                |
                v
+--------------------------------+
| AM335x SPI Hardware            |
+---------------+----------------+
                |
                v
+--------------------------------+
| MOSI / MISO / SCLK / CS        |
+---------------+----------------+
                |
                v
+--------------------------------+
| SPI Slave Device               |
+--------------------------------+
```

---

# 30. SPI Device Tree Concept

A typical SPI setup contains:

```text
SPI Controller
    |
    +-- pinctrl
    |
    +-- status = "okay"
    |
    +-- SPI Slave
          |
          +-- reg = <chip-select>
          |
          +-- compatible = "vendor,device"
```

The exact properties depend on the SPI controller and peripheral.

---

# 31. SPI Debugging

If `/dev/spidev*` does not exist:

```text
SPI Device Missing
       |
       +--> Check Device Tree
       |
       +--> Check SPI controller
       |
       +--> Check pinmux
       |
       +--> Check kernel SPI support
       |
       +--> Check spidev configuration
       |
       +--> Check chip-select
       |
       +--> Check driver binding
```

---

# 32. Check Kernel Logs

Run:

```bash
dmesg | grep -i spi
```

Also:

```bash
dmesg | grep -i pinctrl
```

Check device nodes:

```bash
ls -l /dev/spidev*
```

---

# 33. SPI Hardware Debugging

If the device is not responding:

```text
SPI Communication Failure
        |
        +--> Check VCC
        |
        +--> Check GND
        |
        +--> Check MOSI
        |
        +--> Check MISO
        |
        +--> Check SCLK
        |
        +--> Check CS
        |
        +--> Check SPI mode
        |
        +--> Check clock frequency
        |
        +--> Check bit order
        |
        +--> Check device address/command
        |
        +--> Check pinmux
```

---

# 34. MOSI/MISO Wiring Error

Incorrect:

```text
BBB MOSI -------- MISO Device
BBB MISO -------- MOSI Device
```

Correct:

```text
BBB MOSI -------- MOSI Device
BBB MISO -------- MISO Device
```

However, some breakout boards label signals according to their
perspective, so always verify the peripheral documentation.

---

# 35. Chip Select Test

CS should normally become active before the SPI transaction.

Conceptually:

```text
CS
────────────┐
            │
            └────────────────┐
                             │
                         Transaction
```

If CS is not asserted correctly:

```text
SPI Slave
   |
   v
Does not recognize transaction
```

Check:

```text
[ ] Correct CS
[ ] Correct polarity
[ ] Correct chip-select number
[ ] Correct Device Tree configuration
```

---

# 36. SPI Clock Test

Use a logic analyzer or oscilloscope.

Check:

```text
Frequency
Clock polarity
Clock phase
Signal integrity
```

Example:

```text
Configured:
1 MHz

Measured:
approximately 1 MHz
```

The measured value may differ slightly depending on the controller
clock configuration.

---

# 37. SPI Loopback Checklist

```text
[ ] SPI controller enabled
[ ] SPI pinmux configured
[ ] SPI device node available
[ ] MOSI connected to MISO
[ ] GND connected
[ ] SPI test application available
[ ] Data transmitted
[ ] Data received
[ ] TX = RX verified
```

---

# 38. SPI Physical Device Checklist

```text
[ ] SPI device powered
[ ] VCC correct
[ ] GND connected
[ ] MOSI connected
[ ] MISO connected
[ ] SCLK connected
[ ] CS connected
[ ] Correct SPI mode
[ ] Correct clock frequency
[ ] Correct bit order
[ ] Correct command
[ ] Correct register/address
[ ] Logic analyzer verified
```

---

# 39. SPI Test Example

Example loopback:

```bash
ls /dev/spidev*
```

Expected:

```text
/dev/spidev1.0
```

Connect:

```text
MOSI ↔ MISO
```

Run:

```bash
sudo ./spidev_test -D /dev/spidev1.0
```

Expected concept:

```text
TX = 00 01 02 03 04
RX = 00 01 02 03 04
```

If TX and RX match, the SPI loopback path is functioning.

---

# 40. SPI Sensor Test

For an actual sensor:

```text
                BeagleBone Black
                       |
                       v
                   SPI Master
                       |
        +--------------+--------------+
        |       |       |       |
       MOSI    MISO    SCLK     CS
        |       |       |       |
        +-------+-------+-------+
                |
                v
             SPI Sensor
                |
                v
           Sensor Data
                |
                v
          User Application
```

The application can then read sensor registers through the peripheral
driver or an appropriate userspace interface.

---

# 41. SPI Test Flow

```text
                   SPI Hardware
                        |
                        v
                   SPI Pin Map
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
                    SPI Driver
                        |
                        v
                    SPI Core
                        |
                        v
                 /dev/spidevX.Y
                        |
                        v
                 SPI Transaction
                        |
              +---------+---------+
              |                   |
              v                   v
             MOSI                MISO
              |                   |
              +---------+---------+
                        |
                        v
                     SPI Slave
```

---

# 42. SPI Test Checklist

```text
[ ] SPI-capable pins identified
[ ] P8/P9 header verified
[ ] MOSI verified
[ ] MISO verified
[ ] SCLK verified
[ ] CS verified
[ ] VCC verified
[ ] GND verified
[ ] Pinmux configured
[ ] Device Tree configured
[ ] SPI controller enabled
[ ] SPI driver loaded
[ ] /dev/spidevX.Y available
[ ] Loopback test completed
[ ] SPI device connected
[ ] SPI mode verified
[ ] SPI frequency verified
[ ] SPI data verified
[ ] Logic analyzer test completed
[ ] Kernel logs checked
```

---

# 43. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── spi_pin_map.md
│   │
│   └── schematics/
│       └── spi/
│           └── spi_test_circuit.md
│
├── device-tree/
│   └── spi/
│       ├── bbb-spi.dts
│       ├── bbb-spi.dtsi
│       └── README.md
│
├── drivers/
│   └── spi/
│       └── README.md
│
└── tests/
    └── spi/
        ├── spi_loopback_test.sh
        ├── spi_transfer_test.sh
        └── README.md
```

---

# 44. Complete SPI Bring-Up

```text
                     SPI Hardware
                          |
                          v
                     SPI Pin Map
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
                      SPI Driver
                          |
                          v
                       SPI Core
                          |
                          v
                    SPI Controller
                          |
                          v
                    /dev/spidevX.Y
                          |
                          v
                  SPI Transaction
                          |
              +-----------+-----------+
              |           |           |
              v           v           v
             MOSI        MISO        SCLK
              |           |           |
              +-----------+-----------+
                          |
                          v
                       CS Select
                          |
                          v
                     SPI Device
```

---

# 45. Final Objective

The purpose of this test circuit is to validate the complete SPI
communication path on the BeagleBone Black:

```text
User Application
       ↓
spidev / Peripheral Driver
       ↓
Linux SPI Framework
       ↓
AM335x SPI Controller Driver
       ↓
AM335x SPI Hardware
       ↓
Pinmux
       ↓
MOSI / MISO / SCLK / CS
       ↓
SPI Slave Device
```

The **loopback test** validates the SPI controller and data path.

The **logic-analyzer test** validates the physical SPI waveform.

The **actual SPI-device test** validates the complete hardware,
Device Tree, Linux driver, and peripheral communication path.

> For this BeagleBone Black driver project, perform the SPI bring-up in
> three stages: **loopback → logic-analyzer verification → real SPI
> peripheral**.

````

**File location:**

```text
beaglebone-black/hardware/schematics/spi/spi_test_circuit.md
````

