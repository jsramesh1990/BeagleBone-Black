# BeagleBone Black ADC Device Tree

## 1. Overview

This directory contains the Device Tree configuration used to enable
and test the ADC functionality on the BeagleBone Black.

The BeagleBone Black uses the Texas Instruments AM335x SoC, which
contains an integrated ADC/TSC controller.

The Linux ADC functionality is normally exposed through the
Industrial I/O (IIO) subsystem.

---

## 2. Directory Structure

```text
adc/
├── bbb-adc.dts
├── bbb-adc.dtsi
└── README.md


Yes. For your current folder:

```text
beaglebone-black/device-tree/adc/
├── bbb-adc.dts
├── bbb-adc.dtsi
└── README.md
```

here are the **complete files** you can create. These are written for the BeagleBone Black / AM335x ADC and are intended as a starting point for your Linux device-driver project. The exact DTS include path/properties can vary with the kernel/BSP version, so verify against the kernel tree you build against.

### 1. `bbb-adc.dts`

```dts
/*
 * BeagleBone Black ADC Device Tree Test
 *
 * Platform : BeagleBone Black
 * SoC      : TI AM335x
 *
 * Purpose:
 *   Enable the ADC/TSC hardware and provide a test Device Tree
 *   configuration for Linux IIO ADC functionality.
 */

/dts-v1/;

/include/ "am335x-boneblack.dts"
/include/ "bbb-adc.dtsi"

/ {
	model = "TI AM335x BeagleBone Black - ADC Test";
	compatible = "ti,am335x-bone-black", "ti,am335x";
};
```

---

### 2. `bbb-adc.dtsi`

```dts
/*
 * BeagleBone Black ADC configuration
 *
 * AM335x ADC/TSC controller configuration.
 *
 * The ADC is normally exposed through the Linux IIO subsystem.
 */

&tscadc {
	status = "okay";
};
```

---

### 3. `README.md`

````markdown
# BeagleBone Black ADC Device Tree

## 1. Overview

This directory contains the Device Tree configuration used to enable
and test the ADC functionality on the BeagleBone Black.

The BeagleBone Black uses the Texas Instruments AM335x SoC, which
contains an integrated ADC/TSC controller.

The Linux ADC functionality is normally exposed through the
Industrial I/O (IIO) subsystem.

---

## 2. Directory Structure

```text
adc/
├── bbb-adc.dts
├── bbb-adc.dtsi
└── README.md
````

### Files

| File           | Description                    |
| -------------- | ------------------------------ |
| `bbb-adc.dts`  | Main ADC Device Tree test file |
| `bbb-adc.dtsi` | Reusable ADC configuration     |
| `README.md`    | ADC Device Tree documentation  |

---

## 3. Hardware

### Target Board

```text
Board : BeagleBone Black
SoC   : TI AM335x
CPU   : ARM Cortex-A8
```

The AM335x provides an integrated ADC/TSC peripheral.

---

## 4. ADC Architecture

The complete ADC data flow is:

```text
Analog Signal
      |
      v
ADC Input
      |
      v
AM335x ADC/TSC
      |
      v
ADC Driver
      |
      v
Linux IIO Framework
      |
      v
/sys/bus/iio/devices/
      |
      v
User-Space Application
```

---

## 5. Device Tree Flow

```text
bbb-adc.dts
     |
     +---- am335x-boneblack.dts
     |
     +---- bbb-adc.dtsi
     |
     v
Device Tree Compiler
     |
     v
bbb-adc.dtb
     |
     v
Bootloader
     |
     v
Linux Kernel
     |
     v
ADC Device
     |
     v
IIO ADC Driver
```

---

## 6. Device Tree Configuration

The ADC configuration is located in:

```text
bbb-adc.dtsi
```

The main ADC node is enabled using:

```dts
&tscadc {
    status = "okay";
};
```

This tells the Linux kernel that the ADC/TSC hardware should be
enabled.

---

## 7. Build Device Tree

The Device Tree Compiler (`dtc`) is required.

Check:

```bash
dtc --version
```

Example compilation:

```bash
dtc -I dts -O dtb -o bbb-adc.dtb bbb-adc.dts
```

This produces:

```text
bbb-adc.dtb
```

---

## 8. Important Note About Kernel Device Trees

The BeagleBone Black Device Tree is normally part of the Linux kernel
source tree.

For example:

```text
arch/arm/boot/dts/
```

The exact Device Tree filename and include structure depend on the
Linux kernel version.

Before compiling, verify the available BeagleBone Black DTS files:

```bash
find arch/arm/boot/dts -iname "*boneblack*"
```

Also search for the ADC node:

```bash
grep -R "tscadc" arch/arm/boot/dts/
```

---

## 9. Kernel Configuration

