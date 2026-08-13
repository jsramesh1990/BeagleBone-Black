# BeagleBone Black Hardware Pinout

## 1. Overview

This directory documents the **BeagleBone Black hardware pinout**, with a
focus on the peripherals used in the Embedded Linux device-driver project.

The BeagleBone Black provides two expansion headers:

* **P8** — 46 pins
* **P9** — 46 pins

The AM335x SoC uses **pin multiplexing (pinmux)**, so a physical header
pin can provide different functions such as GPIO, UART, I2C, SPI, PWM,
ADC, or CAN.

---

## 2. Directory Structure

```text
beaglebone-black/
└── hardware/
    └── pinout/
        ├── README.md
        ├── adc_pin_map.md
        ├── bbb_header_p8.md
        ├── bbb_header_p9.md
        ├── can_pin_map.md
        ├── gpio_pin_map.md
        ├── i2c_pin_map.md
        ├── pwm_pin_map.md
        ├── spi_pin_map.md
        └── uart_pin_map.md
```

---

## 3. Pinout Documentation

| File               | Description                              |
| ------------------ | ---------------------------------------- |
| `bbb_header_p8.md` | Complete P8 expansion-header reference   |
| `bbb_header_p9.md` | Complete P9 expansion-header reference   |
| `gpio_pin_map.md`  | GPIO pin mapping and GPIO numbering      |
| `i2c_pin_map.md`   | I2C bus and SDA/SCL mapping              |
| `spi_pin_map.md`   | SPI bus, MOSI, MISO, SCLK and CS mapping |
| `uart_pin_map.md`  | UART TX/RX mapping                       |
| `pwm_pin_map.md`   | PWM-capable pin mapping                  |
| `adc_pin_map.md`   | ADC input mapping                        |
| `can_pin_map.md`   | CAN TX/RX mapping                        |

---

# 4. BeagleBone Black Headers

The two main expansion headers are:

```text
             BeagleBone Black
          +---------------------+
          |                     |
          |      AM335x SoC     |
          |                     |
          +---------------------+
             |               |
             v               v
           P8 Header       P9 Header
           46 Pins         46 Pins
```

Each header contains:

```text
GPIO
UART
I2C
SPI
PWM
ADC
CAN
Power
Ground
```

depending on the particular pin.

---

# 5. Important Pin Categories

## GPIO

General-purpose digital input/output.

```text
Application
    |
    v
GPIO Driver
    |
    v
GPIO Controller
    |
    v
BBB Header Pin
```

See:

```text
gpio_pin_map.md
```

---

## UART

Used for:

* Serial console
* GPS
* Modem
* Bluetooth
* MCU communication
* Debugging

See:

```text
uart_pin_map.md
```

---

## I2C

Used for:

* Sensors
* EEPROM
* RTC
* GPIO expanders
* PMICs
* Temperature sensors

Signals:

```text
SDA
SCL
```

See:

```text
i2c_pin_map.md
```

---

## SPI

Used for:

* ADC
* DAC
* Displays
* Flash
* Sensors
* External controllers

Signals:

```text
SCLK
MOSI
MISO
CS
```

See:

```text
spi_pin_map.md
```

---

## PWM

Used for:

* Motor control
* Servo control
* LED brightness
* Fan control
* Actuators

See:

```text
pwm_pin_map.md
```

---

## ADC

Used for reading analog voltage.

Typical applications:

```text
Analog Sensor
      |
      v
     ADC
      |
      v
BeagleBone Black
      |
      v
Linux Application
```

See:

```text
adc_pin_map.md
```

---

## CAN

Used for:

* Automotive systems
* Industrial controllers
* Robotics
* Motor controllers
* Embedded networking

Typical signals:

```text
CAN_TX
CAN_RX
```

An external CAN transceiver is normally required to interface with a
physical CAN bus.

See:

```text
can_pin_map.md
```

---

# 6. Pin Multiplexing

The AM335x supports multiple functions on many physical pins.

Conceptually:

```text
                    Physical Pin
                         |
          +--------------+--------------+
          |              |              |
          v              v              v
         GPIO           UART           SPI
          |              |              |
          +--------------+--------------+
                         |
                      Pinmux
                         |
                         v
                    Active Function
```

For example:

```text
P9.xx
  |
  +---- GPIO
  +---- UART
  +---- SPI
  +---- PWM
```

