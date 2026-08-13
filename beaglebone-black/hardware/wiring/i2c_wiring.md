# `i2c_wiring.md`

````markdown
# BeagleBone Black I2C Wiring

## 1. Overview

This document describes the hardware wiring required to test I2C
(Inter-Integrated Circuit) communication on the BeagleBone Black.

I2C is a two-wire synchronous communication protocol commonly used for:

- Temperature sensors
- EEPROMs
- RTCs
- IMUs
- Accelerometers
- Gyroscopes
- GPIO expanders
- ADCs
- DACs
- Display controllers
- Power-management ICs

The basic communication path is:

```text
User Application
       |
       v
Linux I2C Framework
       |
       v
I2C Controller Driver
       |
       v
AM335x I2C Controller
       |
       v
SDA / SCL
       |
       v
I2C Sensor / EEPROM / Peripheral
````

---

# 2. I2C Bus Signals

I2C uses two main signals:

```text
SDA → Serial Data
SCL → Serial Clock
```

Both lines are shared by all devices on the same I2C bus.

```text
                 I2C MASTER
              BeagleBone Black
                     |
             +-------+-------+
             |               |
            SDA             SCL
             |               |
             |               |
             +-------+-------+
                     |
              I2C BUS
                     |
        +------------+------------+
        |                         |
        v                         v
   I2C Sensor                 EEPROM
```

---

# 3. Required Components

For a basic I2C test:

```text
1 × BeagleBone Black
1 × I2C sensor / EEPROM / RTC module
Jumper wires
Breadboard
```

If the I2C module does not already contain pull-up resistors:

```text
2 × pull-up resistors
Typical values: 2.2 kΩ – 10 kΩ
```

The correct value depends on bus speed, capacitance, voltage, and the
connected devices.

---

# 4. I2C Pin Selection

Before wiring the device:

```text
hardware/pinout/i2c_pin_map.md
```

Use this file to identify:

```text
I2C controller
SDA pin
SCL pin
Header location
Pinmux mode
```

Do not assume that every header pin is available as I2C.

The AM335x pins are multiplexed between several peripheral functions.

---

# 5. Basic I2C Wiring

Typical connection:

```text
BeagleBone Black          I2C Device
----------------          ----------

3.3 V  -----------------> VCC

GND    -----------------> GND

I2C SDA ----------------> SDA

I2C SCL ----------------> SCL
```

Complete:

```text
             BeagleBone Black
                    |
          +---------+---------+
          |         |         |
         3.3V      SDA       SCL
          |         |         |
          v         v         v
       +-------------------------+
       |       I2C Device        |
       |                         |
       | VCC   GND   SDA   SCL  |
       +-------------------------+
```

---

# 6. I2C Wiring Table

| BeagleBone Black | I2C Peripheral |
| ---------------- | -------------- |
| 3.3 V            | VCC            |
| GND              | GND            |
| SDA              | SDA            |
| SCL              | SCL            |

The exact SDA/SCL header pins must be taken from:

```text
hardware/pinout/i2c_pin_map.md
```

---

# 7. I2C Pull-Up Resistors

I2C uses open-drain/open-collector style signaling.

Therefore SDA and SCL normally require pull-up resistors.

```text
             3.3 V
               |
          +----+----+
          |         |
         R1        R2
       Pull-up   Pull-up
          |         |
          |         |
         SDA       SCL
          |         |
          |         |
          +----+----+
               |
         I2C Devices
```

Typical arrangement:

```text
3.3 V
 |
 +----[R1]---- SDA
 |
 +----[R2]---- SCL
```

---

# 8. Why Pull-Ups Are Required

I2C devices normally pull the bus lines LOW rather than actively driving
them HIGH.

Conceptually:

```text
              3.3 V
                |
              Pull-up
                |
                +-------- SDA
                |
             I2C Device
                |
             Open-drain
                |
               GND
