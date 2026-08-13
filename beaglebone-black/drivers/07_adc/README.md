# BeagleBone Black ADC Driver

## Overview

This directory contains an example Linux kernel ADC driver for the
BeagleBone Black using the Linux **IIO (Industrial I/O) framework**.

The driver demonstrates:

- Platform driver registration
- Device Tree matching
- ADC register mapping
- ADC channel selection
- ADC conversion start
- ADC conversion status polling
- Raw ADC data reading
- Linux IIO integration
- Mutex protection
- Kernel module build and loading

---

## Directory Structure

```text
07_adc/
├── adc_driver.c
├── adc_driver.h
├── Makefile
└── README.md
Driver Architecture
                  Device Tree
                       |
                       v
                ADC Platform Device
                       |
                       v
                  adc_probe()
                       |
              +--------+--------+
              |                 |
              v                 v
        Get MEM Resource      I/O Map
              |                 |
              +--------+--------+
                       |
                       v
                  ADC Registers
                       |
                       v
                  Linux IIO Core
                       |
                       v
        /sys/bus/iio/devices/iio:deviceX
                       |
                       v
              in_voltage*_raw
                       |
                       v
                 User Space
ADC Read Flow
User Application
       |
       v
IIO sysfs
       |
       v
read_raw()
       |
       v
adc_read_channel()
       |
       v
Select ADC Channel
       |
       v
Start Conversion
       |
       v
Wait for Conversion Done
       |
       v
Read ADC Data Register
       |
       v
Return Raw ADC Value
Device Tree

The driver expects:

compatible = "bbb,adc-test";

Example:

adc_test {
    compatible = "bbb,adc-test";
    reg = <0x00000000 0x00001000>;
    status = "okay";
};

The actual register address must be replaced with the ADC peripheral
address for the target hardware.

Build

Run:

make

Expected module:

adc_driver.ko

Verify:

ls -l adc_driver.ko
Load Driver
sudo insmod adc_driver.ko

Check:

lsmod | grep adc_driver

Check kernel messages:

dmesg | grep -i adc
IIO Device

After successful driver probing:

ls /sys/bus/iio/devices/

Example:

iio:device0

Check the device name:

cat /sys/bus/iio/devices/iio:device0/name
ADC Channels

The example driver provides eight voltage channels:

in_voltage0_raw
in_voltage1_raw
in_voltage2_raw
in_voltage3_raw
in_voltage4_raw
in_voltage5_raw
in_voltage6_raw
in_voltage7_raw

Check:

ls /sys/bus/iio/devices/iio:device0/
Read ADC Value

Read channel 0:

cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw

Read channel 1:

cat /sys/bus/iio/devices/iio:device0/in_voltage1_raw

Read all available channels:

make read
ADC Resolution

The example driver uses:

Resolution = 12 bits

Therefore:

Minimum raw value = 0
Maximum raw value = 4095

The raw ADC value represents the sampled input according to the ADC
reference voltage and hardware configuration.

For example, with a 1.8 V reference:

Voltage = Raw Value × 1.8 / 4095

The actual reference voltage must match the hardware.

Makefile Commands

Build:

make

Load:

make load

Unload:

make unload

Status:

make status

IIO information:

make info

Read ADC channels:

make read

Test:

make test

Kernel logs:

make logs

Clean:

make clean
Debugging

Check loaded module:

lsmod | grep adc_driver

Check IIO devices:

ls -l /sys/bus/iio/devices/

Check ADC device name:

cat /sys/bus/iio/devices/iio:device0/name

Check ADC channels:

find /sys/bus/iio/devices/ -name "in_voltage*_raw"

Check kernel logs:

dmesg | grep -i adc
Important ADC Registers

The example driver uses:

ADC_CONTROL_REG
ADC_STATUS_REG
ADC_CHANNEL_SELECT_REG
ADC_DATA_REG

Conceptually:

CONTROL
   |
   +--> Start Conversion

CHANNEL_SELECT
   |
   +--> Select ADC Input

STATUS
   |
   +--> Conversion Complete

DATA
   |
   +--> ADC Raw Result
Important Note

The register offsets and control/status bits in this example are
generic placeholders.

For a production BeagleBone Black/AM335x implementation, the ADC
driver should use the actual AM335x ADC/TSC hardware register map,
clock configuration, Device Tree properties, interrupts, and Linux
IIO conventions.

The BeagleBone Black's ADC is normally handled through the existing
Linux IIO/ADC support rather than creating a second raw register driver
for hardware already owned by an existing kernel driver.


### Final structure

```text
beaglebone-black/
└── drivers/
    └── 07_adc/
        ├── adc_driver.c
        ├── adc_driver.h
        ├── Makefile
        └── README.md

This keeps the ADC driver consistent with your previous 06_uart and 05_spi driver directories.
