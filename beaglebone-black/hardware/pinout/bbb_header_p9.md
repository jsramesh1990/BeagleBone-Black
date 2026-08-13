# BeagleBone Black P9 Header Pinout

## 1. Overview

The **P9 header** is a 46-pin expansion header on the BeagleBone Black. It provides access to power, ground, ADC, GPIO, UART, I2C, SPI, PWM, and other AM335x peripheral functions.

```text
                 BeagleBone Black

             +----------------------+
             |                      |
             |       P9 Header      |
             |       46 Pins        |
             |                      |
             +----------------------+
```

> **Important:** BeagleBone Black pins are multiplexed. The actual peripheral function depends on the AM335x pinmux and Device Tree configuration.

---

# 2. P9 Header Layout

```text
                    P9 HEADER

        1   3   5   7   9  11  13  15  17  19  21  23
        2   4   6   8  10  12  14  16  18  20  22  24
       26  28  30  32  34  36  38  40  42  44  46
       25  27  29  31  33  35  37  39  41  43  45
```

P9 provides several important interfaces used in this device-driver
project.

---

# 3. P9 Power Pins

|   Pin | Voltage / Function |
| ----: | ------------------ |
|  P9.1 | GND                |
|  P9.2 | GND                |
|  P9.3 | 3.3V               |
|  P9.4 | 3.3V               |
|  P9.5 | VDD_5V             |
|  P9.6 | VDD_5V             |
|  P9.7 | SYS_5V             |
|  P9.8 | SYS_5V             |
|  P9.9 | GND                |
| P9.10 | GND                |
| P9.21 | GND                |
| P9.22 | GND                |
| P9.43 | GND                |
| P9.44 | GND                |
| P9.45 | GND                |
| P9.46 | GND                |

### Ground

Common ground is required when connecting most external devices:

```text
BeagleBone Black          External Device
----------------          ---------------

P9.1  GND  ------------- GND
```

---

# 4. P9 GPIO Pins

Many P9 pins can operate as GPIO through the AM335x GPIO controllers.

| Header Pin | GPIO     | Typical Use |
| ---------- | -------- | ----------- |
| P9.11      | GPIO0_30 | GPIO        |
| P9.12      | GPIO1_28 | GPIO        |
| P9.13      | GPIO0_31 | GPIO        |
| P9.14      | GPIO1_18 | GPIO        |
| P9.15      | GPIO1_16 | GPIO        |
| P9.16      | GPIO1_19 | GPIO        |
| P9.17      | GPIO0_5  | GPIO / SPI  |
| P9.18      | GPIO0_4  | GPIO / SPI  |
| P9.21      | GPIO0_3  | GPIO / SPI  |
| P9.22      | GPIO0_2  | GPIO / SPI  |
| P9.23      | GPIO1_17 | GPIO        |
| P9.24      | GPIO0_15 | GPIO        |
| P9.25      | GPIO3_21 | GPIO        |
| P9.26      | GPIO0_14 | GPIO / UART |
| P9.27      | GPIO3_19 | GPIO        |
| P9.28      | GPIO3_17 | GPIO / SPI  |
| P9.29      | GPIO3_16 | GPIO / SPI  |
| P9.30      | GPIO3_16 | GPIO        |
| P9.31      | GPIO3_14 | GPIO / SPI  |
| P9.41      | GPIO0_20 | GPIO        |
| P9.42      | GPIO0_7  | GPIO        |

> **Note:** Some pins have multiple possible functions. Always verify the exact GPIO number and active mux configuration before using a pin in a driver.

---

# 5. UART Pins

P9 provides commonly used UART interfaces.

## UART1

| Signal    | Header Pin |
| --------- | ---------: |
| UART1_TXD |      P9.24 |
| UART1_RXD |      P9.26 |

Connection:

```text
BeagleBone Black          USB-UART Adapter
----------------          ----------------

P9.24 TX  -------------> RX
P9.26 RX  <------------- TX
P9.1  GND ------------- GND
```

### Linux Device

Depending on the board configuration:

```bash
ls /dev/ttyS*
```

Example:

```text
/dev/ttyS1
```

---

# 6. I2C Pins

P9 provides the commonly used I2C2 interface.

| Signal   | Header Pin |
| -------- | ---------: |
| I2C2_SCL |      P9.19 |
| I2C2_SDA |      P9.20 |

Connection:

```text
BeagleBone Black          I2C Sensor
----------------          ----------

P9.19 SCL -------------- SCL
P9.20 SDA -------------- SDA
P9.3  3.3V ------------- VCC
P9.1  GND -------------- GND
```

### Linux Verification

```bash
i2cdetect -l
```

Then scan the appropriate bus:

```bash
sudo i2cdetect -y 1
```

Example:

```text
     0 1 2 3 4 5 6 7 8 9 a b c d e f

00:          -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- 48 -- -- -- -- -- -- --
```

The `48` represents an example I2C device address.

---

# 7. SPI Pins

P9 contains the commonly used SPI0 signals.

| SPI Signal     | Header Pin |
| -------------- | ---------: |
| SPI0_CS0       |      P9.17 |
| SPI0_D1 / MOSI |      P9.18 |
| SPI0_D0 / MISO |      P9.21 |
| SPI0_SCLK      |      P9.22 |

Connection:

```text
BeagleBone Black          SPI Device
----------------          ----------

P9.22 SCLK -------------> SCLK
P9.18 MOSI -------------> MOSI
P9.21 MISO <------------- MISO
P9.17 CS0  -------------> CS
P9.1  GND  ------------- GND
```

Linux device example:

```bash
ls /dev/spidev*
```

Possible result:

```text
/dev/spidev1.0
```

---

# 8. PWM Pins

Several P9 pins support PWM functionality through AM335x eHRPWM
controllers.

Common PWM-capable pins include:

| Header Pin | Possible Function |
| ---------: | ----------------- |
|      P9.14 | PWM               |
|      P9.16 | PWM               |
|      P9.21 | PWM               |
|      P9.22 | PWM               |
|      P9.28 | PWM               |
|      P9.29 | PWM               |
|      P9.31 | PWM               |

The exact PWM channel depends on the selected pinmux.

PWM flow:

```text
User Application
       |
       v
Linux PWM Sysfs/Character Interface
       |
       v
PWM Subsystem
       |
       v
AM335x PWM Driver
       |
       v
eHRPWM Hardware
       |
       v
P9 PWM Pin
```

---

# 9. ADC Pins

The BeagleBone Black exposes eight analog input channels on P9.

| ADC Channel | Header Pin | Signal         |
| ----------- | ---------: | -------------- |
| AIN0        |      P9.39 | Analog Input 0 |
| AIN1        |      P9.40 | Analog Input 1 |
| AIN2        |      P9.37 | Analog Input 2 |
| AIN3        |      P9.38 | Analog Input 3 |
| AIN4        |      P9.33 | Analog Input 4 |
| AIN5        |      P9.36 | Analog Input 5 |
| AIN6        |      P9.35 | Analog Input 6 |
| AIN7        |      P9.34 | Analog Input 7 |

Example:

```text
Analog Sensor
     |
     +---------- P9.39 AIN0
     |
    GND
     |
     +---------- P9.1 GND
```

### ADC Safety

The AM335x ADC input range is approximately:

```text
0 V to 1.8 V
```

Do **not** connect a 3.3 V analog signal directly to an ADC input.

Use signal conditioning or a voltage divider when required.

---

# 10. CAN Interface

The BeagleBone Black can expose CAN functionality through appropriate
pinmux and an external CAN transceiver.

Typical architecture:

```text
AM335x CAN Controller
          |
          v
     CAN Driver
          |
          v
     SocketCAN
          |
          v
 CAN Transceiver
          |
          v
       CAN Bus
```

Example hardware:

```text
BBB CAN TX/RX
      |
      v
CAN Transceiver
      |
      +------ CANH
      |
      +------ CANL
```

> The AM335x CAN controller uses a transceiver for physical CAN bus
> signaling. Do not connect CAN controller signals directly to a CAN bus.

---

# 11. GPIO LED Test

Example GPIO output:

```text
P9 GPIO
   |
  330R
   |
  LED
   |
  GND
```

Driver/application flow:

```text
Application
     |
     v
GPIO Character Device
     |
     v
GPIO Subsystem
     |
     v
AM335x GPIO Controller
     |
     v
P9 GPIO
     |
     v
LED
```

---

# 12. GPIO Button Test

```text
                 +3.3V
                   |
                 Button
                   |
                   +-------- P9 GPIO
                   |
                Pull-down
                   |
                  GND
```

Expected operation:

```text
Button Released → GPIO LOW

Button Pressed  → GPIO HIGH
```

---

# 13. Device Tree Relationship

The P9 header is controlled through the AM335x pinmux.

Example architecture:

```text
                 P9 Header
                    |
                    v
              AM335x Pinmux
                    |
                    v
              Device Tree
                    |
          +---------+---------+
          |         |         |
        GPIO       UART      I2C
          |         |         |
          v         v         v
       Driver    Driver    Driver
          |         |         |
          +---------+---------+
                    |
                    v
             Linux Subsystem
                    |
                    v
              User Space
```

Example project Device Tree structure:

```text
device-tree/
├── adc/
├── can/
├── gpio/
├── i2c/
├── overlays/
├── pwm/
├── spi/
└── uart/
```

---

# 14. P9 Peripheral Summary

| Peripheral | P9 Pins                                         | Linux Subsystem |
| ---------- | ----------------------------------------------- | --------------- |
| GPIO       | Multiple                                        | GPIO            |
| UART       | P9.24/P9.26                                     | TTY             |
| I2C        | P9.19/P9.20                                     | I2C             |
| SPI        | P9.17/P9.18/P9.21/P9.22                         | SPI             |
| PWM        | Multiple                                        | PWM             |
| ADC        | P9.33/P9.34/P9.35/P9.36/P9.37/P9.38/P9.39/P9.40 | IIO             |
| CAN        | Pinmux dependent                                | SocketCAN       |
| 3.3V       | P9.3/P9.4                                       | Power           |
| 5V         | P9.5/P9.6/P9.7/P9.8                             | Power           |
| GND        | Multiple                                        | Ground          |

---

# 15. P9 Driver Testing Flow

```text
             P9 HEADER
                 |
                 v
            PINMUX CONFIG
                 |
                 v
           DEVICE TREE
                 |
                 v
          LINUX KERNEL
                 |
        +--------+--------+
        |        |        |
       GPIO     UART     I2C
        |        |        |
        v        v        v
      Driver   Driver   Driver
        |        |        |
        +--------+--------+
                 |
        +--------+--------+
        |        |        |
       SPI      PWM      ADC
        |        |        |
        v        v        v
      Driver   Driver   IIO
        |        |        |
        +--------+--------+
                 |
                 v
            USER SPACE
                 |
                 v
             TEST TOOLS
```

---

# 16. Linux Verification Commands

### GPIO

```bash
gpioinfo
```

### UART

```bash
ls -l /dev/ttyS*
```

### I2C

```bash
i2cdetect -l
```

### SPI

```bash
ls -l /dev/spidev*
```

### PWM

```bash
ls /sys/class/pwm/
```

### ADC

```bash
ls /sys/bus/iio/devices/
```

### CAN

```bash
ip link show
```

If CAN is enabled:

```bash
ip link show can0
```

---

# 17. Important Electrical Rules

```text
1. Verify the pinmux before connecting hardware.
2. Do not apply unsupported voltage levels.
3. Use common GND between the BBB and external device.
4. Do not connect 5 V signals directly to 3.3 V GPIO.
5. Do not connect 3.3 V directly to the ADC input.
6. Use level shifters where required.
7. Use appropriate resistors for LEDs and buttons.
8. Check for pinmux conflicts.
9. Check the active Device Tree before testing.
10. Verify the signal using a multimeter or logic analyzer.
```

---

# 18. P8 vs P9

| Feature      | P8                    | P9        |
| ------------ | --------------------- | --------- |
| GPIO         | Yes                   | Yes       |
| UART         | Yes                   | Yes       |
| SPI          | Yes                   | Yes       |
| I2C          | Limited/alternate mux | Yes       |
| PWM          | Yes                   | Yes       |
| ADC          | No                    | Yes       |
| Power        | Limited               | Extensive |
| Ground       | Yes                   | Yes       |
| Analog Input | No                    | Yes       |

For this project, **P9 is especially important** because it provides
the ADC inputs and several commonly used communication interfaces.

---

# 19. Project File

This document belongs to:

```text
beaglebone-black/
└── hardware/
    └── pinout/
        └── bbb_header_p9.md
```

It should be used together with:

```text
bbb_header_p8.md
gpio_pin_map.md
uart_pin_map.md
i2c_pin_map.md
spi_pin_map.md
pwm_pin_map.md
adc_pin_map.md
can_pin_map.md
```

---

# 20. Complete Hardware-to-Driver Concept

```text
                  BEAGLEBONE BLACK
                         |
             +-----------+-----------+
             |                       |
          P8 HEADER               P9 HEADER
             |                       |
             |          +------------+------------+
             |          |     |      |      |     |
            GPIO       UART   I2C    SPI    PWM   ADC
             |          |     |      |      |     |
             +----------+-----+------+------+-    |
                        |                       |
                        v                       v
                  DEVICE TREE                 IIO
                        |
                        v
                  PINMUX CONFIG
                        |
                        v
                  LINUX KERNEL
                        |
             +----------+----------+
             |          |          |
          GPIO      TTY/I2C/SPI   PWM
          Driver      Drivers    Driver
             |          |          |
             +----------+----------+
                        |
                        v
                  USER SPACE
                        |
                        v
                 TEST APPLICATIONS
```

This makes `bbb_header_p9.md` the **physical pin reference** for the
rest of your BeagleBone Black Linux device-driver project.