```

Device releases the line:

```text
SDA = HIGH
```

Device pulls the line LOW:

```text
SDA = LOW
```

---

# 9. I2C Multiple Devices

Multiple I2C devices can share the same SDA and SCL lines.

```text
                    3.3 V
                      |
                  Pull-ups
                      |
             +--------+--------+
             |                 |
            SDA               SCL
             |                 |
      +------+-----------------+------+
      |             I2C BUS           |
      +------+-----------------+------+
             |                 |
             |                 |
        +----+----+       +----+----+
        | Sensor  |       | EEPROM  |
        +---------+       +---------+
```

Each device normally has a unique I2C address.

Example:

```text
Sensor  → 0x48
EEPROM  → 0x50
RTC     → 0x68
```

The actual address depends on the device and its hardware address
configuration.

---

# 10. I2C Address

I2C communication uses a device address.

Example:

```text
BeagleBone
     |
     | Address = 0x48
     v
Temperature Sensor
```

The Linux I2C tools can be used to discover devices on the bus.

---

# 11. I2C Bus Topology

Recommended topology:

```text
                 BeagleBone
                     |
                  SDA/SCL
                     |
          +----------+----------+
          |          |          |
          v          v          v
       Sensor      EEPROM       RTC
```

Avoid unnecessarily long wires and large numbers of stubs.

Keep the I2C bus wiring short, especially at higher bus speeds.

---

# 12. Common Ground

The BeagleBone Black and I2C peripheral should share an appropriate
ground/reference.

```text
BBB GND ---------------- I2C Device GND
```

Without a proper electrical reference, communication may be unreliable.

---

# 13. Voltage Compatibility

Before connecting an I2C device, check:

```text
[ ] Device VCC
[ ] SDA voltage level
[ ] SCL voltage level
[ ] Pull-up voltage
[ ] BeagleBone GPIO voltage limits
```

Do not connect a 5 V I2C pull-up directly to a BeagleBone I2C pin.

For a 5 V I2C peripheral, use an appropriate level-shifting solution
when required.

---

# 14. I2C Level Shifter

For different logic-voltage domains:

```text
BeagleBone
   |
   | 3.3 V I2C
   v
+----------------+
| I2C Level      |
| Shifter        |
+----------------+
   |
   | 5 V I2C
   v
I2C Peripheral
```

The level shifter must be designed for bidirectional I2C signals.

---

# 15. I2C Device Tree

Project Device Tree files:

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
I2C Controller Driver
      |
      v
Linux I2C Core
      |
      v
I2C Client Driver
      |
      v
I2C Sensor / EEPROM
```

---

# 16. I2C Device Tree Concept

An I2C bus may contain child devices.

Conceptually:

```text
I2C Controller
     |
     +---- Sensor @ address
     |
     +---- EEPROM @ address
     |
     +---- RTC @ address
```

The Device Tree describes:

```text
I2C controller
I2C bus status
Pinmux
Child device
I2C address
Compatible string
Interrupt GPIO
Power supplies
```

---

# 17. Check I2C Interfaces

After booting Linux:

```bash
i2cdetect -l
```

Example:

```text
i2c-0
i2c-1
```

The exact controller numbers depend on the board's kernel/device-tree
configuration.

---

# 18. Check Kernel Logs

Use:

```bash
dmesg | grep -i i2c
```

You can also check:

```bash
dmesg | grep -i -E "i2c|omap|sensor"
```

This can help identify controller initialization and client-driver
probe messages.

---

# 19. Install I2C Tools

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install i2c-tools
```

Verify:

```bash
i2cdetect --version
```

---

# 20. Scan I2C Bus

Example:

```bash
sudo i2cdetect -y 1
```

Example output:

```text
     0 1 2 3 4 5 6 7 8 9 a b c d e f
