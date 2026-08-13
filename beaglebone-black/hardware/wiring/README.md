# `beaglebone-black/hardware/wiring/README.md`

````markdown
# BeagleBone Black Wiring Guide

## Overview

This directory contains the hardware wiring documentation for the
BeagleBone Black peripheral and device-driver project.

The purpose of this section is to provide the physical wiring,
signal connections, voltage considerations, and basic hardware
validation procedures required before testing Linux device drivers.

---

## Supported Interfaces

The project covers the following major BeagleBone Black interfaces:

```text
                    BeagleBone Black
                           |
        +------------------+------------------+
        |                  |                  |
       ADC                GPIO               PWM
        |                  |                  |
        +------------------+------------------+
                           |
        +------------------+------------------+
        |                  |                  |
       I2C                SPI                UART
        |                  |                  |
        +------------------+------------------+
                           |
                          CAN
````

---

## Directory Contents

| File             | Description                      |
| ---------------- | -------------------------------- |
| `adc_wiring.md`  | ADC signal and hardware wiring   |
| `can_wiring.md`  | CAN transceiver and bus wiring   |
| `gpio_wiring.md` | GPIO input/output wiring         |
| `i2c_wiring.md`  | I2C SDA/SCL wiring               |
| `pwm_wiring.md`  | PWM output and peripheral wiring |
| `spi_wiring.md`  | SPI MOSI/MISO/SCLK/CS wiring     |
| `uart_wiring.md` | UART TX/RX wiring                |

---

## Peripheral Wiring Overview

### ADC

ADC wiring is used to connect analog signals to the BeagleBone Black
analog input channels.

```text
Analog Sensor
      |
      v
   ADC Input
      |
      v
BeagleBone Black
```

Refer to:

```text
adc_wiring.md
```

---

### CAN

CAN requires a CAN transceiver between the processor and the physical
CAN bus.

```text
BeagleBone Black
       |
       | CAN TX/RX
       v
+----------------+
| CAN Transceiver|
+----------------+
       |
       +---- CANH
       |
       +---- CANL
```

Refer to:

```text
can_wiring.md
```

---

### GPIO

GPIO can be configured as either an input or output.

Input:

```text
External Signal
      |
      v
   GPIO Input
      |
      v
BeagleBone Black
```

Output:

```text
BeagleBone Black
      |
      v
 GPIO Output
      |
      v
 LED / Relay / Sensor
```

Refer to:

```text
gpio_wiring.md
```

---

### I2C

I2C normally uses two signal lines:

```text
SDA → Serial Data
SCL → Serial Clock
```

Typical connection:

```text
BeagleBone Black        I2C Device

SDA ------------------> SDA

SCL ------------------> SCL

GND ------------------> GND
```

I2C devices normally share the same SDA/SCL bus and are selected by
their I2C address.

Refer to:

```text
i2c_wiring.md
```

---

### PWM

PWM generates a digital waveform with configurable frequency and
duty cycle.

```text
BeagleBone Black
      |
      v
  PWM Output
      |
      v
+-------------+
| LED / Motor |
| / Buzzer    |
+-------------+
```

Refer to:

```text
pwm_wiring.md
```

---

### SPI

SPI normally uses:

```text
MOSI
MISO
SCLK
CS
GND
```

Typical connection:

```text
BeagleBone Black        SPI Device

MOSI -----------------> MOSI

MISO <----------------- MISO

SCLK -----------------> SCLK

CS -------------------> CS

GND ------------------> GND
```

Refer to:

```text
spi_wiring.md
```

---

### UART

UART normally uses:

```text
TX
RX
GND
```

Typical connection:

```text
BeagleBone Black        UART Device

TX -------------------> RX

RX <------------------- TX

GND ------------------> GND
```

Refer to:

```text
uart_wiring.md
```

---

# Hardware Wiring Flow

The recommended hardware validation flow is:

```text
                    BeagleBone Black
                           |
                           v
                    Pin Identification
                           |
                           v
                    Device Tree / Pinmux
                           |
                           v
                    Physical Wiring
                           |
                           v
                    Power Verification
                           |
                           v
                    Signal Verification
                           |
                           v
                    Linux Device Detection
                           |
                           v
                    Driver Testing
                           |
                           v
                    Application Testing
```

---

# Pin Mapping

Before connecting any external hardware, always check the corresponding
pin-map documentation:

```text
hardware/pinout/
```

Available pin maps:

```text
adc_pin_map.md
can_pin_map.md
gpio_pin_map.md
i2c_pin_map.md
pwm_pin_map.md
spi_pin_map.md
uart_pin_map.md
```

Do not select pins only based on their physical header number.

The AM335x processor uses pin multiplexing, so a physical pin can have
multiple possible functions.

---

# Device Tree Relationship

The physical wiring must match the Linux Device Tree configuration.

```text
Physical Pin
      |
      v
Pin Multiplexer
      |
      v
Device Tree
      |
      v
Linux Driver
      |
      v
User Application
```

For example, for SPI:

```text
SPI Device
    |
    v
SPI Wiring
    |
    v
SPI Pinmux
    |
    v
Device Tree
    |
    v
SPI Controller Driver
    |
    v
SPI Device Driver
```

---

# Hardware Validation Checklist

Before powering the board:

```text
[ ] Correct BeagleBone header selected
[ ] Correct pin number verified
[ ] Pin function verified
[ ] Device Tree configuration verified
[ ] Signal direction verified
[ ] VCC voltage verified
[ ] I/O voltage verified
[ ] GND connection verified
[ ] No short circuit
[ ] External device supply verified
```

---

# Signal Verification

For debugging, the following equipment can be useful:

```text
Multimeter
    |
    +---- Power / Voltage verification

