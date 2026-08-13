# `i2c_test_circuit.md`

````markdown
# BeagleBone Black I2C Test Circuit

## 1. Overview

This document describes a basic hardware test circuit for validating I2C
communication on the BeagleBone Black.

The test uses an external I2C device such as an EEPROM, temperature
sensor, RTC, or I/O expander.

The complete path is:

```text
I2C Sensor / EEPROM
        |
        | SDA
        | SCL
        v
BeagleBone Black
        |
        v
AM335x I2C Controller
        |
        v
Linux I2C Driver
        |
        v
I2C Framework
        |
        v
/dev/i2c-X
        |
        v
User-Space Application
````

---

# 2. I2C Test Objectives

This test validates:

* I2C SDA communication
* I2C SCL communication
* I2C master operation
* I2C device detection
* I2C Device Tree configuration
* Linux I2C driver
* I2C bus scanning
* Register read/write
* User-space I2C communication

---

# 3. Required Components

Recommended test components:

```text
1 × BeagleBone Black
1 × I2C sensor / EEPROM
2 × 4.7 kΩ pull-up resistors
Breadboard
Jumper wires
Multimeter
```

Example I2C devices:

```text
EEPROM
Temperature sensor
RTC
GPIO expander
Accelerometer
IMU
```

For a simple first test, an I2C EEPROM or temperature sensor is
recommended.

---

# 4. Basic I2C Circuit

I2C requires two signal lines:

```text
SDA = Serial Data
SCL = Serial Clock
```

Basic connection:

```text
             BeagleBone Black
                  |
        +---------+---------+
        |                   |
       SDA                 SCL
        |                   |
        |                   |
        v                   v
     I2C Device          I2C Device
        |                   |
        +-------------------+
                |
               GND
```

---

# 5. I2C Pull-Up Resistors

I2C SDA and SCL lines normally require pull-up resistors.

Typical test values:

```text
4.7 kΩ
```

Circuit:

```text
                    3.3 V
                     |
              +------+------+
              |             |
             4.7k          4.7k
              |             |
              |             |
             SDA           SCL
              |             |
              |             |
              +------+------+ 
                     |
               I2C Device
```

The actual pull-up value should be selected based on bus speed,
capacitance, voltage, and the connected devices.

---

# 6. Complete I2C Test Circuit

```text
                         3.3 V
                           |
                    +------+------+
                    |             |
                   4.7k          4.7k
                    |             |
                    |             |
BBB SDA ------------+-------------+---- SDA
                                      |
                                  +---+---+
                                  | I2C   |
                                  |Device |
                                  +---+---+
                                      |
BBB SCL ----------------------------- SCL

BBB GND ----------------------------- GND
```

More clearly:

```text
             BeagleBone Black

                3.3V
                  |
             +----+----+
             |         |
           4.7k      4.7k
             |         |
             |         |
            SDA       SCL
             |         |
             |         |
             +---------+------------+
                       |            |
                       v            v
                    I2C Device
                       |
                       |
                      GND
```

---

# 7. Power Connection

The I2C device must be powered using the voltage appropriate for that
device.

Typical arrangement:

```text
BBB 3.3V  ----------> I2C Device VCC
BBB GND  -----------> I2C Device GND
BBB SDA  -----------> I2C Device SDA
BBB SCL  -----------> I2C Device SCL
```

Important:

> Do not connect a 5 V I2C pull-up directly to a 3.3 V BeagleBone
> Black I/O bus. Verify the voltage requirements of the I2C device and
> its pull-ups.

---

# 8. I2C Device Address

Every I2C slave normally has an address.

Example:

```text
I2C Device
Address = 0x48
```

The Linux system can communicate with that device using its address.

Example:

```text
SLA = 0x48
```

The actual address depends on the connected device and its address-pin
configuration.

---

# 9. Example EEPROM Circuit

An I2C EEPROM is a good device for testing read/write communication.

```text
                  3.3 V
                    |
              +-----+-----+
              |           |
             4.7k        4.7k
              |           |
              |           |
