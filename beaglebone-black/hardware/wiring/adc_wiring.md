# `adc_wiring.md`

````markdown
# BeagleBone Black ADC Wiring

## 1. Overview

This document describes the wiring required to test the onboard ADC of
the BeagleBone Black.

The BeagleBone Black uses the AM335x SoC, which provides an integrated
12-bit ADC.

The ADC can be used for:

- Potentiometer testing
- Analog sensor interfaces
- Battery voltage monitoring
- Light sensors
- Temperature sensors
- Analog voltage measurement
- Embedded Linux ADC driver testing

---

## 2. ADC Signal Flow

```text
Analog Sensor
      |
      v
Analog Voltage
      |
      v
BeagleBone Black ADC Pin
      |
      v
AM335x ADC
      |
      v
Linux IIO Framework
      |
      v
/sys/bus/iio/devices/
      |
      v
User Application
````

---

## 3. Required Components

For a basic ADC test:

```text
1 × BeagleBone Black
1 × 10 kΩ potentiometer
Jumper wires
Breadboard
```

Optional:

```text
Multimeter
Oscilloscope
Analog sensor
```

---

## 4. ADC Pin Reference

Use the project ADC pin map:

```text
hardware/pinout/adc_pin_map.md
```

The BeagleBone Black exposes multiple ADC inputs on the expansion
headers.

Typical ADC inputs include:

```text
AIN0
AIN1
AIN2
AIN3
AIN4
AIN5
AIN6
```

Use the exact header pin mapping documented in:

```text
hardware/pinout/adc_pin_map.md
```

---

# 5. Important ADC Voltage Limit

The BeagleBone Black ADC inputs are **3.3 V maximum** inputs.

Do not apply a voltage greater than the supported ADC input range.

```text
ADC Input
   |
   +---- 0 V minimum
   |
   +---- 3.3 V maximum
```

For an external sensor producing a higher voltage, use an appropriate
voltage divider or signal-conditioning circuit.

---

# 6. Basic ADC Wiring

For a potentiometer:

```text
              10 kΩ Potentiometer

                 +3.3 V
                   |
                   |
                +-----+
                |     |
                | POT |
                |     |
                +-----+
                   |
                   +----------> ADC Input
                   |
                Wiper
                   |
                   |
                   +----------> AINx

                   |
                   |
                  GND
```

More clearly:

```text
BBB 3.3V
   |
   |
  [ POT ]
   |
   +----------> AINx
   |
  [ POT ]
   |
  GND
```

The potentiometer wiper should connect to the selected ADC input.

---

# 7. Potentiometer Wiring

A standard 10 kΩ potentiometer has three terminals:

```text
        Potentiometer

        Terminal 1
             |
             +---- 3.3 V

        Terminal 2
             |
             +---- ADC Input

        Terminal 3
             |
             +---- GND
```

Turning the potentiometer changes the voltage at the wiper.

Therefore:

```text
Potentiometer Position
        |
        v
Wiper Voltage
        |
        v
ADC Input
        |
        v
ADC Digital Value
```

---

# 8. ADC Wiring Table

| Potentiometer | BeagleBone Black |
| ------------- | ---------------- |
| Terminal 1    | 3.3 V            |
| Wiper         | AINx             |
| Terminal 3    | GND              |

Where `AINx` is the ADC input selected from:

```text
hardware/pinout/adc_pin_map.md
```

---

# 9. Voltage Range

With the potentiometer connected between 3.3 V and GND:

```text
Wiper Position       ADC Voltage

Minimum              ~0 V
      |
      v
      |
Middle               ~1.65 V
      |
      v
      |
Maximum              ~3.3 V
```

Conceptually:

```text
3.3 V
  |
  |\
  | \
  |  \
  |   \
  |    \
  |     \
  |      \
  |       \
  0 V -----+
```

---

# 10. ADC Test Circuit

```text
                    BeagleBone Black

                     3.3 V
                       |
                       |
                    +------+
                    |      |
                    | 10K  |
                    | POT  |
                    |      |
                    +------+
                       |
                       |
                       +-----------> AINx
                       |
                     WIPER
                       |
                       |
                    +------+
                    |      |
                    | 10K  |
                    | POT  |
                    |      |
                    +------+
                       |
                       |
                      GND
```

The wiper provides a variable analog voltage to the ADC input.

---

# 11. Simple Voltage Source Test

Instead of a potentiometer, a controlled voltage source can be used.

```text
Voltage Source
      |
      +----------> AINx
      |
      +----------> GND