00: -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- 48 -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- 68 -- -- -- --
70: -- -- -- -- -- -- -- -- -- -- -- --
```

This indicates devices responding at addresses such as:

```text
0x48
0x68
```

The actual result depends on the connected hardware.

---

# 21. Important Note About I2C Scanning

`i2cdetect` sends transactions to addresses to determine whether
devices respond.

Some I2C devices do not respond safely or predictably to generic probe
transactions.

Therefore:

```text
Use i2cdetect carefully.
```

For production hardware, consult the peripheral datasheet before
scanning unknown devices.

---

# 22. Read I2C Device Data

For simple devices, Linux I2C tools can be used to inspect registers.

Example:

```bash
sudo i2cget -y 1 0x48 0x00
```

Conceptually:

```text
Bus       = 1
Address   = 0x48
Register  = 0x00
```

The correct command depends entirely on the peripheral's register
protocol.

Do not use random `i2cget`/`i2cset` commands on an unknown device.

---

# 23. Write I2C Register

For devices supporting register writes:

```bash
sudo i2cset -y 1 0x48 0x01 0x80
```

Conceptually:

```text
I2C Bus
   |
   v
Address 0x48
   |
   v
Register 0x01
   |
   v
Data 0x80
```

Only perform register writes according to the device datasheet.

---

# 24. I2C Transfer Flow

Example read operation:

```text
START
  |
  v
Slave Address
  |
  v
Write Register Address
  |
  v
Repeated START
  |
  v
Slave Address + READ
  |
  v
Read Data
  |
  v
STOP
```

The exact transaction depends on the peripheral protocol.

---

# 25. I2C Hardware Flow

```text
CPU
 |
 v
Linux I2C Framework
 |
 v
I2C Controller Driver
 |
 v
AM335x I2C Controller
 |
 +----------+
 |          |
 v          v
SDA        SCL
 |          |
 +----+-----+
      |
      v
I2C Peripheral
```

---

# 26. I2C Sensor Example

Example sensor wiring:

```text
Sensor
+----------------+
|                |
| VCC ---------- +------ 3.3 V
| GND ---------- +------ GND
| SDA ---------- +------ BBB SDA
| SCL ---------- +------ BBB SCL
|                |
+----------------+
```

---

# 27. I2C EEPROM Example

Example EEPROM connection:

```text
BBB SDA ---------------- SDA
BBB SCL ---------------- SCL
BBB 3.3V --------------- VCC
BBB GND ---------------- GND
```

Address pins, if present:

```text
A0
A1
A2
```

can be used to select the device address according to the EEPROM
datasheet.

---

# 28. I2C Address Conflict

Two devices should not normally use the same address on the same bus.

Example:

```text
Sensor 1 → 0x48
Sensor 2 → 0x49
```

Good:

```text
0x48
0x49
0x50
```

Potential conflict:

```text
Sensor 1 → 0x48
Sensor 2 → 0x48
```

If two devices require the same fixed address, consider:

```text
Address-select pins
I2C multiplexer
Separate I2C bus
```

depending on the hardware.

---

# 29. I2C Pull-Up Calculation

The pull-up resistor must satisfy the I2C electrical requirements.

Conceptually:

```text
Lower R
  |
  +--> Faster rise time
  |
  +--> Higher LOW-level current

Higher R
  |
  +--> Slower rise time
  |
  +--> Lower current
```

The correct resistor depends on:

```text
Bus capacitance
Bus speed
Supply voltage
Device sink-current capability
Number of devices
PCB/wire length
```

Use the I2C specification and device datasheets when selecting the
value.

---

# 30. I2C Speed

Common I2C modes include:

```text
100 kHz   → Standard-mode
400 kHz   → Fast-mode
```

Other I2C speed modes exist, but support depends on the controller and
peripheral.

For initial board bring-up, 100 kHz is often a useful starting point.

---

# 31. Check I2C Bus Speed

The configured bus speed is normally controlled by the kernel/device
tree and controller driver.

For debugging, verify the configured controller settings and kernel
logs rather than assuming the bus is operating at a particular speed.

An oscilloscope or logic analyzer can directly verify SCL frequency.

---

# 32. I2C Logic Analyzer Test

Connect a logic analyzer to:

```text
SDA
SCL
GND
```

Example:

```text
BeagleBone
     |
     +------ SDA ------ Logic Analyzer
     |
     +------ SCL ------ Logic Analyzer
     |
     +------ GND ------ Logic Analyzer