BBB SDA ------+-----------+------ SDA
                              +-------+
BBB SCL ----------------------| SCL   |
                              | EEPROM|
3.3 V ------------------------| VCC   |
GND --------------------------| GND   |
                              +-------+
```

EEPROM address pins should be connected according to the EEPROM
datasheet.

---

# 10. I2C Test Architecture

```text
                   User Application
                          |
                          v
                    I2C User API
                          |
                          v
                    /dev/i2c-X
                          |
                          v
                    Linux I2C Core
                          |
                          v
                    I2C Controller
                          |
                          v
                    AM335x I2C
                          |
                          v
                       SDA/SCL
                          |
                          v
                    I2C Device
```

---

# 11. Device Tree

The I2C controller must be enabled and configured through Device Tree.

Project files:

```text
beaglebone-black/
└── device-tree/
    └── i2c/
        ├── bbb-i2c.dts
        ├── bbb-i2c.dtsi
        └── README.md
```

Conceptual flow:

```text
bbb-i2c.dts
      |
      v
Pinmux Configuration
      |
      v
I2C Controller
      |
      v
Linux I2C Driver
      |
      v
/dev/i2c-X
      |
      v
I2C Device
```

---

# 12. Check I2C Devices

After booting Linux:

```bash
ls /dev/i2c*
```

Example:

```text
/dev/i2c-0
/dev/i2c-1
```

The actual bus numbers depend on the active board configuration.

---

# 13. Install I2C Tools

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install i2c-tools
```

Useful commands:

```text
i2cdetect
i2cget
i2cset
i2cdump
i2ctransfer
```

---

# 14. Detect I2C Bus

Run:

```bash
i2cdetect -l
```

Example:

```text
i2c-1   i2c   OMAP I2C adapter   I2C adapter
```

This confirms that the Linux I2C adapter is registered.

---

# 15. Scan I2C Bus

Suppose the bus is:

```text
/dev/i2c-1
```

Run:

```bash
sudo i2cdetect -y 1
```

Example output:

```text
     0 1 2 3 4 5 6 7 8 9 a b c d e f
00:          -- -- -- -- -- -- -- -- --
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

indicates a responding I2C device at address `0x48`.

Do not assume `0x48` is universal; use the address specified by the
device datasheet.

---

# 16. I2C Detection Flow

```text
I2C Device Connected
        |
        v
SDA/SCL
        |
        v
I2C Controller
        |
        v
Linux I2C Driver
        |
        v
I2C Adapter
        |
        v
i2cdetect
        |
        v
Device Address
```

---

# 17. I2C Read Test

For a register-based device, the exact command depends on the device
datasheet.

Generic example:

```bash
sudo i2cget -y 1 0x48 0x00
```

Meaning:

```text
Bus       = 1
Slave     = 0x48
Register  = 0x00
```

The correct register address and transaction type must be taken from
the device datasheet.

---

# 18. I2C Write Test

Generic example:

```bash
sudo i2cset -y 1 0x48 0x01 0x55
```

Conceptually:

```text
I2C Bus
   |
   +-- Slave Address = 0x48
   |
   +-- Register = 0x01
   |
   +-- Data = 0x55
```

Only use `i2cset` when the device datasheet confirms that the selected
register supports writing.

---

# 19. I2C Transfer Test

For devices requiring more complex transactions:

```bash
sudo i2ctransfer -y 1 w2@0x48 0x00 0x55
```

The exact transaction must match the target device's protocol.

---

# 20. I2C Bus Signals

I2C uses:

```text
SDA
SCL
```

Conceptually:

```text
SCL  ──┐_┌─┐_┌─┐_┌─┐_┌─