Only the selected function should normally be active at a time.

---

# 7. Device Tree Relationship

The hardware pinout documentation connects directly to the Device Tree
portion of this project.

```text
hardware/pinout/
        |
        v
Physical Pin Mapping
        |
        v
device-tree/
        |
        v
Pinmux Configuration
        |
        v
Linux Kernel
        |
        v
Device Driver
        |
        v
Hardware
```

For example:

```text
SPI Pin
   |
   v
spi_pin_map.md
   |
   v
bbb-spi.dts
   |
   v
SPI Pinmux
   |
   v
AM335x McSPI Driver
```

---

# 8. Pin Conflict

Because the BeagleBone Black uses pin multiplexing, peripheral functions
can conflict.

Example:

```text
          P9.xx
            |
      +-----+-----+
      |     |     |
     GPIO UART   SPI
```

If the pin is configured for SPI, it cannot simultaneously function as
a normal GPIO or UART pin.

Before enabling a peripheral:

```text
1. Check pin map
2. Check P8/P9 header
3. Check pinmux
4. Check Device Tree
5. Check existing peripherals
6. Check for pin conflicts
```

---

# 9. Power and Ground

The pinout documentation also identifies:

```text
5V
3.3V
GND
VIN
```

Power pins should be used carefully.

Important:

> Do not assume every external peripheral can be powered directly from
> a BeagleBone Black header pin. Check the peripheral's voltage and
> current requirements.

---

# 10. Recommended Hardware Bring-Up Flow

For every peripheral in this project:

```text
              Hardware
                 |
                 v
           Check Pin Map
                 |
                 v
          Check Electrical
            Requirements
                 |
                 v
          Configure Pinmux
                 |
                 v
          Configure Device Tree
                 |
                 v
          Enable Linux Driver
                 |
                 v
             Test Device
                 |
                 v
           Debug with Tools
```

---

# 11. Pinout Files

### Header Reference

```text
bbb_header_p8.md
bbb_header_p9.md
```

These provide the physical P8/P9 header reference.

### Peripheral References

```text
adc_pin_map.md
can_pin_map.md
gpio_pin_map.md
i2c_pin_map.md
pwm_pin_map.md
spi_pin_map.md
uart_pin_map.md
```

These provide peripheral-specific mappings.

---

# 12. Device Driver Project Mapping

This hardware directory supports the complete device-driver project:

```text
                     BeagleBone Black
                            |
                            v
                     Hardware Pinout
                            |
       +---------+----------+----------+---------+
       |         |          |          |         |
       v         v          v          v         v
      GPIO      UART       I2C        SPI       PWM
       |         |          |          |         |
       +---------+----------+----------+---------+
                            |
                            v
                       Device Tree
                            |
                            v
                      Linux Drivers
                            |
                            v
                       User Space
```

Additional peripherals:

```text
ADC
CAN
```

are documented separately and integrated into the same architecture.

---

# 13. Quick Navigation

```text
P8 Header
└── bbb_header_p8.md

P9 Header
└── bbb_header_p9.md

GPIO
└── gpio_pin_map.md

UART
└── uart_pin_map.md

I2C
└── i2c_pin_map.md

SPI
└── spi_pin_map.md

PWM
└── pwm_pin_map.md

ADC
└── adc_pin_map.md

CAN
└── can_pin_map.md
```

---

# 14. Hardware Verification Checklist

Before connecting any peripheral:

```text
[ ] Correct P8/P9 pin identified
[ ] Correct peripheral function verified
[ ] Pinmux checked
[ ] Device Tree checked
[ ] Voltage level checked
[ ] Ground connected
[ ] Power requirements checked
[ ] Pin conflict checked
[ ] External transceiver checked if required
[ ] Signal direction checked
[ ] Linux device node verified
[ ] Hardware communication tested
```

---

# 15. Project Goal

The purpose of this directory is to provide a single hardware reference
for implementing and testing **multiple Linux device drivers on the
BeagleBone Black**.

The project covers:

```text
GPIO
UART
I2C
SPI
PWM
ADC
CAN
```

and connects the physical hardware layer with:

```text
Pinout
   ↓
Pinmux
   ↓
Device Tree
   ↓
Linux Kernel
   ↓
Device Driver
   ↓
User-Space Application
   ↓
Hardware Testing
```

---

## Directory

```text
beaglebone-black/hardware/pinout/README.md
```