The ADC functionality is normally provided through the Linux IIO
subsystem.

Check the kernel configuration:

```bash
zcat /proc/config.gz | grep CONFIG_IIO
```

If `/proc/config.gz` is not available:

```bash
grep CONFIG_IIO /boot/config-$(uname -r)
```

Important configuration options depend on the kernel version.

Typical IIO-related configuration:

```text
CONFIG_IIO
```

The AM335x ADC driver configuration should also be enabled in the
kernel configuration used for the target board.

---

## 10. Boot and Verify Device Tree

After deploying the Device Tree and booting the board, check:

```bash
dmesg | grep -i adc
```

Also check:

```bash
dmesg | grep -i iio
```

Check the Device Tree:

```bash
ls /proc/device-tree/
```

Check the IIO subsystem:

```bash
ls /sys/bus/iio/devices/
```

Expected output may contain:

```text
iio:device0
```

The actual device number can vary.

---

## 11. Find ADC Device

Run:

```bash
ls -l /sys/bus/iio/devices/
```

Example:

```text
iio:device0
```

Check the device name:

```bash
cat /sys/bus/iio/devices/iio:device0/name
```

The exact name depends on the kernel driver and kernel version.

---

## 12. Find Available ADC Channels

Run:

```bash
ls /sys/bus/iio/devices/iio:device0/
```

Look for files such as:

```text
in_voltage0_raw
in_voltage1_raw
in_voltage2_raw
...
```

The exact channel names depend on the active Device Tree configuration
and kernel version.

---

## 13. Read ADC Raw Value

Example:

```bash
cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw
```

Possible output:

```text
1234
```

The raw ADC value represents the digital conversion result.

---

## 14. ADC Resolution

The ADC resolution depends on the AM335x ADC configuration.

For a 12-bit ADC:

```text
Minimum = 0
Maximum = 4095
```

Therefore:

```text
2^12 = 4096 levels
```

---

## 15. Voltage Calculation

For a 12-bit ADC:

```text
Voltage = Raw_Value × Reference_Voltage / 4095
```

For example, assuming a 1.8 V reference:

```text
Raw Value = 2048

Voltage = 2048 × 1.8 / 4095

Voltage ≈ 0.9 V
```

Always verify the actual ADC input range and reference configuration
for the specific hardware before applying a voltage calculation.

---

## 16. ADC Driver Architecture

```text
                  USER SPACE
                      |
                      v
          /sys/bus/iio/devices/
                      |
                      v
               Linux IIO Core
                      |
                      v
              ADC Driver
                      |
                      v
             AM335x TSCADC
                      |
                      v
                ADC Hardware
                      |
                      v
               Analog Input
```

---

## 17. Device Tree to Driver Matching

The general Linux flow is:

```text
Device Tree
     |
     v
ADC Hardware Node
     |
     v
Kernel Device Creation
     |
     v
Driver Matching
     |
     v
ADC Driver Probe
     |
     v
IIO Device Registration
     |
     v
/sys/bus/iio/devices/
```

---

## 18. Check Driver

Check loaded modules:

```bash
lsmod
```

Search for ADC/IIO-related modules:

```bash
lsmod | grep -Ei "iio|adc|ti"
```

Check kernel messages:

```bash
dmesg | grep -Ei "adc|iio|tscadc"
```

---

## 19. Debugging

### Check Kernel Messages

```bash
dmesg | grep -i adc
```

### Check IIO Devices

```bash
ls /sys/bus/iio/devices/
```

### Check Device Tree

```bash
find /proc/device-tree -iname "*adc*" -o -iname "*tsc*"
```

### Check Kernel Configuration

```bash
grep CONFIG_IIO /boot/config-$(uname -r)
```

---

## 20. Common Problems

### Problem 1: No IIO Device

Check:

```bash
ls /sys/bus/iio/devices/
```

If no device appears:

```bash
dmesg | grep -Ei "adc|iio|tscadc"
```

Check whether the ADC node is enabled:

```dts
status = "okay";
```

---

### Problem 2: Driver Not Loaded

Check:

```bash
lsmod
```

Then:

```bash
dmesg | grep -Ei "adc|iio"
```

Verify that the required kernel configuration and driver are enabled.

---

### Problem 3: Incorrect Device Tree

Check the Device Tree source:

```bash
grep -R "tscadc" arch/arm/boot/dts/
```

Compare the configuration with the Device Tree used by your exact
kernel version.

---

### Problem 4: ADC Channel Missing

Check:

```bash
ls /sys/bus/iio/devices/iio:device0/
```

If the expected `in_voltageX_raw` file is missing, verify the ADC
channel configuration and the kernel's AM335x ADC Device Tree binding.

---

## 21. Testing Procedure

The basic test sequence is:

```text
Power ON Board
      |
      v
Linux Boot
      |
      v
Device Tree Loaded
      |
      v
ADC Driver Probe
      |
      v
IIO Device Registered
      |
      v
Check /sys/bus/iio/
      |
      v
Read ADC Channel
      |
      v
Verify Raw Value
      |
      v
Calculate Voltage
      |
      v
Repeat Test
```

---

## 22. Functional Test

Run:

```bash
ls /sys/bus/iio/devices/
```

Find the ADC device:

```bash
ls /sys/bus/iio/devices/iio:device0/
```

Read the channel:

```bash
cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw
```

Apply a known analog input and repeat the measurement.

---

## 23. Test with Different Input Voltages

A useful test is:

```text
Input Voltage
      |
      v
ADC
      |
      v
Raw ADC Value
      |
      v
Compare Expected vs Actual
```

Record:

| Input | Raw Value | Calculated Voltage | Result |
| ----: | --------: | -----------------: | ------ |
|   0 V |           |                    |        |
| 0.3 V |           |                    |        |
| 0.6 V |           |                    |        |
| 0.9 V |           |                    |        |
| 1.2 V |           |                    |        |
| 1.5 V |           |                    |        |
| 1.8 V |           |                    |        |

Do not exceed the ADC input voltage limits of the specific board/input.

---

## 24. Driver Development Connection

This Device Tree configuration is part of the larger driver project:

```text
device-tree/adc/
        |
        v
ADC Device Tree
        |
        v
drivers/07_adc/
        |
        v
Linux ADC/IIO Driver
        |
        v
user-space/adc_test/
        |
        v
ADC Functional Test
```

---

## 25. Repository Integration

The complete project contains:

```text
beaglebone-black/
|
+-- device-tree/
|   |
|   +-- adc/
|   +-- can/
|   +-- gpio/
|   +-- i2c/
|   +-- overlays/
|   +-- pwm/
|   +-- spi/
|   +-- uart/
|
+-- drivers/
|   |
|   +-- 01_char_driver/
|   +-- 02_gpio/
|   +-- 03_interrupt/
|   +-- ...
|   +-- 07_adc/
|
+-- user-space/
|   |
|   +-- adc_test/
|   +-- gpio_test/
|   +-- i2c_test/
|   +-- ...
|
+-- tests/
|
+-- docs/
|
+-- scripts/
```

---

## 26. Development Checklist

* [ ] Verify AM335x ADC hardware
* [ ] Verify ADC pins
* [ ] Check kernel Device Tree
* [ ] Create `bbb-adc.dtsi`
* [ ] Create `bbb-adc.dts`
* [ ] Enable ADC node
* [ ] Enable IIO support
* [ ] Build Device Tree
* [ ] Deploy to BeagleBone Black
* [ ] Boot Linux
* [ ] Check `dmesg`
* [ ] Check IIO devices
* [ ] Check ADC channels
* [ ] Read raw ADC value
* [ ] Calculate voltage
* [ ] Test multiple input voltages
* [ ] Document results

---

## 27. Expected Result

A successful configuration should result in:

```text
Device Tree
     |
     v
ADC Enabled
     |
     v
ADC Driver Probed
     |
     v
IIO Device Registered
     |
     v
ADC Channel Available
     |
     v
Raw ADC Value
     |
     v
Voltage Measurement
```

---

## 28. Related Project Sections

ADC Device Tree:

```text
device-tree/adc/
```

ADC Driver:

```text
drivers/07_adc/
```

ADC User-Space Test:

```text
user-space/adc_test/
```

ADC Documentation:

```text
docs/
```

---

## 29. Status

```text
Device Tree Configuration : In Development
ADC Driver                : In Development
IIO Integration           : In Development
User-Space Test           : Planned
Stress Test               : Planned
Documentation             : In Progress
```

---

## 30. Summary

This ADC module demonstrates the complete Linux ADC development flow
on the BeagleBone Black:

```text
AM335x ADC
    ↓
Device Tree
    ↓
Linux Kernel
    ↓
ADC Driver
    ↓
IIO Framework
    ↓
Sysfs
    ↓
User Space
    ↓
ADC Measurement
```

The same architecture will be followed for GPIO, I2C, SPI, UART, PWM,
CAN and the other peripherals in this project.

````

### Your ADC folder is now

```text
beaglebone-black/device-tree/adc/
├── bbb-adc.dts
├── bbb-adc.dtsi
└── README.md
````

One important thing: **don't copy an arbitrary `tscadc` node from a different kernel version and assume it will compile.** For this project, the next step should be to inspect the **exact Linux kernel Device Tree already used by your BeagleBone Black**, then we can make `bbb-adc.dts`/`.dtsi` match it precisely.

