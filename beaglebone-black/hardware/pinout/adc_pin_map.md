````markdown
# BeagleBone Black ADC Pin Map

## 1. Overview

The BeagleBone Black uses the **AM335x** processor, which provides an
ADC through the TI **TSC/ADC subsystem**.

The ADC inputs are available on the **P9 expansion header**.

> **Important:** BeagleBone Black ADC inputs are **3.3 V maximum**.
> Do not apply 5 V directly to an ADC input.

---

## 2. ADC Pin Mapping

| ADC Channel | BeagleBone Pin | Signal | Function |
|---|---|---|---|
| AIN0 | P9.39 | AIN0 | Analog Input 0 |
| AIN1 | P9.40 | AIN1 | Analog Input 1 |
| AIN2 | P9.37 | AIN2 | Analog Input 2 |
| AIN3 | P9.38 | AIN3 | Analog Input 3 |
| AIN4 | P9.33 | AIN4 | Analog Input 4 |
| AIN5 | P9.36 | AIN5 | Analog Input 5 |
| AIN6 | P9.35 | AIN6 | Analog Input 6 |
| AIN7 | P9.34 | AIN7 | Analog Input 7 |

---

## 3. Power and Ground

For an external analog sensor:

| BeagleBone Pin | Function |
|---|---|
| P9.1 / P9.2 | GND |
| P9.3 / P9.4 | 3.3 V |
| P9.39 | AIN0 |

Example:

```text
              BeagleBone Black
             +------------------+
             |                  |
Sensor VCC --| P9.3  3.3V       |
Sensor GND --| P9.1  GND        |
Sensor OUT --| P9.39 AIN0       |
             |                  |
             +------------------+
````

---

## 4. ADC Channels

### AIN0

```text
ADC Channel : AIN0
Header Pin  : P9.39
```

### AIN1

```text
ADC Channel : AIN1
Header Pin  : P9.40
```

### AIN2

```text
ADC Channel : AIN2
Header Pin  : P9.37
```

### AIN3

```text
ADC Channel : AIN3
Header Pin  : P9.38
```

### AIN4

```text
ADC Channel : AIN4
Header Pin  : P9.33
```

### AIN5

```text
ADC Channel : AIN5
Header Pin  : P9.36
```

### AIN6

```text
ADC Channel : AIN6
Header Pin  : P9.35
```

### AIN7

```text
ADC Channel : AIN7
Header Pin  : P9.34
```

---

## 5. ADC Voltage Range

The ADC input should remain within the board's supported analog input
range.

```text
Minimum : 0 V
Maximum : 1.8 V
```

Therefore, **do not assume that 3.3 V is a safe ADC input voltage**.
For the BeagleBone Black ADC inputs, the AM335x ADC input range is
approximately **0–1.8 V**.

For a 3.3 V sensor output, use an appropriate voltage divider or signal
conditioning circuit.

Example:

```text
3.3 V Sensor Output
        |
        R1
        |
        +--------> AIN0
        |
        R2
        |
       GND
```

---

## 6. Voltage Divider Example

For a 3.3 V maximum sensor output, a resistor divider can reduce the
voltage to the ADC-safe range.

Example:

```text
Sensor OUT
   |
  10K
   |
   +-------- AIN0
   |
  10K
   |
  GND
```

Output:

```text
Vout = Vin × R2 / (R1 + R2)

For Vin = 3.3 V:

Vout = 3.3 × 10K / (10K + 10K)
     = 1.65 V
```

This keeps the ADC input below approximately 1.8 V.

---

## 7. Linux ADC Verification

Check IIO devices:

```bash
ls /sys/bus/iio/devices/
```

Example:

```text
iio:device0
```

Check the ADC device:

```bash
cat /sys/bus/iio/devices/iio:device0/name
```

Find ADC channels:

```bash
ls /sys/bus/iio/devices/iio:device0/
```

Look for entries such as:

```text
in_voltage0_raw
in_voltage1_raw
in_voltage2_raw
...
```

Read an ADC channel:

```bash
cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw
```

---

## 8. ADC Driver Flow

```text
             Analog Sensor
                   |
                   v
              AIN0-AIN7
                   |
                   v
          AM335x ADC / TSC
                   |
                   v
             ADC Driver
                   |
                   v
               IIO Core
                   |
                   v
          /sys/bus/iio/devices
                   |
                   v
           User Application
```

---

## 9. Device Tree Relationship

The ADC hardware is enabled/configured through the Device Tree.

Typical flow:

```text
bbb-adc.dts
     |
     v
bbb-adc.dtsi
     |
     v
Device Tree Compiler
     |
     v
.dtb
     |
     v
Linux Kernel
     |
     v
ADC Driver
     |
     v
IIO subsystem
```

Project files:

```text
beaglebone-black/
└── device-tree/
    └── adc/
        ├── bbb-adc.dts
        ├── bbb-adc.dtsi
        └── README.md
```

---

## 10. ADC Test Setup

For basic testing, use a controlled voltage source.

```text
Voltage Source
      |
      +-------- AIN0 (P9.39)
      |
     GND
      |
      +-------- P9.1 GND
```

Test points:

```text
0.0 V
0.5 V
1.0 V
1.5 V
```

Record:

| Input Voltage | Raw ADC Value | Converted Voltage | Result |
| ------------: | ------------: | ----------------: | ------ |
|         0.0 V |               |                   |        |
|         0.5 V |               |                   |        |
|         1.0 V |               |                   |        |
|         1.5 V |               |                   |        |

---

## 11. ADC Pin Test Checklist

```text
[ ] AIN0 detected
[ ] AIN1 detected
[ ] AIN2 detected
[ ] AIN3 detected
[ ] AIN4 detected
[ ] AIN5 detected
[ ] AIN6 detected
[ ] AIN7 detected

[ ] Device Tree enabled
[ ] IIO device available
[ ] Raw ADC value readable
[ ] 0 V test completed
[ ] Known voltage test completed
[ ] Repeated sampling tested
[ ] Voltage remains within ADC limits
[ ] No kernel errors
```

---

## 12. Important Hardware Rules

```text
1. Never apply 5 V directly to an ADC input.
2. Keep the ADC input within the AM335x-supported range.
3. Connect the sensor ground to BeagleBone ground.
4. Use voltage-divider circuitry for higher-voltage signals.
5. Avoid floating ADC inputs during testing.
6. Verify the sensor output with a multimeter before connecting it.
7. Use appropriate signal conditioning for noisy analog sources.
```

---

## 13. Quick Reference

```text
+---------+----------+----------+
| Channel | BBB Pin  | Signal   |
+---------+----------+----------+
| AIN0    | P9.39    | Analog 0 |
| AIN1    | P9.40    | Analog 1 |
| AIN2    | P9.37    | Analog 2 |
| AIN3    | P9.38    | Analog 3 |
| AIN4    | P9.33    | Analog 4 |
| AIN5    | P9.36    | Analog 5 |
| AIN6    | P9.35    | Analog 6 |
| AIN7    | P9.34    | Analog 7 |
+---------+----------+----------+
```

---

## 14. Project Integration

This file documents the physical ADC connections for the device-driver
project.

```text
hardware/pinout/adc_pin_map.md
              |
              v
device-tree/adc/
              |
              v
drivers/adc/
              |
              v
tests/adc/
              |
              v
User Application
```

**ADC hardware → Pin Mapping → Device Tree → Linux Driver → IIO →
Testing → Application**

```
```