```

You should observe:

```text
SCL: _|‾|_|‾|_|‾|_|‾|_

SDA: ___|‾‾|___|‾|____
```

A decoder can display:

```text
START
ADDRESS
READ / WRITE
ACK / NACK
DATA
STOP
```

---

# 33. I2C ACK

After a successful byte transfer, the receiver can acknowledge the
transaction.

Conceptually:

```text
Master → Address + R/W
             |
             v
          Slave
             |
             v
            ACK
```

If the slave does not respond:

```text
NACK
```

may occur.

---

# 34. I2C Debugging Flow

```text
                  I2C Failure
                      |
                      v
               Is /dev/i2c-* present?
                 /            \
               NO              YES
               |                |
               v                v
        Check Device Tree    Run i2cdetect
                                |
                                v
                         Device detected?
                          /           \
                        NO             YES
                        |               |
                        v               v
                  Check wiring      Test register
                  Check pull-ups    read/write
                  Check voltage          |
                  Check address           v
                                   Check driver
```

---

# 35. I2C No Device Detected

If:

```bash
sudo i2cdetect -y 1
```

does not show the expected address, check:

```text
[ ] SDA wiring
[ ] SCL wiring
[ ] GND
[ ] VCC
[ ] Device address
[ ] Pull-up resistors
[ ] Pinmux
[ ] I2C controller enabled
[ ] Correct I2C bus number
[ ] Device power/reset
```

---

# 36. SDA/SCL Reversed

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

Always verify the module's pin labels.

---

# 37. I2C Bus Stuck LOW

If SDA or SCL remains LOW:

```text
Possible causes:

[ ] Device holding line LOW
[ ] Missing pull-up
[ ] Incorrect voltage
[ ] Short circuit
[ ] Incorrect device state
[ ] Bus transaction interrupted
[ ] Hardware fault
```

Use a logic analyzer or oscilloscope to determine which device is
holding the line.

---

# 38. I2C Wiring Troubleshooting

### Problem: No I2C controller

Check:

```text
[ ] Device Tree
[ ] Kernel I2C support
[ ] Pinmux
[ ] Controller status
[ ] Kernel logs
```

### Problem: Controller exists but device is missing

Check:

```text
[ ] SDA
[ ] SCL
[ ] VCC
[ ] GND
[ ] Pull-ups
[ ] I2C address
[ ] Device reset
```

### Problem: Communication is unreliable

Check:

```text
[ ] Pull-up value
[ ] Bus capacitance
[ ] Wire length
[ ] Bus speed
[ ] Noise
[ ] Voltage compatibility
```

---

# 39. I2C Test Procedure

## Step 1 — Power Off

Disconnect the board before changing wiring.

## Step 2 — Select I2C Bus

Check:

```text
hardware/pinout/i2c_pin_map.md
```

## Step 3 — Connect Device

```text
3.3V → VCC
GND  → GND
SDA  → SDA
SCL  → SCL
```

## Step 4 — Verify Pull-Ups

Check whether the module already contains pull-up resistors.

## Step 5 — Boot Linux

## Step 6 — Check I2C Devices

```bash
i2cdetect -l
```

## Step 7 — Scan the Bus

```bash
sudo i2cdetect -y <bus>
```

## Step 8 — Verify Address

Example:

```text
0x48
```

## Step 9 — Test Device

Use the appropriate driver or device-specific I2C commands.

## Step 10 — Verify With Logic Analyzer

If necessary:

```text
SDA
SCL
GND
```

---

# 40. I2C Device Driver Flow

For a Linux kernel device driver:

```text
Device Tree
     |
     v
I2C Controller
     |
     v
I2C Core
     |
     v
I2C Client Device
     |
     v
Driver Probe()
     |
     v
Register Initialization
     |
     v
Data Transfer
     |
     v