SDA  ────┐___┌─────┐___
```

The master controls the clock and initiates transactions.

---

# 21. I2C START Condition

A START condition occurs when SDA transitions from HIGH to LOW while
SCL is HIGH.

```text
SCL  ────────────────
SDA  ────────┐_______
             ^
           START
```

---

# 22. I2C STOP Condition

A STOP condition occurs when SDA transitions from LOW to HIGH while
SCL is HIGH.

```text
SCL  ────────────────
SDA  _________┌──────
              ^
             STOP
```

---

# 23. I2C ACK

The receiving device normally acknowledges a transferred byte.

Conceptually:

```text
Master → Address → Slave
Master → Data    → Slave
Slave  → ACK
```

The ACK is important when debugging communication.

---

# 24. I2C Pull-Up Test

With the bus powered:

```text
Measure:

SDA → 3.3 V approximately when idle
SCL → 3.3 V approximately when idle
```

If either line is permanently LOW, investigate:

```text
[ ] Short circuit
[ ] Incorrect wiring
[ ] Device holding bus
[ ] Missing pull-up
[ ] Incorrect voltage
[ ] Pinmux configuration
```

The exact idle voltage depends on the board and connected circuitry.

---

# 25. I2C Hardware Test Procedure

### Step 1 — Power Off

Disconnect power before changing wiring.

### Step 2 — Connect I2C Device

```text
BBB 3.3V → Device VCC
BBB GND  → Device GND
BBB SDA  → Device SDA
BBB SCL  → Device SCL
```

### Step 3 — Add Pull-Ups

```text
SDA → 4.7 kΩ → 3.3 V
SCL → 4.7 kΩ → 3.3 V
```

If the breakout board already contains pull-ups, do not blindly add
additional resistors; check the board documentation.

### Step 4 — Boot Linux

### Step 5 — Check I2C Adapter

```bash
i2cdetect -l
```

### Step 6 — Scan Bus

```bash
sudo i2cdetect -y 1
```

### Step 7 — Identify Device

Example:

```text
0x48
```

### Step 8 — Read Device

Use the appropriate device-specific command.

### Step 9 — Validate Data

Compare the result against the device datasheet or known sensor value.

---

# 26. I2C Debugging

If no device appears:

```text
I2C Device Not Detected
        |
        +--> Check VCC
        |
        +--> Check GND
        |
        +--> Check SDA
        |
        +--> Check SCL
        |
        +--> Check pull-ups
        |
        +--> Check I2C address
        |
        +--> Check Device Tree
        |
        +--> Check pinmux
        |
        +--> Check I2C driver
```

---

# 27. SDA/SCL Reversed

Incorrect:

```text
BBB SDA -------- SCL Device
BBB SCL -------- SDA Device
```

Correct:

```text
BBB SDA -------- SDA Device
BBB SCL -------- SCL Device
```

---

# 28. Address Conflict

Multiple devices can share the same I2C bus if they have different
addresses.

```text
             I2C BUS
                |
        +-------+-------+
        |       |       |
        v       v       v
      0x48    0x50    0x68
     Sensor   EEPROM    RTC
```

Two devices with the same address can conflict unless the device
provides an address-selection mechanism or an I2C multiplexer is used.

---

# 29. I2C Bus Architecture

```text
                  3.3V
                   |
              Pull-Ups
               |     |
               |     |
               SDA   SCL
                |     |
       +--------+-----+--------+
       |        |     |        |
       v        v     v        v
    Sensor    EEPROM  RTC    Expander
    0x48       0x50  0x68     0x20
```

All devices share SDA and SCL.

---

# 30. Linux I2C Architecture

```text
+--------------------------------+
| User Application               |
+---------------+----------------+
                |
                v
+--------------------------------+
| /dev/i2c-X                     |
+---------------+----------------+
                |
                v
+--------------------------------+
| Linux I2C Core                 |
+---------------+----------------+
                |
                v
+--------------------------------+
| AM335x I2C Adapter Driver      |
+---------------+----------------+
                |
                v
