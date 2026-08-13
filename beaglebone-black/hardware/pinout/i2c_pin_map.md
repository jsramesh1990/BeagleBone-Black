# BeagleBone Black I2C Pin Map

## 1. Overview

The **BeagleBone Black** is based on the TI AM335x processor, which
provides multiple I2C controller instances.

I2C is commonly used for:

* Sensors
* EEPROM
* RTC
* PMIC
* Temperature sensors
* Accelerometers
* GPIO expanders
* Display controllers

Linux provides the **I2C subsystem** for communicating with I2C devices.

```text
User Application
       |
       v
   /dev/i2c-X
       |
       v
    I2C Core
       |
       v
 I2C Controller Driver
       |
       v
  AM335x I2C
       |
       v
   SDA / SCL
       |
       v
   I2C Device
```

---

# 2. BeagleBone Black I2C Interfaces

The AM335x provides several I2C controller instances.

For BeagleBone Black expansion-header use, the commonly used interface is:

```text
I2C2
```

The physical pins are:

| Signal   | Header Pin |
| -------- | ---------: |
| I2C2_SCL |      P9.19 |
| I2C2_SDA |      P9.20 |

These pins are also multiplexed with other peripheral functions.

---

# 3. I2C2 Pin Mapping

## SCL

```text
Signal      : I2C2_SCL
Header Pin  : P9.19
Function    : I2C Clock
```

## SDA

```text
Signal      : I2C2_SDA
Header Pin  : P9.20
Function    : I2C Data
```

Quick reference:

```text
+-------------+-------------+
| I2C Signal  | BBB Pin     |
+-------------+-------------+
| I2C2_SCL    | P9.19       |
| I2C2_SDA    | P9.20       |
| GND         | P9.1        |
| 3.3V        | P9.3        |
+-------------+-------------+
```

---

# 4. Basic I2C Wiring

Example connection to an I2C sensor:

```text
BeagleBone Black             I2C Sensor
----------------             ----------

P9.19 SCL  ----------------> SCL

P9.20 SDA  <--------------> SDA

P9.3  3.3V  --------------> VCC

P9.1  GND   --------------> GND
```

Complete diagram:

```text
              BeagleBone Black
             +----------------+
             |                |
P9.19 SCL ---|----------------|--- SCL
             |                |
P9.20 SDA ---|----------------|--- SDA
             |                |
P9.3  3.3V --|----------------|--- VCC
             |                |
P9.1  GND ---|----------------|--- GND
             |                |
             +----------------+
```

---

# 5. I2C Bus Structure

I2C supports multiple devices on the same bus.

```text
                    +----------------+
                    | BeagleBone      |
                    | Black           |
                    +----------------+
                       |          |
                      SCL        SDA
                       |          |
          +------------+----------+------------+
          |            |                       |
          v            v                       v
     +---------+  +---------+             +---------+
     | Sensor  |  | EEPROM  |             | RTC     |
     | 0x48    |  | 0x50    |             | 0x68    |
     +---------+  +---------+             +---------+
```

All devices share:

```text
SCL
SDA
GND
```

Each device must have a unique I2C address.

---

# 6. I2C Pull-Up Resistors

I2C uses **open-drain/open-collector signaling**, so pull-up resistors
are required.

Typical arrangement:

```text
                  3.3V
                   |
             +-----+-----+
             |           |
            4.7K        4.7K
             |           |
             |           |
            SDA         SCL
             |           |
             +-----+-----+
                   |
              I2C Devices
```

Typical values:

```text
SDA → 4.7KΩ → 3.3V
SCL → 4.7KΩ → 3.3V
```

The exact pull-up value depends on:

* Bus capacitance
* Bus speed
* Number of devices
* Device specifications
* Total wiring length

> Many I2C breakout boards already include pull-up resistors. Check the
> module schematic before adding another set.

---

# 7. I2C Voltage

For the BeagleBone Black expansion headers, use appropriate **3.3 V
logic levels**.

```text
BBB Logic Level
      |
      v
   3.3 V
      |
      v
I2C Sensor
```

Do not connect 5 V I2C signals directly to the BeagleBone Black GPIO/I2C
pins.

For a 5 V-only I2C device, use an appropriate bidirectional level
shifter.

---

# 8. Linux I2C Device

Check available I2C buses:

```bash
i2cdetect -l
```

Example:

```text
i2c-0   i2c   OMAP I2C adapter
i2c-1   i2c   OMAP I2C adapter
```

The bus number depends on the kernel/Device Tree configuration.

> Do not assume `i2c-1` always corresponds to a particular physical
> header configuration. Verify the adapter name and Device Tree.

---

# 9. Scan I2C Bus

Install I2C utilities if required:

```bash
sudo apt install i2c-tools
```

Scan a bus:

```bash
sudo i2cdetect -y 1
```

Example:

