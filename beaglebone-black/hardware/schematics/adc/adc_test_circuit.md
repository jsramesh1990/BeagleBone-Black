# `adc_test_circuit.md`

````markdown
# BeagleBone Black ADC Test Circuit

## 1. Overview

This document describes a basic hardware test circuit for validating the
ADC input of the BeagleBone Black.

The test circuit provides a controlled analog voltage to the ADC input and
allows the Linux ADC interface to be verified.

```text
Analog Voltage Source
        |
        v
   Voltage Divider
        |
        v
   BBB ADC Input
        |
        v
    ADC Driver
        |
        v
   Linux IIO Subsystem
        |
        v
    User Space
````

---

## 2. Important ADC Safety

The BeagleBone Black ADC inputs are **3.3 V domain inputs** and the ADC
input voltage must not exceed the board's specified ADC input range.

For a safe test, use a regulated 3.3 V supply and a voltage divider rather
than connecting an unknown analog source directly to the ADC pin.

> Never apply 5 V directly to an ADC input.

---

## 3. Basic Test Circuit

A simple potentiometer can be used as a variable analog voltage source.

```text
              3.3 V
                |
                |
             [ POT ]
                |
                +-----------> ADC INPUT
                |
             [ GND ]
                |
               GND
```

A more practical representation:

```text
              BBB 3.3V
                 |
                 |
              +--+--+
              | POT |
              +--+--+
                 |
                 +------------ ADCx
                 |
                 |
              BBB GND
```

The potentiometer wiper is connected to the ADC input.

---

## 4. Recommended Test Setup

```text
             BeagleBone Black
          +---------------------+
          |                     |
          |   3.3V              |
          |     |               |
          |     v               |
          |   +-----+           |
          |   | POT |           |
          |   +--+--+           |
          |      |              |
          |      +----------+   |
          |                 |   |
          |              ADC IN |
          |                     |
          |   GND <-------------+
          |                     |
          +---------------------+
```

Connections:

| Component           | Connection |
| ------------------- | ---------- |
| Potentiometer VCC   | BBB 3.3 V  |
| Potentiometer GND   | BBB GND    |
| Potentiometer Wiper | ADC input  |

---

## 5. Voltage Divider Test

A fixed resistor divider can also be used.

```text
       3.3 V
         |
        R1
         |
         +----------> ADC INPUT
         |
        R2
         |
        GND
```

The ADC voltage is determined by the resistor values.

For example:

```text
R1 = 10 kΩ
R2 = 10 kΩ
```

The midpoint voltage is approximately:

```text
ADC voltage ≈ 1.65 V
```

---

## 6. Voltage Divider Formula

For a divider:

```text
Vin
 |
 R1
 |
 +------ Vout
 |
 R2
 |
GND
```

The output voltage is:

```text
Vout = Vin × R2 / (R1 + R2)
```

Example:

```text
Vin = 3.3 V
R1  = 10 kΩ
R2  = 10 kΩ
```

Therefore:

```text
Vout ≈ 1.65 V
```

---

## 7. ADC Test Flow

```text
3.3 V Supply
     |
     v
Voltage Divider / Potentiometer
     |
     v
ADC Input Pin
     |
     v
AM335x ADC
     |
     v
Linux IIO Driver
     |
     v
/sys/bus/iio/devices/
     |
     v
User-Space Application
```

---

## 8. Hardware Components

Recommended components:

```text
1 × BeagleBone Black
1 × 10 kΩ potentiometer
2 × 10 kΩ resistors
Jumper wires
Breadboard
3.3 V supply from BBB
Multimeter
```

For a simple potentiometer test, the two fixed resistors are not
required.

---

## 9. Multimeter Verification

Before connecting the ADC input, measure the voltage with a multimeter.

Example:

```text
Multimeter

     +--------+
     |        |
     |  1.65V |
     |        |
     +--------+
       |    |
       |    |
      ADC  GND
```

Verify:

```text
ADC voltage < permitted ADC input voltage
```

before starting the software test.

---

## 10. ADC Pin Connection

The exact ADC input/header mapping should be taken from:

```text
hardware/pinout/adc_pin_map.md
```

Example project flow:

```text
adc_pin_map.md
      |
      v
Select ADC Channel
      |
      v
Wire Test Circuit
      |
      v
Enable ADC in Device Tree
      |
      v
Boot Linux
      |
      v
Check IIO Device
      |
      v