User Application
```

---

# 41. I2C Driver Communication

A kernel driver typically uses the Linux I2C APIs to communicate with
the device.

Conceptually:

```text
Driver
  |
  +---- Write Register
  |
  +---- Read Register
  |
  +---- Configure Sensor
  |
  +---- Read Sensor Data
  |
  +---- Handle Interrupt
```

The exact APIs depend on the driver implementation and device protocol.

---

# 42. I2C Interrupt Wiring

Some I2C sensors provide an interrupt pin.

Example:

```text
Sensor
+-----------------------+
|                       |
| SDA ------------------+---- BBB SDA
| SCL ------------------+---- BBB SCL
| INT ------------------+---- BBB GPIO
| VCC ------------------+---- 3.3 V
| GND ------------------+---- GND
|                       |
+-----------------------+
```

The interrupt path becomes:

```text
Sensor
  |
  v
INT
  |
  v
GPIO
  |
  v
Linux IRQ
  |
  v
Sensor Driver
```

---

# 43. Complete I2C Hardware Test

```text
                     BeagleBone Black

                     3.3 V
                       |
                       |
                +------+------+
                |             |
             Pull-up       Pull-up
                |             |
                v             v
               SDA           SCL
                |             |
                +------+------+
                       |
                 I2C BUS
                       |
             +---------+---------+
             |                   |
             v                   v
        I2C Sensor            EEPROM
```

---

# 44. I2C Wiring Checklist

```text
[ ] I2C controller selected
[ ] SDA pin verified
[ ] SCL pin verified
[ ] Pinmux configured
[ ] VCC connected
[ ] GND connected
[ ] SDA connected
[ ] SCL connected
[ ] Pull-up resistors verified
[ ] Logic voltage verified
[ ] Device address verified
[ ] I2C controller visible
[ ] /dev/i2c-* available
[ ] Device detected
[ ] Driver loaded
[ ] Data transfer verified
[ ] Logic analyzer test completed if required
```

---

# 45. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── i2c_pin_map.md
│   │
│   └── wiring/
│       └── i2c_wiring.md
│
├── hardware/
│   └── schematics/
│       └── i2c/
│           └── i2c_test_circuit.md
│
├── device-tree/
│   ├── i2c/
│   │   ├── bbb-i2c.dts
│   │   ├── bbb-i2c.dtsi
│   │   └── README.md
│   │
│   └── overlays/
│       └── bbb-i2c-overlay.dts
│
├── drivers/
│   └── i2c/
│       └── README.md
│
└── tests/
    └── i2c/
        ├── i2c_scan_test.sh
        ├── i2c_read_test.sh
        ├── i2c_write_test.sh
        └── README.md
```

---

# 46. Complete I2C Bring-Up

```text
                         I2C Device
                              |
                              v
                        SDA / SCL Bus
                              |
                              v
                    AM335x I2C Controller
                              |
                              v
                       I2C Controller
                           Driver
                              |
                              v
                         Linux I2C Core
                              |
                              v
                        I2C Client Driver
                              |
                              v
                         User Application
```

---

# 47. Final Test Objective

The objective of this wiring test is to validate the complete I2C path:

```text
I2C Peripheral
      ↓
SDA / SCL
      ↓
AM335x I2C Controller
      ↓
Linux I2C Driver
      ↓
Linux I2C Core
      ↓
I2C Client Driver
      ↓
User Application
```

Recommended validation sequence:

```text
1. Verify I2C pin mapping
2. Verify pinmux
3. Connect SDA and SCL
4. Connect VCC and GND
5. Verify pull-up resistors
6. Verify voltage compatibility
7. Boot Linux
8. Check i2c interfaces
9. Scan the bus
10. Verify device address
11. Load/test the device driver
12. Read/write device registers
13. Verify SDA/SCL with a logic analyzer
```

> **Important:** I2C is an open-drain bus and normally requires
> appropriate pull-up resistors. Always verify voltage compatibility
> before connecting an external I2C device to the BeagleBone Black.

````

**File location:**

```text
beaglebone-black/hardware/wiring/i2c_wiring.md
````