```

Example test points:

```text
0.0 V
0.5 V
1.0 V
1.5 V
2.0 V
2.5 V
3.0 V
3.3 V
```

Never exceed the ADC input voltage limit.

---

# 12. Voltage Divider Test

For sensors producing a voltage higher than the ADC input range,
use a voltage divider.

```text
Sensor Output
      |
     R1
      |
      +----------> AINx
      |
     R2
      |
     GND
```

The output voltage is:

```text
Vout = Vin × R2 / (R1 + R2)
```

The divider must be designed so that:

```text
Vout <= ADC maximum input voltage
```

---

# 13. Example Voltage Divider

Suppose:

```text
Vin = 5 V
R1  = 10 kΩ
R2  = 10 kΩ
```

Then:

```text
Vout = 5 × 10 / (10 + 10)

Vout = 2.5 V
```

Therefore:

```text
5 V Sensor
    |
   R1
    |
    +-------> AINx
    |
   R2
    |
   GND
```

The ADC sees approximately:

```text
2.5 V
```

---

# 14. Analog Sensor Wiring

For a 0–3.3 V analog sensor:

```text
Sensor                BeagleBone Black

VCC  ----------------> 3.3 V
GND  ----------------> GND
OUT  ----------------> AINx
```

Complete flow:

```text
+----------------+
| Analog Sensor  |
|                |
| VCC -----------|------ 3.3V
| GND -----------|------ GND
| OUT -----------|------ AINx
+----------------+
```

---

# 15. Common Ground

The sensor and BeagleBone Black must share a common ground.

```text
Sensor GND
     |
     |
     +------------ BBB GND
```

Without a common reference, ADC readings may be incorrect or unstable.

---

# 16. ADC Pin Configuration

Before wiring the circuit:

```text
1. Select ADC channel
2. Check header pin
3. Check pinmux
4. Check Device Tree
5. Verify ADC driver
6. Verify voltage range
7. Connect sensor
```

Project Device Tree files:

```text
device-tree/adc/
├── bbb-adc.dts
├── bbb-adc.dtsi
└── README.md
```

---

# 17. ADC Linux Driver Flow

```text
ADC Hardware
     |
     v
AM335x ADC Controller
     |
     v
ADC Driver
     |
     v
Linux IIO Framework
     |
     v
IIO ADC Channel
     |
     v
/ sys / bus / iio
     |
     v
User Application
```

---

# 18. Check ADC Device

After booting Linux:

```bash
ls /sys/bus/iio/devices/
```

Example:

```text
iio:device0
```

Check:

```bash
ls /sys/bus/iio/devices/iio:device0/
```

You may see channel files such as:

```text
in_voltage0_raw
in_voltage1_raw
in_voltage2_raw
```

The exact channel names depend on the kernel/device-tree configuration.

---

# 19. Read ADC Raw Value

Example:

```bash
cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw
```

Example result:

```text
2048
```

The raw ADC value depends on the ADC resolution and configuration.

---

# 20. ADC Conversion Concept

For a 12-bit ADC:

```text
ADC Resolution = 12 bits

Possible values:

0 → 4095
```

Conceptually:

```text
0 V
 |
 v
ADC = 0

1.65 V
 |
 v
ADC ≈ middle-scale

3.3 V
 |
 v
ADC ≈ 4095
```

The exact voltage conversion depends on the configured/reference
voltage and driver representation.

---

# 21. ADC Test Procedure

## Step 1 — Power Off

Disconnect the board power before changing wiring.

## Step 2 — Select ADC Channel

Check:

```text
hardware/pinout/adc_pin_map.md
```

## Step 3 — Connect Potentiometer

```text
Pot Terminal 1 → 3.3 V
Pot Wiper      → AINx
Pot Terminal 3 → GND
```

## Step 4 — Boot Linux

## Step 5 — Check IIO

```bash
ls /sys/bus/iio/devices/
```

## Step 6 — Identify ADC Device

```bash
ls /sys/bus/iio/devices/iio:device0/
```

## Step 7 — Read Raw ADC

```bash
cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw
```

## Step 8 — Rotate Potentiometer

Observe the ADC value changing.

---

# 22. Expected ADC Behavior

Potentiometer at minimum:

```text
Voltage ≈ 0 V
ADC Value ≈ minimum
```

Potentiometer at center:

```text
Voltage ≈ 1.65 V
ADC Value ≈ middle of range
```

Potentiometer at maximum:

```text
Voltage ≈ 3.3 V
ADC Value ≈ maximum
```

Conceptually:

```text
Pot Position       ADC Reading