+--------------------------------+
| AM335x I2C Controller          |
+---------------+----------------+
                |
                v
+--------------------------------+
| SDA / SCL                      |
+---------------+----------------+
                |
                v
+--------------------------------+
| I2C Slave Device               |
+--------------------------------+
```

---

# 31. I2C Kernel Messages

Check kernel logs:

```bash
dmesg | grep -i i2c
```

Check I2C-related messages:

```bash
dmesg | grep -i omap
```

Check device nodes:

```bash
ls -l /dev/i2c*
```

---

# 32. Device Tree Debugging

Check the Device Tree configuration:

```text
beaglebone-black/device-tree/i2c/
├── bbb-i2c.dts
├── bbb-i2c.dtsi
└── README.md
```

Verify:

```text
[ ] I2C controller enabled
[ ] Correct pinmux
[ ] Correct bus
[ ] Correct slave address
[ ] Device node configured
[ ] No pin conflict
```

---

# 33. Oscilloscope / Logic Analyzer Test

For hardware debugging, monitor:

```text
SDA
SCL
GND
```

Expected:

```text
SCL → Clock waveform
SDA → Data transitions
```

A logic analyzer is particularly useful for checking:

```text
START
Address
R/W bit
ACK
Data
STOP
```

---

# 34. I2C Logic Analyzer Flow

```text
BeagleBone Black
       |
       +---- SDA -------------------+
       |                            |
       +---- SCL -------------------+
                                    |
                              Logic Analyzer
                                    |
                                    v
                              Decode I2C
                                    |
                   +----------------+----------------+
                   |                |                |
                   v                v                v
                 START           ADDRESS            ACK
```

---

# 35. I2C Test Checklist

```text
[ ] I2C device selected
[ ] Device voltage verified
[ ] SDA connected
[ ] SCL connected
[ ] GND connected
[ ] Pull-up resistors checked
[ ] I2C pin mapping verified
[ ] Device Tree configured
[ ] Pinmux configured
[ ] Linux I2C driver loaded
[ ] /dev/i2c-X exists
[ ] i2cdetect identifies bus
[ ] I2C device address detected
[ ] Register read tested
[ ] Register write tested if supported
[ ] Logic analyzer test completed
[ ] Kernel logs checked
```

---

# 36. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── i2c_pin_map.md
│   │
│   └── schematics/
│       └── i2c/
│           └── i2c_test_circuit.md
│
├── device-tree/
│   └── i2c/
│       ├── bbb-i2c.dts
│       ├── bbb-i2c.dtsi
│       └── README.md
│
├── drivers/
│   └── i2c/
│       └── README.md
│
└── tests/
    └── i2c/
        ├── i2c_detect_test.sh
        ├── i2c_read_test.sh
        ├── i2c_write_test.sh
        └── README.md
```

---

# 37. Complete I2C Bring-Up

```text
                    I2C Hardware
                         |
                         v
                    I2C Pin Map
                         |
                         v
                    Test Circuit
                         |
                         v
                    SDA / SCL
                         |
                         v
                    Pull-Ups
                         |
                         v
                    Pinmux
                         |
                         v
                   Device Tree
                         |
                         v
                    I2C Driver
                         |
                         v
                    I2C Core
                         |
                         v
                    /dev/i2c-X
                         |
                         v
                    I2C Device
                         |
                         v
                  User Application
```

---

# 38. Final Objective

The purpose of this test circuit is to validate the complete I2C path on
the BeagleBone Black:

```text
I2C Sensor / EEPROM
        ↓
SDA / SCL
        ↓
AM335x I2C Controller
        ↓
Linux I2C Driver
        ↓
Linux I2C Core
        ↓
/dev/i2c-X
        ↓
User-Space Application
```

The hardware test should confirm both the **electrical I2C bus** and the
**Linux software path**.

````

**File:**

```text
beaglebone-black/hardware/schematics/i2c/i2c_test_circuit.md
````

