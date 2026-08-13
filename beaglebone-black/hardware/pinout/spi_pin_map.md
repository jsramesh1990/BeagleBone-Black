# BeagleBone Black SPI Pin Map

## 1. Overview

The **BeagleBone Black** uses the TI AM335x processor, which provides
multiple **McSPI (Multi-Channel SPI)** controllers.

SPI is commonly used for:

* ADCs
* DACs
* Displays
* Flash memory
* Sensors
* IMUs
* GPIO expanders
* Ethernet controllers
* Other high-speed peripherals

The basic Linux architecture is:

```text
User Application
       |
       v
   SPI Device
       |
       v
   SPI Core
       |
       v
SPI Controller Driver
       |
       v
   AM335x McSPI
       |
       v
 MOSI / MISO / CLK / CS
       |
       v
   SPI Peripheral
```

---

# 2. SPI Signals

A standard SPI bus contains four main signals:

| Signal | Meaning              |
| ------ | -------------------- |
| SCLK   | Serial Clock         |
| MOSI   | Master Out, Slave In |
| MISO   | Master In, Slave Out |
| CS     | Chip Select          |

Additional signal:

```text
GND
```

Typical connection:

```text
BeagleBone Black             SPI Device
----------------             ----------

SCLK ----------------------> SCLK

MOSI ----------------------> MOSI

MISO <---------------------- MISO

CS   ----------------------> CS

GND  ---------------------- GND
```

---

# 3. BeagleBone Black SPI Interfaces

The AM335x provides McSPI controllers.

Commonly used BeagleBone Black SPI interfaces include:

```text
SPI0
SPI1
```

The expansion headers expose SPI signals through multiplexed pins.

---

# 4. SPI0 Pin Mapping

A commonly used SPI0 configuration is:

| SPI Signal     | Header Pin | Function      |
| -------------- | ---------: | ------------- |
| SPI0_SCLK      |      P9.22 | SPI Clock     |
| SPI0_D0 / MISO |      P9.29 | SPI Receive   |
| SPI0_D1 / MOSI |      P9.18 | SPI Transmit  |
| SPI0_CS0       |      P9.17 | Chip Select 0 |
| SPI0_CS1       |      P9.28 | Chip Select 1 |

Quick reference:

```text
+-------------+-------------+
| SPI Signal  | BBB Pin     |
+-------------+-------------+
| SPI0_SCLK   | P9.22       |
| SPI0_D0     | P9.29       |
| SPI0_D1     | P9.18       |
| SPI0_CS0    | P9.17       |
| SPI0_CS1    | P9.28       |
+-------------+-------------+
```

---

# 5. SPI1 Pin Mapping

A commonly used SPI1 configuration is:

| SPI Signal | Header Pin | Function    |
| ---------- | ---------: | ----------- |
| SPI1_SCLK  |      P9.31 | SPI Clock   |
| SPI1_D0    |      P9.29 | SPI Data    |
| SPI1_D1    |      P9.30 | SPI Data    |
| SPI1_CS0   |      P9.28 | Chip Select |

However, **SPI1 pin availability and muxing can vary with the board
Device Tree configuration**.

Always verify the active pinmux before wiring an SPI peripheral.

---

# 6. SPI0 Wiring

Example SPI0 device:

```text
                 BeagleBone Black
                +----------------+
                |                |
P9.22 SCLK ---->|----------------|---- SCLK
                |                |
P9.18 MOSI ---->|----------------|---- MOSI
                |                |
P9.29 MISO <----|----------------|---- MISO
                |                |
P9.17 CS0  ---->|----------------|---- CS
                |                |
P9.1  GND  -----|----------------|---- GND
                |                |
                +----------------+
```

---

# 7. SPI Bus Architecture

One SPI master can communicate with multiple SPI slaves.

```text
                       BeagleBone Black
                              |
                         SPI Controller
                              |
                 +------------+------------+
                 |            |            |
                SCLK         MOSI         MISO
                 |            |            |
                 +------------+------------+
                              |
                +-------------+-------------+
                |             |             |
              CS0           CS1           CS2
                |             |             |
                v             v             v
           +---------+   +---------+   +---------+
           | Sensor  |   | Display |   | Flash   |
           +---------+   +---------+   +---------+
```

SCLK, MOSI and MISO are shared.

Each slave normally has its own chip-select.

---

# 8. Chip Select

For example:

```text
SPI0
 |
 +---- CS0 → Sensor
 |
 +---- CS1 → Flash
```

The master selects one device by asserting its CS line.

```text
CS0 = LOW
   |
   v
Sensor selected
```

```text
CS0 = HIGH
   |
   v
Sensor not selected
```

Typical SPI chip-select polarity is active-low.

---

# 9. SPI Data Transfer