0%                 LOW
 |
25%                |
 |
50%                MEDIUM
 |
75%                |
 |
100%               HIGH
```

---

# 23. ADC Wiring With Multimeter

A multimeter can verify the voltage before reading the ADC.

Connect:

```text
Multimeter +
     |
     +------ ADC Input

Multimeter -
     |
     +------ GND
```

Rotate the potentiometer and verify:

```text
0 V → 3.3 V
```

Then compare the measured voltage with the ADC reading.

---

# 24. ADC Debugging

If the ADC reading does not change:

```text
ADC Reading Not Changing
          |
          +--> Check ADC Channel
          |
          +--> Check Pin Mapping
          |
          +--> Check Wiring
          |
          +--> Check GND
          |
          +--> Check 3.3 V
          |
          +--> Check Device Tree
          |
          +--> Check IIO Driver
          |
          +--> Check Pinmux
```

---

# 25. ADC Wiring Troubleshooting

### Problem: Always reads zero

Check:

```text
[ ] Correct ADC channel
[ ] Wiper connected to ADC
[ ] Sensor connected to GND
[ ] ADC driver enabled
[ ] Device Tree enabled
```

### Problem: Always reads maximum

Check:

```text
[ ] ADC input not shorted to 3.3 V
[ ] Potentiometer wiring
[ ] Sensor output
[ ] Correct ADC channel
```

### Problem: Unstable readings

Check:

```text
[ ] Common GND
[ ] Short wiring
[ ] Stable sensor supply
[ ] No floating input
[ ] Appropriate signal conditioning
```

---

# 26. ADC Hardware Checklist

```text
[ ] BeagleBone Black powered
[ ] Correct ADC channel selected
[ ] Correct header pin identified
[ ] 3.3 V supply verified
[ ] GND connected
[ ] ADC input voltage within safe range
[ ] Potentiometer wired correctly
[ ] Wiper connected to ADC
[ ] Device Tree configured
[ ] ADC driver loaded
[ ] IIO device available
[ ] Raw ADC value readable
[ ] ADC value changes with input voltage
```

---

# 27. ADC Wiring Checklist

```text
                    ADC Wiring

                     3.3 V
                       |
                       v
                 +-----------+
                 | 10K POT   |
                 +-----------+
                       |
                     Wiper
                       |
                       v
                      AINx
                       |
                       v
                  AM335x ADC
                       |
                       v
                  Linux IIO
                       |
                       v
                User Application
                       ^
                       |
                      GND
                       |
                 BeagleBone GND
```

---

# 28. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── adc_pin_map.md
│   │
│   └── wiring/
│       └── adc_wiring.md
│
├── hardware/
│   └── schematics/
│       └── adc/
│           └── adc_test_circuit.md
│
├── device-tree/
│   └── adc/
│       ├── bbb-adc.dts
│       ├── bbb-adc.dtsi
│       └── README.md
│
├── drivers/
│   └── adc/
│       └── README.md
│
└── tests/
    └── adc/
        ├── adc_read_test.sh
        └── README.md
```

---

# 29. Complete ADC Bring-Up

```text
                    ADC Sensor
                        |
                        v
                  Analog Voltage
                        |
                        v
                  ADC Input Pin
                        |
                        v
                  AM335x ADC
                        |
                        v
                   ADC Driver
                        |
                        v
                 Linux IIO Core
                        |
                        v
                 IIO ADC Channel
                        |
                        v
              /sys/bus/iio/devices/
                        |
                        v
                 User Application
```

---

# 30. Final Test Objective

The objective of this wiring test is to validate the complete ADC path:

```text
Analog Sensor
      ↓
ADC Pin
      ↓
AM335x ADC
      ↓
Linux ADC Driver
      ↓
IIO Framework
      ↓
Raw ADC Value
      ↓
User Application
```

The recommended first hardware test is a **10 kΩ potentiometer connected
between 3.3 V and GND**, with the wiper connected to the selected ADC
input.

This provides a simple variable 0–3.3 V signal for validating the
complete BeagleBone Black ADC driver and Linux IIO path.

````

**File location:**

```text
beaglebone-black/hardware/wiring/adc_wiring.md
````