```text
     0 1 2 3 4 5 6 7 8 9 a b c d e f

00:          -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- 48 -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

Here:

```text
0x48
```

is an example detected I2C slave address.

---

# 10. I2C Read Test

Example:

```bash
sudo i2cget -y 1 0x48 0x00
```

Meaning:

```text
Bus       : 1
Slave     : 0x48
Register  : 0x00
```

Example output:

```text
0x1a
```

---

# 11. I2C Write Test

Example:

```bash
sudo i2cset -y 1 0x48 0x01 0x80
```

Meaning:

```text
Bus       : 1
Slave     : 0x48
Register  : 0x01
Value     : 0x80
```

> Use `i2cset` only when you know the device register map. Writing an
> incorrect register can put some devices into an unexpected state.

---

# 12. I2C Register Dump

For supported devices:

```bash
sudo i2cdump -y 1 0x48
```

This can be useful for debugging register-based I2C devices.

Use it carefully because some devices have registers with side effects
when read.

---

# 13. I2C Device Tree

I2C controllers and attached devices are described using Device Tree.

Conceptually:

```dts
&i2c2 {
    status = "okay";

    sensor@48 {
        compatible = "vendor,sensor";
        reg = <0x48>;
    };
};
```

Architecture:

```text
Device Tree
     |
     v
I2C Controller
     |
     +------ sensor@48
     |
     +------ eeprom@50
     |
     +------ rtc@68
```

Project files:

```text
beaglebone-black/
└── device-tree/
    └── i2c/
        ├── bbb-i2c.dts
        ├── bbb-i2c.dtsi
        └── README.md
```

---

# 14. Device Tree Pinmux

The physical pins must be configured for I2C rather than GPIO or another
alternate function.

Conceptually:

```text
P9.19
  |
  +---- I2C2_SCL
  |
  v
Pin Controller
  |
  v
Device Tree
```

```text
P9.20
  |
  +---- I2C2_SDA
  |
  v
Pin Controller
  |
  v
Device Tree
```

---

# 15. I2C Driver Architecture

Linux I2C is divided into several layers.

```text
                   User Application
                          |
                          v
                    I2C Device
                     Interface
                          |
                          v
                      I2C Core
                          |
             +------------+------------+
             |                         |
      I2C Controller Driver       I2C Client Driver
             |                         |
             v                         v
        AM335x I2C HW             Sensor/EEPROM
             |                         |
             +------------+------------+
                          |
                          v
                      I2C Bus
```

---

# 16. I2C Controller Driver

The controller driver manages the AM335x I2C hardware.

Responsibilities include:

```text
1. Controller initialization
2. Clock configuration
3. Bus speed configuration
4. Start condition
5. Stop condition
6. Address transmission
7. Data transmission
8. Data reception
9. Interrupt handling
10. Error handling
```

---

# 17. I2C Client Driver

A sensor or EEPROM normally has its own I2C client driver.

Example:

```text
I2C Bus
   |
   +---- 0x48 → Temperature Sensor
   |
   +---- 0x50 → EEPROM
   |
   +---- 0x68 → RTC
```

The client driver communicates with the device using the Linux I2C API.

Typical APIs include:

```c
i2c_smbus_read_byte_data()
i2c_smbus_write_byte_data()
i2c_transfer()
```

---

# 18. I2C Probe Flow

When Linux detects an I2C device:

```text
Linux Boot
    |
    v
Device Tree
    |
    v
I2C Controller Driver
    |
    v
I2C Bus Registered
    |
    v
I2C Device Created
    |
    v
Driver Matching
    |
    v
Client Driver probe()
    |
    v
Device Initialization
    |
    v
Device Ready
```

---

# 19. I2C Driver Probe Example

Conceptually:

```c
static int sensor_probe(struct i2c_client *client)
{
    dev_info(&client->dev, "I2C sensor detected\n");

    /* Device initialization */

    return 0;
}
```

The driver is matched using the Device Tree:

```dts
sensor@48 {
    compatible = "mycompany,my-sensor";
    reg = <0x48>;
};
```

---

# 20. I2C Communication Flow

A typical register read:

```text
START
  |
  v
Slave Address + WRITE
  |
  v
Register Address
  |
  v
RESTART
  |
  v
Slave Address + READ
  |
  v
Device Data
  |
  v
STOP
```

Example:

```text
Host
 |
 | START
 |
 | 0x48 + WRITE
 |
 | Register 0x00
 |
 | RESTART
 |
 | 0x48 + READ
 |
 | <--- Data
 |
 | STOP
```

---

# 21. I2C Speed Modes

Common I2C modes:

| Mode            | Maximum Frequency |
| --------------- | ----------------: |
| Standard Mode   |           100 kHz |
| Fast Mode       |           400 kHz |
| Fast Mode Plus  |             1 MHz |
| High-Speed Mode |           3.4 MHz |

The actual supported speed depends on the controller, board, device,
pull-up network, and Device Tree configuration.

---

# 22. I2C Interrupt Flow

The controller can use interrupts to handle transfers.

```text
I2C Transfer
     |
     v
AM335x I2C Controller
     |
     v
Hardware Interrupt
     |
     v
I2C Driver ISR
     |
     v