SPI is full-duplex.

```text
Master                         Slave
------                         -----

MOSI  ----------------------->

MISO  <-----------------------

SCLK  ----------------------->

CS    ----------------------->
```

During every clock cycle:

```text
Master sends data
       |
       +-------->
                  Slave receives

Master receives data
       <--------+
                  Slave sends
```

---

# 10. SPI Clock

SPI clock frequency determines the transfer rate.

Example:

```text
SPI Clock = 1 MHz
```

Waveform:

```text
SCLK
 ┌───┐   ┌───┐   ┌───┐
 │   │   │   │   │   │
─┘   └───┘   └───┘   └──
```

The maximum supported frequency depends on:

* AM335x McSPI
* Peripheral device
* PCB design
* Wiring
* Signal integrity
* Voltage level
* Device timing requirements

---

# 11. SPI Modes

SPI has four standard modes.

| Mode   | CPOL | CPHA |
| ------ | ---: | ---: |
| Mode 0 |    0 |    0 |
| Mode 1 |    0 |    1 |
| Mode 2 |    1 |    0 |
| Mode 3 |    1 |    1 |

The slave device datasheet determines which mode must be configured.

### Mode 0

```text
CPOL = 0
CPHA = 0
```

This is a commonly used configuration.

---

# 12. SPI Device Tree

SPI controllers are enabled through Device Tree.

Conceptually:

```dts
&spi0 {
    status = "okay";
};
```

An SPI device can then be described as a child node:

```dts
&spi0 {
    status = "okay";

    sensor@0 {
        compatible = "mycompany,my-spi-sensor";
        reg = <0>;
        spi-max-frequency = <1000000>;
    };
};
```

Here:

```text
reg = <0>
```

means the device is associated with chip-select 0.

---

# 13. SPI Device Tree Flow

```text
bbb-spi.dts
      |
      v
bbb-spi.dtsi
      |
      v
Device Tree Compiler
      |
      v
      DTB
      |
      v
Linux Kernel
      |
      v
McSPI Controller
      |
      v
SPI Core
      |
      v
SPI Device Driver
```

Project files:

```text
beaglebone-black/
└── device-tree/
    └── spi/
        ├── bbb-spi.dts
        ├── bbb-spi.dtsi
        └── README.md
```

---

# 14. SPI Pinmux

SPI pins are multiplexed with other AM335x functions.

Conceptually:

```text
P9.22
  |
  +---- GPIO
  |
  +---- SPI0_SCLK
  |
  +---- Alternate Function
```

Device Tree selects the required SPI function.

For example:

```text
P9.22 → SPI0_SCLK
P9.18 → SPI0_D1
P9.29 → SPI0_D0
P9.17 → SPI0_CS0
```

Do not configure these same physical pins for conflicting peripherals.

---

# 15. Linux SPI Architecture

```text
                 User Application
                        |
                        v
                  /dev/spidevX.Y
                        |
                        v
                    SPI Core
                        |
                        v
                 SPI Controller
                    Driver
                        |
                        v
                    AM335x
                     McSPI
                        |
                        v
                SPI Physical Bus
                        |
                        v
                  SPI Peripheral
```

---

# 16. SPI Device Node

When `spidev` is enabled, a device may appear as:

```text
/dev/spidev0.0
```

Meaning:

```text
SPI Bus       : 0
Chip Select   : 0
```

Another example:

```text
/dev/spidev0.1
```

means:

```text
SPI Bus       : 0
Chip Select   : 1
```

Check:

```bash
ls -l /dev/spidev*
```

---

# 17. SPI Kernel Configuration

Check SPI support:

```bash
zcat /proc/config.gz | grep CONFIG_SPI
```

Typical options include:

```text
CONFIG_SPI=y
CONFIG_SPI_MASTER=y
CONFIG_SPI_SPIDEV=m
```

The exact configuration depends on the kernel/BSP.

---

# 18. SPI Device Driver

For a dedicated SPI peripheral, a kernel driver can register an
`spi_driver`.

Conceptual structure:

```c
static struct spi_driver my_spi_driver = {
    .driver = {
        .name = "my_spi_device",
    },
    .probe = my_spi_probe,
    .remove = my_spi_remove,
};
```

Probe:

```c
static int my_spi_probe(struct spi_device *spi)
{
    dev_info(&spi->dev, "SPI device detected\n");

    return 0;
}
```

---

# 19. SPI Probe Flow

```text
Linux Boot
    |
    v
Device Tree
    |
    v
SPI Controller Enabled
    |
    v
McSPI Driver Probe
    |
    v
SPI Bus Registered
    |
    v
SPI Device Created
    |
    v
Driver Matching
    |
    v
SPI Driver probe()
    |
    v
Device Initialization
```

