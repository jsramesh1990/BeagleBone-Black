# `04_device_tree.md`

Create:

```text
beaglebone-black/docs/04_device_tree.md
```

````markdown
# 04 - Device Tree on BeagleBone Black

## Table of Contents

- [1. Overview](#1-overview)
- [2. Why Device Tree](#2-why-device-tree)
- [3. Device Tree Architecture](#3-device-tree-architecture)
- [4. BeagleBone Black Hardware](#4-beaglebone-black-hardware)
- [5. Device Tree Source Files](#5-device-tree-source-files)
- [6. DTS vs DTSI](#6-dts-vs-dtsi)
- [7. DTB](#7-dtb)
- [8. Device Tree Compilation](#8-device-tree-compilation)
- [9. Device Tree Boot Flow](#9-device-tree-boot-flow)
- [10. Device Tree Node Structure](#10-device-tree-node-structure)
- [11. compatible Property](#11-compatible-property)
- [12. status Property](#12-status-property)
- [13. reg Property](#13-reg-property)
- [14. interrupts Property](#14-interrupts-property)
- [15. pinctrl](#15-pinctrl)
- [16. Pin Multiplexing on BeagleBone Black](#16-pin-multiplexing-on-beaglebone-black)
- [17. GPIO Device Tree](#17-gpio-device-tree)
- [18. UART Device Tree](#18-uart-device-tree)
- [19. I2C Device Tree](#19-i2c-device-tree)
- [20. SPI Device Tree](#20-spi-device-tree)
- [21. PWM Device Tree](#21-pwm-device-tree)
- [22. ADC Device Tree](#22-adc-device-tree)
- [23. CAN Device Tree](#23-can-device-tree)
- [24. Device Tree Overlays](#24-device-tree-overlays)
- [25. Overlay Architecture](#25-overlay-architecture)
- [26. BeagleBone Overlay Files](#26-beaglebone-overlay-files)
- [27. Compiling Overlays](#27-compiling-overlays)
- [28. Loading Device Tree Overlays](#28-loading-device-tree-overlays)
- [29. Checking Device Tree at Runtime](#29-checking-device-tree-at-runtime)
- [30. Device Tree and Driver Matching](#30-device-tree-and-driver-matching)
- [31. Device Tree Resources](#31-device-tree-resources)
- [32. Device Tree GPIO References](#32-device-tree-gpio-references)
- [33. Device Tree I2C Example](#33-device-tree-i2c-example)
- [34. Device Tree SPI Example](#34-device-tree-spi-example)
- [35. Device Tree UART Example](#35-device-tree-uart-example)
- [36. Device Tree PWM Example](#36-device-tree-pwm-example)
- [37. Device Tree ADC Example](#37-device-tree-adc-example)
- [38. Device Tree CAN Example](#38-device-tree-can-example)
- [39. Device Tree Debugging](#39-device-tree-debugging)
- [40. Common Device Tree Errors](#40-common-device-tree-errors)
- [41. Project Directory Mapping](#41-project-directory-mapping)
- [42. Recommended Development Flow](#42-recommended-development-flow)
- [43. Interview Explanation](#43-interview-explanation)
- [44. Summary](#44-summary)

---

# 1. Overview

Device Tree is a hardware description mechanism used by Linux to
describe hardware that cannot be automatically discovered by the
kernel.

For the BeagleBone Black, Device Tree describes peripherals of the
TI AM335x SoC and the board-level connections.

This project uses Device Tree to configure:

```text
GPIO
UART
I2C
SPI
PWM
ADC
CAN
Pin Multiplexing
Interrupts
Clocks
DMA
````

The basic relationship is:

```text
+----------------------+
| BeagleBone Hardware  |
+----------+-----------+
           |
           v
+----------------------+
| Device Tree          |
| DTS / DTSI / Overlay |
+----------+-----------+
           |
           v
+----------------------+
| Linux Kernel         |
+----------+-----------+
           |
           v
+----------------------+
| Device / Driver      |
| Matching             |
+----------------------+
           |
           v
+----------------------+
| Hardware Driver      |
+----------------------+
```

---

# 2. Why Device Tree

Without Device Tree, platform-specific hardware information would
have to be hard-coded into kernel source code.

Device Tree separates:

```text
Hardware Description
```

from:

```text
Driver Implementation
```

For example, the driver can remain generic while Device Tree tells it:

```text
Which peripheral?
Which address?
Which IRQ?
Which GPIO?
Which clock?
Which pins?
Which hardware configuration?
```

This makes the driver reusable.

---

# 3. Device Tree Architecture

The overall architecture is:

```text
                 Device Tree Source
                       |
                 .dts / .dtsi
                       |
                       v
                Device Tree Compiler
                       |
                       v
                      .dtb
                       |
                       v
                    U-Boot
                       |
                       v
                 Linux Kernel
                       |
                       v
                OF Device Layer
                       |
             +---------+---------+
             |                   |
             v                   v
          Device              Driver
             |                   |
             +---------+---------+
                       |
                     Match
                       |
                       v
                     probe()
```

Where:

```text
DTS  = Device Tree Source
DTSI = Device Tree Include
DTB  = Device Tree Blob
DTC  = Device Tree Compiler
```

---

# 4. BeagleBone Black Hardware

The BeagleBone Black is based on the TI AM335x SoC.

Important peripherals for this project include:

```text
+--------------------------------------+
|          BeagleBone Black            |
|                                      |
|              AM335x                  |
|                                      |
|  GPIO                                |
|  UART                                |
|  I2C                                 |
|  SPI                                 |
|  PWM                                 |
|  ADC                                 |
|  CAN                                 |
|                                      |
+--------------------------------------+
```

The board exposes many of these peripherals through its expansion
headers.

The exact pins used must always be checked against the board
documentation and the active Device Tree configuration before wiring
external hardware.

---

# 5. Device Tree Source Files

Device Tree source files normally use:

```text
.dts
.dtsi
```

Example project structure:

```text
beaglebone-black/
|
+-- device-tree/
|   |
|   +-- adc/
|   |   +-- bbb-adc.dts
|   |   +-- bbb-adc.dtsi
|   |   +-- README.md
|   |
|   +-- can/
|   |   +-- bbb-can.dts
|   |   +-- bbb-can.dtsi
|   |   +-- README.md
|   |
|   +-- gpio/
|   |   +-- bbb-gpio.dts
|   |   +-- bbb-gpio.dtsi
|   |   +-- README.md
|   |
|   +-- i2c/
|   |   +-- bbb-i2c.dts
|   |   +-- bbb-i2c.dtsi
|   |   +-- README.md
|   |
|   +-- overlays/
|   |   +-- bbb-gpio-overlay.dts
|   |   +-- bbb-i2c-overlay.dts
|   |   +-- bbb-spi-overlay.dts
|   |   +-- bbb-uart-overlay.dts
|   |   +-- README.md
|   |
|   +-- pwm/
|   |   +-- bbb-pwm.dts
|   |   +-- bbb-pwm.dtsi
|   |   +-- README.md
|   |
|   +-- spi/
|   |   +-- bbb-spi.dts
|   |   +-- bbb-spi.dtsi
|   |   +-- README.md
|   |
|   +-- uart/
|       +-- bbb-uart.dts
|       +-- bbb-uart.dtsi
|       +-- README.md
```

---

# 6. DTS vs DTSI

## `.dts`

A DTS file is normally a top-level Device Tree source file.

Example:

```dts
/dts-v1/;

/include/ "bbb-gpio.dtsi"

/ {
    model = "BeagleBone Black";
};
```

## `.dtsi`

A DTSI file is normally an included fragment containing reusable
hardware definitions.

Example:

```dts
&gpio1 {
    status = "okay";
};
```

Conceptually:

```text
bbb-gpio.dts
      |
      +---- include
               |
               v
          bbb-gpio.dtsi
```

DTSI files are useful for keeping the project organized.

---

# 7. DTB

The Linux kernel does not normally boot directly from the textual
DTS source.

DTS is compiled into a binary Device Tree Blob:

```text
.dts
 |
 | DTC
 v
.dtb
```

Example:

```text
bbb-gpio.dts
      |
      v
bbb-gpio.dtb
```

The DTB is passed to the Linux kernel during boot.

---

# 8. Device Tree Compilation

The Device Tree Compiler is:

```text
dtc
```

Check:

```bash
dtc --version
```

Compile:

```bash
dtc -I dts -O dtb -o bbb-gpio.dtb bbb-gpio.dts
```

Where:

```text
-I dts
    Input format = Device Tree Source

-O dtb
    Output format = Device Tree Blob

-o bbb-gpio.dtb
    Output file
```

For kernel-integrated Device Trees, the Linux kernel build system
usually invokes `dtc` automatically.

---

# 9. Device Tree Boot Flow

A typical embedded Linux boot flow is:

```text
Power ON
   |
   v
Boot ROM
   |
   v
SPL
   |
   v
U-Boot
   |
   +---- Load Kernel
   |
   +---- Load DTB
   |
   +---- Apply configuration / overlays if configured
   |
   v
Linux Kernel
   |
   v
Device Tree Parsing
   |
   v
Create Devices
   |
   v
Driver Matching
   |
   v
probe()
   |
   v
Peripheral Available
```

The Device Tree therefore participates directly in hardware
initialization.

---

# 10. Device Tree Node Structure

A typical node looks like:

```dts
uart1: serial@48022000 {
    compatible = "ti,am3352-uart";
    reg = <0x48022000 0x2000>;
    interrupts = <73>;
    status = "okay";
};
```

Breakdown:

```text
uart1
 |
 +-- label
 |
 +-- serial@48022000
       |
       +-- node name
       |
       +-- @48022000
              |
              +-- unit address
```

Properties:

```text
compatible
reg
interrupts
status
```

---

# 11. compatible Property

`compatible` is one of the most important Device Tree properties.

Example:

```dts
compatible = "ti,am3352-uart";
```

It tells Linux which hardware/driver compatibility description
applies to the node.

Driver:

```c
static const struct of_device_id bbb_of_match[] = {
    {
        .compatible = "ti,am3352-uart",
    },
    { }
};
```

Matching:

```text
Device Tree
     |
     | compatible
     v
"ti,am3352-uart"
     |
     v
Driver match table
     |
     v
probe()
```

---

# 12. status Property

The `status` property controls whether a device is enabled.

Common values:

```dts
status = "okay";
```

and:

```dts
status = "disabled";
```

Example:

```dts
&uart1 {
    status = "okay";
};
```

Disabled:

```dts
&uart1 {
    status = "disabled";
};
```

Typical interpretation:

```text
okay
  |
  v
Device available

disabled
  |
  v
Device not enabled
```

---

# 13. reg Property

`reg` describes a hardware resource such as a memory-mapped register
region.

Example:

```dts
reg = <0x48022000 0x2000>;
```

Conceptually:

```text
Start Address = 0x48022000
Size          = 0x2000
```

The exact interpretation depends on the parent bus's address and
size cell configuration.

A platform driver can retrieve resources through kernel APIs such as:

```c
platform_get_resource()
```

or:

```c
devm_platform_ioremap_resource()
```

---

# 14. interrupts Property

An interrupt property tells Linux which interrupt resource is
associated with a device.

Example:

```dts
interrupts = <73>;
```

The exact format depends on the interrupt controller binding.

Driver code may request the interrupt using:

```c
devm_request_irq()
```

or:

```c
devm_request_threaded_irq()
```

Flow:

```text
Hardware Event
      |
      v
Interrupt Controller
      |
      v
Linux IRQ
      |
      v
Driver IRQ Handler
```

---

# 15. pinctrl

Pin control configures SoC pins for their required peripheral
function.

A pin may support several alternate functions.

Conceptually:

```text
One Physical Pin
      |
      +---- GPIO
      |
      +---- UART
      |
      +---- SPI
      |
      +---- I2C
      |
      +---- PWM
```

The pinctrl subsystem selects the desired function.

Example structure:

```dts
&pinctrl {
    uart1_pins: uart1_pins {
        pinctrl-single,pins = <
            /* pin configuration */
        >;
    };
};
```

Peripheral:

```dts
&uart1 {
    pinctrl-names = "default";
    pinctrl-0 = <&uart1_pins>;
    status = "okay";
};
```

---

# 16. Pin Multiplexing on BeagleBone Black

This is one of the most important topics on the BeagleBone Black.

The AM335x has multiplexed pins.

Therefore, enabling a peripheral is not enough.

You normally need:

```text
Peripheral Enable
        +
Pin Multiplexing
        +
Electrical Configuration
```

Example:

```text
UART
 |
 +-- UART peripheral enabled
 |
 +-- TX pin configured for UART TX
 |
 +-- RX pin configured for UART RX
```

If pinmux is incorrect:

```text
Driver may probe successfully
        |
        v
Hardware still does not work
```

This is a common embedded Linux debugging issue.

---

# 17. GPIO Device Tree

GPIO configuration can reference GPIO controllers and GPIO lines.

Example pattern:

```dts
led {
    compatible = "gpio-leds";

    user_led {
        label = "bbb-user-led";
        gpios = <&gpio1 21 GPIO_ACTIVE_HIGH>;
        default-state = "off";
    };
};
```

Important properties:

```text
gpios
GPIO_ACTIVE_HIGH
GPIO_ACTIVE_LOW
```

The exact GPIO controller and line must match the BeagleBone Black
hardware and your selected pin.

Driver/application flow:

```text
Device Tree
     |
     v
GPIO Controller
     |
     v
GPIO Framework
     |
     v
GPIO Consumer
```

---

# 18. UART Device Tree

A UART node may look conceptually like:

```dts
&uart1 {
    pinctrl-names = "default";
    pinctrl-0 = <&uart1_pins>;
    status = "okay";
};
```

The associated pinctrl configuration determines which physical pins
are connected to UART TX/RX.

Flow:

```text
UART Device Tree
       |
       +---- pinctrl
       |
       +---- status
       |
       v
UART Controller
       |
       v
Serial Core
       |
       v
/dev/tty*
```

Verify available serial devices:

```bash
ls -l /dev/ttyS*
ls -l /dev/ttyO*
```

The exact device naming depends on the kernel/BSP configuration.

---

# 19. I2C Device Tree

An I2C controller can be enabled:

```dts
&i2c1 {
    status = "okay";
};
```

An I2C peripheral can then be described as a child node.

Example:

```dts
&i2c1 {
    status = "okay";

    sensor@48 {
        compatible = "example,my-sensor";
        reg = <0x48>;
    };
};
```

Architecture:

```text
I2C Controller
      |
      v
I2C Bus
      |
      +---- sensor@48
      |
      +---- EEPROM
      |
      +---- Other Device
```

Important:

```text
reg = <0x48>;
```

represents the I2C slave address in this example.

---

# 20. SPI Device Tree

An SPI controller can be enabled:

```dts
&spi0 {
    status = "okay";
};
```

An SPI device is represented as a child node.

Example:

```dts
&spi0 {
    status = "okay";

    flash@0 {
        compatible = "jedec,spi-nor";
        reg = <0>;
        spi-max-frequency = <10000000>;
    };
};
```

Important properties:

```text
compatible
reg
spi-max-frequency
```

Here:

```text
reg = <0>;
```

typically identifies the chip-select index for the SPI device.

---

# 21. PWM Device Tree

PWM configuration generally involves:

```text
PWM Controller
PWM Channel
Pinmux
PWM Consumer
```

Conceptually:

```text
PWM Controller
      |
      +---- Channel 0
      +---- Channel 1
      +---- Channel 2
```

A consumer can reference a PWM:

```dts
pwms = <&ehrpwm1 0 1000000 0>;
```

The exact controller and channel depend on the selected AM335x
peripheral and board pin configuration.

Important concepts:

```text
Period
Duty Cycle
Polarity
Enable
```

---

# 22. ADC Device Tree

The BeagleBone Black's analog inputs are normally exposed through the
AM335x ADC/TSC hardware and the Linux IIO subsystem.

Conceptual flow:

```text
AIN Pin
   |
   v
AM335x ADC
   |
   v
ADC Driver
   |
   v
IIO Framework
   |
   v
IIO Device
```

Check IIO devices:

```bash
ls /sys/bus/iio/devices/
```

Check channels:

```bash
ls /sys/bus/iio/devices/iio:device0/
```

Typical attributes can include:

```text
in_voltage*_raw
in_voltage*_scale
```

The exact channel naming depends on the kernel/device-tree version.

---

# 23. CAN Device Tree

CAN requires both:

```text
CAN Controller
CAN Transceiver
```

Conceptually:

```text
AM335x CAN
    |
    v
CAN Controller
    |
    v
CAN Transceiver
    |
    v
CANH / CANL
```

The Device Tree must configure:

```text
CAN controller
Pinmux
Clock
Status
```

Example pattern:

```dts
&dcan0 {
    pinctrl-names = "default";
    pinctrl-0 = <&dcan0_pins>;
    status = "okay";
};
```

The actual pinctrl definition must correspond to the selected
BeagleBone Black pins.

After boot:

```bash
ip link show
```

You may see:

```text
can0
```

Configure bitrate:

```bash
ip link set can0 type can bitrate 500000
```

Bring interface up:

```bash
ip link set can0 up
```

---

# 24. Device Tree Overlays

A Device Tree Overlay is a fragment that modifies an existing
Device Tree.

Conceptually:

```text
Base DTB
   |
   +------------------+
   |                  |
   v                  v
Overlay 1          Overlay 2
   |                  |
   v                  v
GPIO             UART / I2C / SPI
```

Overlays are useful when hardware configuration needs to be changed
without replacing the entire base Device Tree.

---

# 25. Overlay Architecture

Base Device Tree:

```text
am335x-boneblack.dtb
```

Overlay:

```text
bbb-uart-overlay.dtbo
```

Runtime configuration:

```text
Base DTB
   +
Overlay
   |
   v
Modified Device Tree
   |
   v
Linux Kernel
```

---

# 26. BeagleBone Overlay Files

Your project currently has:

```text
device-tree/overlays/
|
+-- bbb-gpio-overlay.dts
+-- bbb-i2c-overlay.dts
+-- bbb-spi-overlay.dts
+-- bbb-uart-overlay.dts
+-- README.md
```

These should contain valid overlay syntax.

For example:

```dts
/dts-v1/;
/plugin/;
```

An overlay commonly uses:

```text
fragment@
target
__overlay__
```

Example structure:

```dts
/dts-v1/;
/plugin/;

/ {
    compatible = "ti,am335x-bone-black";

    fragment@0 {
        target = <&uart1>;

        __overlay__ {
            status = "okay";
        };
    };
};
```

The exact overlay target must be verified against the Device Tree
used by your selected BeagleBone Black Linux image.

---

# 27. Compiling Overlays

Compile an overlay using:

```bash
dtc -@ -I dts -O dtb \
    -o bbb-uart-overlay.dtbo \
    bbb-uart-overlay.dts
```

The important option is:

```text
-@
```

It enables symbol information required for many overlay workflows.

Output:

```text
bbb-uart-overlay.dtbo
```

---

# 28. Loading Device Tree Overlays

The exact overlay loading mechanism depends on the BeagleBone Black
BSP/image and U-Boot configuration.

First inspect the running system:

```bash
ls /boot/
```

and:

```bash
find /boot -name "*.dtbo"
```

Also inspect:

```bash
ls /sys/firmware/devicetree/base/
```

On systems using U-Boot overlay configuration, overlays may be
specified through boot configuration.

Do not assume that an overlay mechanism from an older BeagleBone
image is identical to a newer Debian/kernel image.

---

# 29. Checking Device Tree at Runtime

Linux exposes the live Device Tree through:

```text
/sys/firmware/devicetree/base/
```

Check:

```bash
ls /sys/firmware/devicetree/base/
```

Check a node:

```bash
ls /sys/firmware/devicetree/base/
```

Read a property:

```bash
cat /sys/firmware/devicetree/base/model
```

For binary properties:

```bash
xxd /sys/firmware/devicetree/base/some-property
```

Another useful location is:

```text
/proc/device-tree/
```

Example:

```bash
ls /proc/device-tree/
```

---

# 30. Device Tree and Driver Matching

This is the most important connection between Device Tree and the
driver.

Device Tree:

```dts
my_device {
    compatible = "bbb,my-device";
    status = "okay";
};
```

Driver:

```c
static const struct of_device_id bbb_of_match[] = {
    {
        .compatible = "bbb,my-device",
    },
    { }
};
```

Kernel:

```text
Device Tree
     |
     v
Create Device
     |
     v
Driver registered
     |
     v
compatible match
     |
     v
probe()
```

If the compatible strings do not match:

```text
probe() will not be called
```

---

# 31. Device Tree Resources

Device Tree can describe many hardware resources.

Common properties include:

```text
compatible
reg
interrupts
clocks
resets
dmas
gpio
gpios
pinctrl-names
pinctrl-0
status
power-domains
regulators
```

Example:

```dts
my_device@48000000 {
    compatible = "bbb,my-device";
    reg = <0x48000000 0x1000>;
    interrupts = <10>;
    status = "okay";
};
```

Conceptually:

```text
Device
 |
 +-- Register Region
 +-- IRQ
 +-- GPIO
 +-- Clock
 +-- Reset
 +-- DMA
 +-- Pinmux
```

---

# 32. Device Tree GPIO References

A GPIO consumer can reference a GPIO controller.

Example:

```dts
led {
    gpios = <&gpio1 21 GPIO_ACTIVE_HIGH>;
};
```

This means conceptually:

```text
GPIO Controller
       |
       v
GPIO Bank
       |
       v
GPIO Line
       |
       v
Consumer
```

For production code, use the GPIO descriptor API in the driver.

Example:

```c
struct gpio_desc *gpio;

gpio = devm_gpiod_get(dev, "led", GPIOD_OUT_LOW);
```

Avoid writing new drivers around the old integer-based GPIO API unless
the target kernel requires it for compatibility.

---

# 33. Device Tree I2C Example

Example project device:

```dts
&i2c1 {
    status = "okay";

    temperature@48 {
        compatible = "bbb,demo-temperature";
        reg = <0x48>;
    };
};
```

Driver:

```c
static const struct of_device_id bbb_temp_of_match[] = {
    {
        .compatible = "bbb,demo-temperature",
    },
    { }
};
```

Flow:

```text
&i2c1
  |
  v
I2C Controller
  |
  v
temperature@48
  |
  v
compatible = "bbb,demo-temperature"
  |
  v
I2C Driver
  |
  v
probe()
```

---

# 34. Device Tree SPI Example

Example:

```dts
&spi0 {
    status = "okay";

    test-device@0 {
        compatible = "bbb,demo-spi";
        reg = <0>;
        spi-max-frequency = <5000000>;
    };
};
```

Driver:

```c
static const struct of_device_id bbb_spi_of_match[] = {
    {
        .compatible = "bbb,demo-spi",
    },
    { }
};
```

Flow:

```text
SPI Controller
      |
      v
SPI Device
      |
      v
Driver Match
      |
      v
probe()
```

---

# 35. Device Tree UART Example

Example:

```dts
&uart1 {
    pinctrl-names = "default";
    pinctrl-0 = <&uart1_pins>;
    status = "okay";
};
```

Conceptually:

```text
uart1
 |
 +-- pinctrl
 |
 +-- status
 |
 v
UART Driver
 |
 v
Serial Core
 |
 v
/dev/tty*
```

For a custom UART-connected device, additional child-device
description depends on the UART subsystem and protocol.

---

# 36. Device Tree PWM Example

Conceptual PWM consumer:

```dts
pwm_test {
    compatible = "bbb,pwm-test";
    pwms = <&ehrpwm1 0 1000000 0>;
};
```

The values represent:

```text
PWM Controller
Channel
Period
Polarity
```

The exact PWM controller and channel must match the AM335x
configuration used by the board.

Driver:

```text
PWM Consumer
     |
     v
PWM Framework
     |
     v
PWM Controller Driver
```

---

# 37. Device Tree ADC Example

The ADC is generally exposed through the IIO subsystem.

Conceptual architecture:

```text
AM335x ADC
    |
    v
ADC Driver
    |
    v
IIO Framework
    |
    v
IIO Channels
```

For the BeagleBone Black, the ADC configuration must also respect
the electrical limitations of the analog input pins.

Do not connect signals above the supported ADC input range.

Check the board/SoC hardware documentation before applying external
voltages.

---

# 38. Device Tree CAN Example

Conceptual:

```dts
&dcan0 {
    pinctrl-names = "default";
    pinctrl-0 = <&dcan0_pins>;
    status = "okay";
};
```

Flow:

```text
Device Tree
     |
     +-- DCAN Controller
     |
     +-- Pinmux
     |
     +-- Clock
     |
     v
CAN Driver
     |
     v
SocketCAN
     |
     v
can0
```

A physical CAN transceiver is required for a real CAN bus connection.

---

# 39. Device Tree Debugging

When a peripheral does not work, inspect Device Tree first.

## Step 1 - Check live Device Tree

```bash
ls /sys/firmware/devicetree/base/
```

## Step 2 - Check node status

For example:

```bash
find /sys/firmware/devicetree/base/ -name status -print
```

## Step 3 - Check kernel logs

```bash
dmesg | grep -i -E "gpio|uart|i2c|spi|pwm|adc|can"
```

## Step 4 - Check drivers

```bash
ls /sys/bus/platform/drivers/
```

I2C:

```bash
ls /sys/bus/i2c/drivers/
```

SPI:

```bash
ls /sys/bus/spi/drivers/
```

## Step 5 - Check devices

```bash
ls /sys/class/
```

---

# 40. Common Device Tree Errors

## Error 1 - Wrong `compatible`

```dts
compatible = "wrong,device";
```

Result:

```text
Driver does not match
probe() not called
```

---

## Error 2 - Missing `status`

If a device is disabled:

```dts
status = "disabled";
```

it may not be initialized.

---

## Error 3 - Incorrect pinmux

Driver may load:

```text
probe() = success
```

but hardware does not operate.

---

## Error 4 - Wrong register address

Example:

```dts
reg = <wrong-address>;
```

The driver may fail when accessing the resource.

---

## Error 5 - Incorrect IRQ

Wrong interrupt configuration can cause:

```text
No interrupts
Spurious interrupts
System instability
```

---

## Error 6 - Wrong GPIO

A wrong GPIO controller or line can cause:

```text
LED does not work
Button does not work
Interrupt does not work
```

---

## Error 7 - I2C Address Conflict

Two devices cannot normally use the same address on the same I2C
bus unless the hardware provides an appropriate way to distinguish
them.

---

## Error 8 - SPI Chip Select

Wrong chip select configuration can result in:

```text
SPI transfers occur
but device never responds
```

---

## Error 9 - Overlay Target Does Not Exist

An overlay may target a label that is absent from the base Device Tree.

Example:

```dts
target = <&uart1>;
```

If `uart1` is not exported as expected:

```text
Overlay application fails
```

---

## Error 10 - Overlay Compiles but Hardware Does Not Work

Successful compilation only means:

```text
DTS syntax / overlay structure is acceptable
```

It does not guarantee:

```text
Correct pinmux
Correct hardware
Correct driver
Correct wiring
```

---

# 41. Project Directory Mapping

Your project should maintain a clear separation:

```text
beaglebone-black/
|
+-- device-tree/
|   |
|   +-- adc/
|   |   +-- bbb-adc.dts
|   |   +-- bbb-adc.dtsi
|   |   +-- README.md
|   |
|   +-- can/
|   |   +-- bbb-can.dts
|   |   +-- bbb-can.dtsi
|   |   +-- README.md
|   |
|   +-- gpio/
|   |   +-- bbb-gpio.dts
|   |   +-- bbb-gpio.dtsi
|   |   +-- README.md
|   |
|   +-- i2c/
|   |   +-- bbb-i2c.dts
|   |   +-- bbb-i2c.dtsi
|   |   +-- README.md
|   |
|   +-- overlays/
|   |   +-- bbb-gpio-overlay.dts
|   |   +-- bbb-i2c-overlay.dts
|   |   +-- bbb-spi-overlay.dts
|   |   +-- bbb-uart-overlay.dts
|   |   +-- README.md
|   |
|   +-- pwm/
|   |   +-- bbb-pwm.dts
|   |   +-- bbb-pwm.dtsi
|   |   +-- README.md
|   |
|   +-- spi/
|   |   +-- bbb-spi.dts
|   |   +-- bbb-spi.dtsi
|   |   +-- README.md
|   |
|   +-- uart/
|       +-- bbb-uart.dts
|       +-- bbb-uart.dtsi
|       +-- README.md
|
+-- drivers/
|
+-- tests/
|
+-- docs/
```

---

# 42. Recommended Development Flow

For every peripheral in this project, use the same development
methodology.

```text
                 Hardware
                    |
                    v
              Check Datasheet
                    |
                    v
              Identify Pins
                    |
                    v
               Configure
                Pinmux
                    |
                    v
              Create DTS/DTSI
                    |
                    v
               Compile DTB
                    |
                    v
              Boot BeagleBone
                    |
                    v
             Check Live DT
                    |
                    v
             Driver Matching
                    |
                    v
                  probe()
                    |
                    v
             Test Interface
                    |
                    v
             Test Hardware
```

---

# 43. Peripheral-by-Peripheral Flow

## GPIO

```text
GPIO Pin
  |
  v
Pinmux
  |
  v
GPIO Controller
  |
  v
GPIO Framework
  |
  v
Driver / Consumer
```

## UART

```text
UART Pins
  |
  v
Pinmux
  |
  v
UART Controller
  |
  v
Serial Core
  |
  v
/dev/tty*
```

## I2C

```text
SDA/SCL
  |
  v
Pinmux
  |
  v
I2C Controller
  |
  v
I2C Core
  |
  v
I2C Client
  |
  v
Device Driver
```

## SPI

```text
MOSI/MISO/SCLK/CS
        |
        v
      Pinmux
        |
        v
  SPI Controller
        |
        v
     SPI Core
        |
        v
   SPI Device
        |
        v
   SPI Driver
```

## PWM

```text
PWM Pin
  |
  v
Pinmux
  |
  v
PWM Controller
  |
  v
PWM Framework
  |
  v
PWM Consumer
```

## ADC

```text
AIN
 |
 v
ADC
 |
 v
IIO Driver
 |
 v
IIO Framework
 |
 v
User Space
```

## CAN

```text
CAN TX/RX
    |
    v
  Pinmux
    |
    v
CAN Controller
    |
    v
CAN Driver
    |
    v
SocketCAN
    |
    v
can0
```

---

# 44. Interview Explanation

A strong interview answer:

> "Device Tree is used to describe board-specific hardware to the
> Linux kernel without hard-coding that information into the driver.
> On the BeagleBone Black, I use DTS and DTSI files to configure
> peripherals such as GPIO, UART, I2C, SPI, PWM, ADC and CAN, along
> with resources such as pinmux, interrupts, clocks and DMA. The DTS
> is compiled into a DTB, which is passed to Linux during boot. Linux
> creates the corresponding devices and matches them with drivers
> using properties such as the compatible string. Once the driver
> matches, its probe function obtains the resources and initializes
> the hardware."

---

# 45. Practical Debugging Example

Suppose an I2C sensor is not working.

Start with:

```bash
dmesg | grep -i i2c
```

Check I2C adapters:

```bash
ls /dev/i2c-*
```

Check the Device Tree:

```bash
ls /sys/firmware/devicetree/base/
```

Check I2C buses:

```bash
ls /sys/bus/i2c/devices/
```

If available in the target image, scan the bus:

```bash
i2cdetect -l
```

Then:

```bash
i2cdetect -y <bus-number>
```

Interpretation:

```text
Device Tree
    |
    +-- I2C enabled?
    |
    +-- Pinmux correct?
    |
    +-- Device address correct?
    |
    v
I2C Controller
    |
    v
I2C Adapter
    |
    v
I2C Device
    |
    v
Driver
```

Do not rely only on `i2cdetect`: some I2C devices may not respond
safely to arbitrary probing, and some devices use special addressing
or bus behavior.

---

# 46. Device Tree Validation Checklist

Before committing a Device Tree change:

```text
[ ] Correct board selected
[ ] Correct SoC selected
[ ] Correct kernel/BSP version
[ ] DTS syntax valid
[ ] DTSI includes correct
[ ] compatible string correct
[ ] status correct
[ ] pinctrl correct
[ ] register address correct
[ ] interrupt configuration correct
[ ] clocks correct
[ ] DMA configuration correct
[ ] GPIO references correct
[ ] I2C address correct
[ ] SPI chip select correct
[ ] PWM channel correct
[ ] ADC channel correct
[ ] CAN pinmux correct
[ ] Overlay target correct
[ ] DTB/DTBO generated
[ ] Boot configuration updated
[ ] dmesg checked
[ ] Hardware tested
```

---

# 47. Important Rule: Device Tree Is Not the Driver

A common beginner mistake is to think:

```text
Device Tree = Driver
```

They are different.

Device Tree:

```text
Describes hardware
```

Driver:

```text
Controls hardware
```

For example:

```text
Device Tree
     |
     | "This device exists at this location"
     |
     v
Driver
     |
     | "Here is how to operate it"
     |
     v
Hardware
```

---

# 48. Important Rule: Don't Reimplement Existing Subsystems

For this project, avoid writing a custom character driver for every
peripheral.

Prefer:

```text
GPIO  -> GPIO Framework
UART  -> TTY / Serial
I2C   -> I2C Framework
SPI   -> SPI Framework
PWM   -> PWM Framework
ADC   -> IIO
CAN   -> SocketCAN
```

This demonstrates production-oriented Linux driver development.

A custom character driver is still useful as a learning exercise for
understanding:

```text
open()
read()
write()
ioctl()
poll()
```

but it should not replace the kernel's standard subsystem when a
suitable subsystem already exists.

---

# 49. Complete Device Tree Architecture

The final project architecture is:

```text
                       BeagleBone Black
                              |
                           AM335x
                              |
          +-------------------+-------------------+
          |                   |                   |
          v                   v                   v
        GPIO                UART                I2C
          |                   |                   |
     GPIO Core            Serial Core          I2C Core
          |                   |                   |
          v                   v                   v
       Driver              Driver              Driver
          |                   |                   |
          +-------------------+-------------------+
                              |
             +----------------+----------------+
             |                |                |
             v                v                v
            SPI              PWM              ADC
             |                |                |
          SPI Core        PWM Framework      IIO
             |                |                |
          Driver            Driver            Driver
             |                |                |
             +----------------+----------------+
                              |
                              v
                             CAN
                              |
                         SocketCAN
                              |
                              v
                         CAN Driver
```

Device Tree configures these components:

```text
                     Device Tree
                          |
        +-----------------+-----------------+
        |                 |                 |
      Pinmux           Resources          Status
        |                 |                 |
        +-----------------+-----------------+
                          |
                          v
                  Linux Device Model
                          |
                          v
                  Driver Matching
                          |
                          v
                       probe()
```

---

# 50. Summary

For this BeagleBone Black project, Device Tree is the bridge between
the physical hardware and the Linux driver model.

The complete concept is:

```text
Hardware
   |
   v
Pin Multiplexing
   |
   v
Device Tree
   |
   v
DTB
   |
   v
U-Boot
   |
   v
Linux Kernel
   |
   v
Device Creation
   |
   v
Driver Matching
   |
   v
probe()
   |
   v
Linux Subsystem
   |
   v
User Space
```

The peripherals covered by this project are:

```text
+--------+---------------------+
| Device | Linux Subsystem     |
+--------+---------------------+
| GPIO   | GPIO Framework      |
| UART   | TTY / Serial        |
| I2C    | I2C Framework       |
| SPI    | SPI Framework       |
| PWM    | PWM Framework       |
| ADC    | IIO Framework       |
| CAN    | SocketCAN           |
+--------+---------------------+
```

The Device Tree directory in this repository therefore provides the
hardware configuration layer, while the `drivers/` directory provides
the driver implementation and `tests/` provides validation.

The key driver-development chain is:

```text
DTS/DTSI
   ↓
DTB/DTBO
   ↓
Bootloader
   ↓
Linux Device Tree
   ↓
Device Creation
   ↓
Driver Matching
   ↓
probe()
   ↓
Resource Acquisition
   ↓
Hardware Initialization
   ↓
Linux Subsystem
   ↓
User-Space Test
```

This separation makes the project suitable for learning and
demonstrating real Embedded Linux BSP and device-driver development.

```
```