Read ADC Value
```

---

## 11. Check Linux IIO Device

After booting Linux:

```bash
ls /sys/bus/iio/devices/
```

You may see:

```text
iio:device0
```

Check:

```bash
ls /sys/bus/iio/devices/iio:device0/
```

Look for ADC channel files such as:

```text
in_voltage0_raw
in_voltage1_raw
in_voltage2_raw
```

The available channels depend on the active kernel/device-tree
configuration.

---

## 12. Read ADC Raw Value

Example:

```bash
cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw
```

Example output:

```text
2048
```

The actual value depends on:

* ADC resolution
* Input voltage
* Kernel driver
* ADC channel
* Hardware configuration

---

## 13. Check ADC Driver

Check kernel messages:

```bash
dmesg | grep -i adc
```

Also check IIO:

```bash
dmesg | grep -i iio
```

Check loaded modules if ADC is configured as a module:

```bash
lsmod | grep -i adc
```

---

## 14. ADC Device Tree

The ADC peripheral must be enabled through the appropriate Device Tree
configuration.

Project files:

```text
beaglebone-black/
└── device-tree/
    └── adc/
        ├── bbb-adc.dts
        ├── bbb-adc.dtsi
        └── README.md
```

Conceptually:

```text
bbb-adc.dts
     |
     v
ADC Pinmux
     |
     v
ADC Controller
     |
     v
Linux IIO Driver
     |
     v
ADC Channel
```

---

## 15. ADC Software Architecture

```text
User Application
       |
       v
     IIO
       |
       v
IIO ADC Driver
       |
       v
AM335x ADC
       |
       v
ADC Input Pin
       |
       v
Analog Signal
```

---

## 16. Test Procedure

### Step 1 — Power Off

Disconnect the BeagleBone Black from external wiring.

### Step 2 — Build Circuit

Connect:

```text
3.3V → Potentiometer → ADC
GND  → Potentiometer
```

### Step 3 — Verify Voltage

Use a multimeter to verify the ADC input voltage.

### Step 4 — Boot Linux

Start the BeagleBone Black.

### Step 5 — Check IIO

```bash
ls /sys/bus/iio/devices/
```

### Step 6 — Identify Channel

```bash
find /sys/bus/iio/devices/ -name "in_voltage*_raw"
```

### Step 7 — Read ADC

```bash
cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw
```

### Step 8 — Change Potentiometer

Rotate the potentiometer.

### Step 9 — Read ADC Again

```bash
cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw
```

The ADC reading should change with the input voltage.

---

## 17. Expected Result

For example:

```text
Potentiometer Position       ADC Reading
------------------------------------------------
Minimum                      Near minimum
25%                          Low
50%                          Mid-scale
75%                          High
Maximum                      Near maximum
```

The exact raw values depend on the ADC resolution and configuration.

---

## 18. ADC Debugging

If the ADC value does not change:

```text
ADC Not Changing
      |
      +--> Check ADC wiring
      |
      +--> Check GND
      |
      +--> Check 3.3V
      |
      +--> Check ADC input voltage
      |
      +--> Check Device Tree
      |
      +--> Check IIO driver
      |
      +--> Check ADC channel
      |
      +--> Check pinmux
      |
      +--> Check kernel logs
```

---

## 19. Multimeter + ADC Verification

A useful validation method is:

```text
              +----------------+
              |   Potentiometer|
              +-------+--------+
                      |
                      |
             +--------+--------+
             |                 |
             v                 v
         Multimeter         BBB ADC
             |                 |
             +--------+--------+
                      |
                     GND
```

Compare:

```text
Measured Voltage
       vs.
ADC Reported Value
```

This validates both the hardware and Linux ADC driver.

---

## 20. ADC Test Checklist

```text
[ ] ADC pin identified
[ ] Correct header pin verified
[ ] 3.3 V supply verified
[ ] GND connected
[ ] ADC input voltage verified
[ ] Voltage is within ADC limits
[ ] Device Tree configured
[ ] ADC driver enabled
[ ] IIO device detected
[ ] ADC channel detected
[ ] Raw value read
[ ] Potentiometer test completed
[ ] Multimeter comparison completed
[ ] Kernel logs checked
```

---

## 21. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   └── schematics/
│       └── adc/
│           └── adc_test_circuit.md
│
├── hardware/
│   └── pinout/
│       └── adc_pin_map.md
│
├── device-tree/
│   └── adc/
│       ├── bbb-adc.dts
│       ├── bbb-adc.dtsi
│       └── README.md
│
└── drivers/
    └── adc/
        └── README.md
```

---

## 22. Complete ADC Bring-Up

```text
                Hardware
                   |
                   v
             ADC Pin Map
                   |
                   v
             Test Circuit
                   |
                   v
              Pinmux Setup
                   |
                   v
             Device Tree
                   |
                   v
             Linux IIO
                   |
                   v
              ADC Driver
                   |
                   v
             ADC Channel
                   |
                   v
          /sys/bus/iio/devices/
                   |
                   v
           User-Space Test
                   |
                   v
          Multimeter Validation
```

---

## 23. Final Objective

The purpose of this schematic is to provide a **safe, repeatable ADC
hardware test** for the BeagleBone Black device-driver project.

The test validates the complete path:

```text
Analog Signal
      ↓
ADC Hardware
      ↓
Device Tree
      ↓
Linux IIO Framework
      ↓
ADC Driver
      ↓
Sysfs / IIO Interface
      ↓
User Application
```

```
```