---

# 20. SPI Data Transfer API

Kernel SPI drivers can use APIs such as:

```c
spi_write()
spi_read()
spi_sync()
spi_async()
spi_write_then_read()
```

For complex transfers:

```c
struct spi_transfer
```

and:

```c
struct spi_message
```

can be used.

Conceptually:

```text
spi_message
     |
     +---- spi_transfer
     |
     +---- spi_transfer
     |
     +---- spi_transfer
```

---

# 21. SPI User-Space Testing

If `spidev` is enabled:

```bash
ls /dev/spidev*
```

Example:

```text
/dev/spidev0.0
```

A common test utility is:

```bash
spidev_test
```

Depending on the BSP, it may need to be built separately.

Example:

```bash
./spidev_test -D /dev/spidev0.0
```

---

# 22. SPI Loopback Test

A simple hardware loopback connects:

```text
MOSI ↔ MISO
```

Example:

```text
BeagleBone
     |
     +---- MOSI ----+
     |              |
     |              |
     +---- MISO <---+
```

Then transmit:

```text
0xAA
0x55
0x12
0x34
```

Expected:

```text
TX: AA 55 12 34
RX: AA 55 12 34
```

This tests the SPI controller and physical data path.

---

# 23. SPI Loopback Wiring

For SPI0:

```text
P9.18 MOSI --------+
                   |
                   +-------- P9.29 MISO

P9.22 SCLK --------> SCLK

P9.17 CS0 ---------> CS

P9.1 GND ----------> GND
```

Do **not** connect MOSI and MISO directly when a real SPI slave is
connected, because the slave also drives MISO.

---

# 24. SPI Logic Analyzer Test

A logic analyzer is very useful for SPI debugging.

Connect:

```text
Logic Analyzer      BeagleBone

CH0 --------------> SCLK
CH1 --------------> MOSI
CH2 --------------> MISO
CH3 --------------> CS
GND --------------> GND
```

Observe:

```text
CS
───────┐                    ┌────────
       └────────────────────┘

SCLK
   ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐
───┘ └─┘ └─┘ └─┘ └─┘ └───

MOSI
───bit7──bit6──bit5──bit4──

MISO
───bit7──bit6──bit5──bit4──
```

This allows you to verify:

* Clock frequency
* SPI mode
* CS timing
* MOSI data
* MISO data
* Bit order
* Transfer length

---

# 25. SPI Bit Order

SPI devices may use:

```text
MSB first
```

or:

```text
LSB first
```

Most devices use MSB-first, but always check the device datasheet.

Example MSB-first:

```text
0xA5

10100101
^
MSB
```

---

# 26. SPI Chip Select Timing

Typical transfer:

```text
CS
 ─────┐                    ┌────
      │                    │
      └────────────────────┘

SCLK     ┌─┐ ┌─┐ ┌─┐ ┌─┐
─────────┘ └─┘ └─┘ └─┘ └──

MOSI     D7  D6  D5  D4 ...
```

The peripheral normally considers the transfer active while CS is
asserted.

---

# 27. SPI Frequency Configuration

Device Tree may specify:

```dts
spi-max-frequency = <1000000>;
```

This means:

```text
Maximum SPI frequency = 1 MHz
```

Example:

```dts
sensor@0 {
    compatible = "mycompany,my-sensor";
    reg = <0>;
    spi-max-frequency = <10000000>;
};
```

This requests up to:

```text
10 MHz
```

The actual frequency may be constrained by the controller and driver.

---

# 28. SPI Debugging

Check SPI devices:

```bash
ls /sys/class/spi_master/
```

Check SPI devices:

```bash
ls /sys/bus/spi/devices/
```

Example:

```text
spi0.0
```

Check kernel messages:

```bash
dmesg | grep -i spi
```

Check SPI device nodes:

```bash
ls -l /dev/spidev*
```

Check pinmux:

```bash
sudo cat /sys/kernel/debug/pinctrl/*/pinmux-pins
```

---

# 29. Common SPI Problems

## `/dev/spidev*` does not exist

Check:

```bash
ls /dev/spidev*
```

Then:

```bash
dmesg | grep -i spi
```

Possible causes:

```text
1. SPI controller disabled
2. Device Tree not configured
3. SPI driver missing
4. spidev not enabled
5. Incorrect chip-select configuration
```

---

# 30. SPI Device Not Responding

Check:

```text
1. SCLK
2. MOSI
3. MISO
4. CS
5. GND
6. Device power
7. SPI mode
8. SPI frequency
9. Bit order
10. Device address/register protocol
```

Use a logic analyzer to determine whether the master is actually
generating the expected waveform.

---

# 31. SPI and GPIO Conflict

SPI pins are multiplexed.

For example:

```text
P9.17
   |
   +---- GPIO
   |
   +---- SPI0_CS0
   |
   +---- Alternate Function
```

When configured for SPI:

```text
P9.17
  |
  v
SPI0_CS0
```

The SPI controller controls chip select rather than normal GPIO
software.

---

# 32. SPI and I2C Conflict

Some header pins can have multiple functions.

Example concept:

```text
Physical Pin
     |
     +---- GPIO
     |
     +---- I2C
     |
     +---- SPI
     |
     +---- PWM
```

The Device Tree and pinmux determine which function is active.

For this project, keep a clear peripheral allocation table so that
GPIO, I2C, SPI, UART, CAN and PWM do not unintentionally claim the same
pins.

---

# 33. SPI Device Tree Overlay

For overlay-based configurations:

```text
beaglebone-black/
└── device-tree/
    └── overlays/
        ├── bbb-gpio-overlay.dts
        ├── bbb-i2c-overlay.dts
        ├── bbb-spi-overlay.dts
        └── bbb-uart-overlay.dts
```

A dedicated SPI overlay can contain:

```text
1. SPI controller enable
2. Pinmux configuration
3. Chip-select configuration
4. SPI child device
5. Device properties
```

---

# 34. SPI Testing Checklist

```text
[ ] SPI controller enabled
[ ] Device Tree configured
[ ] Pinmux configured
[ ] SPI driver loaded
[ ] SPI bus detected
[ ] Correct chip select selected
[ ] /dev/spidevX.Y available
[ ] SPI mode verified
[ ] SPI frequency verified
[ ] MOSI tested
[ ] MISO tested
[ ] SCLK tested
[ ] CS tested
[ ] Loopback tested
[ ] SPI peripheral tested
[ ] Logic analyzer tested
[ ] Kernel logs checked
```

---

# 35. SPI Pin Quick Reference

## SPI0

```text
+-------------+-------------+----------------+
| SPI Signal  | BBB Pin     | AM335x Function|
+-------------+-------------+----------------+
| SPI0_SCLK   | P9.22       | SPI Clock      |
| SPI0_D1     | P9.18       | MOSI           |
| SPI0_D0     | P9.29       | MISO           |
| SPI0_CS0    | P9.17       | Chip Select 0  |
| SPI0_CS1    | P9.28       | Chip Select 1  |
+-------------+-------------+----------------+
```

---

# 36. SPI1

A commonly used SPI1 signal set is:

```text
+-------------+-------------+
| SPI Signal  | BBB Pin     |
+-------------+-------------+
| SPI1_SCLK   | P9.31       |
| SPI1_D0     | P9.29       |
| SPI1_D1     | P9.30       |
| SPI1_CS0    | P9.28       |
+-------------+-------------+
```

**Verify SPI1 pinmux against the exact BeagleBone Black Device Tree/BSP
before wiring**, because these pins are multiplexed and can conflict
with other functions.

---

# 37. SPI Project Structure

```text
beaglebone-black/
│
├── hardware/
│   └── pinout/
│       └── spi_pin_map.md
│
├── device-tree/
│   ├── spi/
│   │   ├── bbb-spi.dts
│   │   ├── bbb-spi.dtsi
│   │   └── README.md
│   │
│   └── overlays/
│       ├── bbb-spi-overlay.dts
│       └── README.md
│
├── drivers/
│   └── spi/
│       ├── README.md
│       └── ...
│
└── tests/
    └── spi/
        ├── spi_loopback_test.c
        ├── spi_device_test.c
        └── README.md
```

---

# 38. Complete SPI Architecture

```text
                         BeagleBone Black
                                |
                                v
                         P8 / P9 Headers
                                |
                                v
                              Pinmux
                                |
                                v
                           Device Tree
                                |
                                v
                         AM335x McSPI
                                |
                                v
                          SPI Controller
                             Driver
                                |
                                v
                            SPI Core
                                |
              +-----------------+-----------------+
              |                 |                 |
             SCLK              MOSI              MISO
              |                 |                 |
              +-----------------+-----------------+
                                |
                +---------------+---------------+
                |               |               |
               CS0             CS1             CS2
                |               |               |
                v               v               v
             Sensor           Flash          Display
```

---

# 39. Complete SPI Development Flow

```text
Hardware
   |
   v
SPI Pin Mapping
   |
   v
Device Tree
   |
   v
Pinmux
   |
   v
McSPI Controller
   |
   v
Linux SPI Driver
   |
   v
SPI Core
   |
   +----------------------+
   |                      |
   v                      v
spidev                 Custom SPI Driver
   |                      |
   v                      v
User Application       Kernel Application
   |
   v
SPI Peripheral
```

## Project File

```text
beaglebone-black/hardware/pinout/spi_pin_map.md
```