Logic Analyzer
    |
    +---- UART / SPI / I2C / GPIO signal verification

Oscilloscope
    |
    +---- PWM / Clock / Signal integrity

CAN Analyzer
    |
    +---- CAN bus testing
```

---

# Recommended Testing Order

For each peripheral, use this sequence:

```text
1. Check pin mapping
        |
        v
2. Check hardware wiring
        |
        v
3. Check power and ground
        |
        v
4. Configure Device Tree
        |
        v
5. Configure pinmux
        |
        v
6. Boot Linux
        |
        v
7. Check kernel logs
        |
        v
8. Check device node / sysfs
        |
        v
9. Perform hardware loopback or peripheral test
        |
        v
10. Test Linux driver
        |
        v
11. Test user-space application
```

---

# Interface Comparison

| Interface | Main Signals         | Typical Use                      |
| --------- | -------------------- | -------------------------------- |
| ADC       | Analog input         | Sensors                          |
| GPIO      | GPIO                 | LEDs, buttons, control           |
| PWM       | PWM output           | Motors, LEDs, buzzers            |
| I2C       | SDA, SCL             | Sensors, EEPROM, RTC             |
| SPI       | MOSI, MISO, SCLK, CS | ADC, Flash, displays             |
| UART      | TX, RX               | Console, GPS, modem              |
| CAN       | CANH, CANL           | Automotive/industrial networking |

---

# Safety and Electrical Notes

## 1. Voltage Levels

Always verify the voltage levels of the external peripheral.

Do not directly connect an incompatible voltage signal to a
BeagleBone Black I/O pin.

---

## 2. Common Ground

For most direct logic-level interfaces, ensure the BeagleBone Black and
the external device have an appropriate common ground.

```text
BBB GND ---------------- Device GND
```

---

## 3. External Power

Do not assume that the BeagleBone Black can safely power every external
peripheral.

Check the peripheral's:

```text
Voltage
Current
Power requirements
```

Use an external regulated supply when required.

---

## 4. Level Shifting

If two devices use incompatible logic voltage levels:

```text
BeagleBone
   |
   v
Level Shifter
   |
   v
External Device
```

Use a level shifter appropriate for the interface and communication
speed.

---

# Hardware-to-Driver Architecture

The wiring documentation is one layer of the complete project.

```text
+------------------------------------------------+
|              User Applications                 |
+------------------------------------------------+
                       |
                       v
+------------------------------------------------+
|              Linux Kernel                      |
|                                                |
|  GPIO | ADC | PWM | I2C | SPI | UART | CAN   |
+------------------------------------------------+
                       |
                       v
+------------------------------------------------+
|             Device Drivers                     |
+------------------------------------------------+
                       |
                       v
+------------------------------------------------+
|              Device Tree                       |
+------------------------------------------------+
                       |
                       v
+------------------------------------------------+
|             Pin Multiplexer                    |
+------------------------------------------------+
                       |
                       v
+------------------------------------------------+
|          BeagleBone Black Hardware             |
+------------------------------------------------+
                       |
                       v
+------------------------------------------------+
|     External Sensors / Peripherals / Devices   |
+------------------------------------------------+
```

---

# Project Documentation Structure

The complete hardware documentation is organized as:

```text
hardware/
│
├── pinout/
│   ├── README.md
│   ├── adc_pin_map.md
│   ├── bbb_header_p8.md
│   ├── bbb_header_p9.md
│   ├── can_pin_map.md
│   ├── gpio_pin_map.md
│   ├── i2c_pin_map.md
│   ├── pwm_pin_map.md
│   ├── spi_pin_map.md
│   └── uart_pin_map.md
│
├── schematics/
│   ├── adc/
│   ├── can/
│   ├── gpio/
│   ├── i2c/
│   ├── pwm/
│   ├── spi/
│   └── uart/
│
└── wiring/
    ├── README.md
    ├── adc_wiring.md
    ├── can_wiring.md
    ├── gpio_wiring.md
    ├── i2c_wiring.md
    ├── pwm_wiring.md
    ├── spi_wiring.md
    └── uart_wiring.md
```

---

# Relationship Between Hardware Documents

```text
                 hardware/
                     |
        +------------+------------+
        |            |            |
        v            v            v
     pinout      schematics     wiring
        |            |            |
        |            |            |
        +------------+------------+
                     |
                     v
               Device Tree
                     |
                     v
               Linux Driver
                     |
                     v
                 Testing
```

### Pinout

Answers:

```text
"Which physical pin do I use?"
```

### Schematic

Answers:

```text
"How should the circuit be connected?"
```

### Wiring

Answers:

```text
"How do I physically connect the board and peripheral?"
```

### Device Tree

Answers:

```text
"How do I describe and enable the hardware to Linux?"
```

### Driver

Answers:

```text
"How does Linux communicate with the hardware?"
```

---

# Final Goal

This project aims to provide a complete BeagleBone Black embedded
Linux hardware-to-driver learning and validation platform.

The complete path is:

```text
                    HARDWARE
                       |
                       v
              BeagleBone Black
                       |
                       v
                 Pin Mapping
                       |
                       v
                   Wiring
                       |
                       v
                 Schematic
                       |
                       v
                Device Tree
                       |
                       v
                  Pinmux
                       |
                       v
              Linux Kernel
                       |
                       v
                  Driver
                       |
                       v
             Device Interface
                       |
                       v
                User Space
                       |
                       v
                Test Program
```

The supported peripheral areas are:

```text
ADC
CAN
GPIO
I2C
PWM
SPI
UART
```

Each interface should be validated independently and then integrated
into the complete BeagleBone Black Linux device-driver project.

```
```