Transfer State Machine
     |
     v
I2C Core
     |
     v
Client Driver
```

---

# 23. I2C Debugging

Check I2C adapters:

```bash
i2cdetect -l
```

Scan the bus:

```bash
sudo i2cdetect -y 1
```

Check kernel logs:

```bash
dmesg | grep -i i2c
```

Check I2C devices:

```bash
ls /sys/bus/i2c/devices/
```

Example:

```text
1-0048
1-0050
```

This can represent:

```text
I2C Bus 1
   |
   +-- 0x48
   |
   +-- 0x50
```

---

# 24. I2C Debugging with Logic Analyzer

A logic analyzer is very useful for debugging I2C.

Connect:

```text
Logic Analyzer       BeagleBone
--------------       ----------

Channel 0  --------> SDA
Channel 1  --------> SCL
GND        --------> GND
```

Observe:

```text
START
  |
ADDRESS
  |
ACK
  |
REGISTER
  |
ACK
  |
DATA
  |
ACK/NACK
  |
STOP
```

---

# 25. Common I2C Problems

## No Device Detected

```bash
sudo i2cdetect -y 1
```

If the expected address does not appear, check:

```text
1. SDA wiring
2. SCL wiring
3. GND connection
4. Device power
5. I2C address
6. Pull-up resistors
7. Device Tree
8. Pinmux
9. Correct I2C bus
```

---

## SDA/SCL Stuck LOW

Possible causes:

```text
1. Incorrect wiring
2. Missing pull-up
3. Short circuit
4. Slave holding bus
5. Incorrect voltage
6. Faulty device
```

---

## Address Conflict

Two devices cannot normally use the same address on the same bus.

Example:

```text
Sensor 1 → 0x48
Sensor 2 → 0x48
```

Possible solutions:

```text
1. Change device address
2. Use separate I2C buses
3. Use an I2C multiplexer
```

---

# 26. I2C Testing Checklist

```text
[ ] I2C controller enabled
[ ] Device Tree configured
[ ] Pinmux configured
[ ] SDA connected
[ ] SCL connected
[ ] GND connected
[ ] Device powered
[ ] Pull-ups verified
[ ] I2C bus detected
[ ] Correct bus selected
[ ] Slave address detected
[ ] Register read tested
[ ] Register write tested
[ ] Driver probe tested
[ ] Kernel logs checked
[ ] Logic analyzer tested
[ ] Error handling tested
```

---

# 27. I2C Hardware Test

Recommended sensor setup:

```text
                BeagleBone Black
                       |
          +------------+------------+
          |                         |
        P9.19                      P9.20
          |                         |
         SCL                       SDA
          |                         |
          +------------+------------+
                       |
                  I2C Sensor
                    0x48
                       |
                  +----+----+
                  |         |
                 3.3V      GND
```

---

# 28. I2C Pin Quick Reference

```text
+----------------+-------------+----------------------+
| Signal         | BBB Pin     | Function             |
+----------------+-------------+----------------------+
| I2C2_SCL       | P9.19       | I2C Clock            |
| I2C2_SDA       | P9.20       | I2C Data             |
| 3.3V           | P9.3        | Sensor Power         |
| GND            | P9.1        | Ground               |
+----------------+-------------+----------------------+
```

---

# 29. I2C and GPIO Conflict

Because I2C pins are multiplexed, the same physical pins should not be
configured simultaneously for unrelated functions.

Example:

```text
P9.19
  |
  +---- GPIO
  |
  +---- I2C2_SCL
  |
  +---- Other Alternate Function
```

Device Tree selects the required function:

```text
              P9.19
                |
             PINMUX
                |
        +-------+-------+
        |               |
       GPIO          I2C2_SCL
                         |
                         v
                    I2C Controller
```

For this project, when I2C is enabled on P9.19/P9.20, don't configure
those same pins for GPIO or CAN at the same time.

---

# 30. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   └── pinout/
│       └── i2c_pin_map.md
│
├── device-tree/
│   └── i2c/
│       ├── bbb-i2c.dts
│       ├── bbb-i2c.dtsi
│       └── README.md
│
├── drivers/
│   └── i2c/
│       ├── README.md
│       └── ...
│
└── tests/
    └── i2c/
        ├── i2c_scan.sh
        ├── i2c_read_test.sh
        └── i2c_write_test.sh
```

---

# 31. Complete I2C Architecture

```text
                    BeagleBone Black
                           |
                           v
                      P9.19/P9.20
                           |
                           v
                        Pinmux
                           |
                           v
                     Device Tree
                           |
                           v
                   AM335x I2C Controller
                           |
                           v
                       I2C Driver
                           |
                           v
                        I2C Core
                           |
              +------------+------------+
              |            |            |
              v            v            v
           Sensor        EEPROM        RTC
           0x48          0x50          0x68
              |            |            |
              +------------+------------+
                           |
                           v
                     User Application
```

---

## Project File

```text
beaglebone-black/hardware/pinout/i2c_pin_map.md
```

