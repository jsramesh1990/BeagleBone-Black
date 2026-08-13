# `03_linux_driver_model.md`

Create this file:

```text
beaglebone-black/docs/03_linux_driver_model.md
```

````markdown
# 03 - Linux Driver Model

## Table of Contents

- [1. Overview](#1-overview)
- [2. Why the Linux Driver Model](#2-why-the-linux-driver-model)
- [3. Linux Driver Architecture](#3-linux-driver-architecture)
- [4. Hardware to User Space Flow](#4-hardware-to-user-space-flow)
- [5. Major Components](#5-major-components)
- [6. Device](#6-device)
- [7. Driver](#7-driver)
- [8. Bus](#8-bus)
- [9. Device-Bus-Driver Relationship](#9-device-bus-driver-relationship)
- [10. Device Tree](#10-device-tree)
- [11. Platform Devices](#11-platform-devices)
- [12. Platform Drivers](#12-platform-drivers)
- [13. Driver Matching](#13-driver-matching)
- [14. Probe Function](#14-probe-function)
- [15. Remove Function](#15-remove-function)
- [16. Linux Driver Registration](#16-linux-driver-registration)
- [17. Module Initialization](#17-module-initialization)
- [18. Module Exit](#18-module-exit)
- [19. Character Device Drivers](#19-character-device-drivers)
- [20. File Operations](#20-file-operations)
- [21. Device Nodes](#21-device-nodes)
- [22. Major and Minor Numbers](#22-major-and-minor-numbers)
- [23. Kernel Subsystems](#23-kernel-subsystems)
- [24. GPIO Driver Model](#24-gpio-driver-model)
- [25. UART Driver Model](#25-uart-driver-model)
- [26. I2C Driver Model](#26-i2c-driver-model)
- [27. SPI Driver Model](#27-spi-driver-model)
- [28. PWM Driver Model](#28-pwm-driver-model)
- [29. ADC / IIO Driver Model](#29-adc--iio-driver-model)
- [30. CAN / SocketCAN Driver Model](#30-can--socketcan-driver-model)
- [31. Interrupts](#31-interrupts)
- [32. DMA](#32-dma)
- [33. Clock and Reset](#33-clock-and-reset)
- [34. Power Management](#34-power-management)
- [35. Runtime PM](#35-runtime-pm)
- [36. Sysfs](#36-sysfs)
- [37. Debugfs](#37-debugfs)
- [38. Kernel Logs](#38-kernel-logs)
- [39. Driver Probe Debugging](#39-driver-probe-debugging)
- [40. Complete Driver Flow](#40-complete-driver-flow)
- [41. BeagleBone Black Project Mapping](#41-beaglebone-black-project-mapping)
- [42. Driver Development Strategy](#42-driver-development-strategy)
- [43. Interview Explanation](#43-interview-explanation)
- [44. Summary](#44-summary)

---

# 1. Overview

The Linux Driver Model provides a common framework for representing:

- Devices
- Drivers
- Buses
- Device classes
- Device nodes
- Device Tree
- Power management
- Hotplug
- Sysfs

For this BeagleBone Black project, the Linux Driver Model is the
foundation for integrating:

```text
GPIO
UART
I2C
SPI
PWM
ADC
CAN
````

The important concept is:

```text
Hardware
   |
   v
Device Tree
   |
   v
Linux Device Model
   |
   v
Bus / Subsystem
   |
   v
Driver
   |
   v
Probe()
   |
   v
Hardware Initialization
   |
   v
User Space Interface
```

---

# 2. Why the Linux Driver Model

Without a common driver model, every driver would need to implement
its own way of handling:

```text
Device registration
Driver registration
Device matching
Power management
Resource management
Device naming
User-space access
Hotplug
```

Linux provides standard frameworks so that drivers can focus on
hardware-specific operations.

For example:

```text
I2C Driver
    |
    +---- I2C Core
              |
              +---- Adapter
              +---- Client
              +---- Driver
```

Similarly:

```text
SPI Driver
    |
    +---- SPI Core
              |
              +---- Controller
              +---- Device
              +---- Driver
```

---

# 3. Linux Driver Architecture

The overall architecture is:

```text
+----------------------------------------------------+
|                    User Space                      |
|                                                    |
| Application / Test Program / Shell                 |
+--------------------------+-------------------------+
                           |
                           | ioctl / read / write
                           v
+----------------------------------------------------+
|                    Kernel Space                    |
|                                                    |
|             Linux Device Driver                   |
|                                                    |
|   +--------------------------------------------+   |
|   |             Kernel Subsystem               |   |
|   |                                            |   |
|   | GPIO | I2C | SPI | TTY | PWM | IIO | CAN |   |
|   +--------------------------------------------+   |
|                       |                            |
|                       v                            |
|                Linux Driver Model                  |
|                       |                            |
|              Device / Bus / Driver                |
+-----------------------+----------------------------+
                        |
                        v
+----------------------------------------------------+
|                     Hardware                       |
|                                                    |
| AM335x GPIO / UART / I2C / SPI / PWM / ADC / CAN  |
+----------------------------------------------------+
```

---

# 4. Hardware to User Space Flow

For this project, the complete flow is:

```text
Physical Hardware
       |
       v
AM335x Peripheral
       |
       v
Pin Multiplexing
       |
       v
Device Tree
       |
       v
Linux Device Model
       |
       v
Bus / Subsystem
       |
       v
Driver Matching
       |
       v
probe()
       |
       v
Resource Acquisition
       |
       v
Hardware Initialization
       |
       v
Kernel Interface
       |
       v
/dev
/sys
netdev
IIO
GPIO character device
       |
       v
User Application
```

---

# 5. Major Components

The Linux Driver Model mainly revolves around:

```text
Device
Driver
Bus
Class
Device Tree
Subsystem
```

Relationship:

```text
                    Linux Driver Model
                           |
        +------------------+------------------+
        |                  |                  |
        v                  v                  v
      Device             Driver             Bus
        |                  |                  |
        +------------------+------------------+
                           |
                           v
                       Subsystem
```

---

# 6. Device

A device represents a hardware device known to the Linux kernel.

Examples:

```text
UART controller
I2C sensor
SPI flash
GPIO controller
PWM controller
ADC
CAN controller
```

A device can contain information such as:

```text
Device name
Resources
Memory address
IRQ
DMA channels
Device Tree node
Parent device
Bus
Driver
```

Conceptually:

```text
Device
 |
 +-- Name
 +-- Resources
 +-- IRQ
 +-- Memory
 +-- Device Tree node
 +-- Bus
 +-- Driver
```

---

# 7. Driver

A driver contains the software required to control a hardware device.

Typical responsibilities:

```text
Initialize hardware
Configure registers
Handle interrupts
Perform data transfers
Manage power
Expose kernel interface
Cleanup resources
```

Typical driver structure:

```text
Driver
 |
 +-- Probe
 +-- Remove
 +-- Suspend
 +-- Resume
 +-- Shutdown
```

For a platform driver:

```c
static struct platform_driver bbb_driver = {
    .probe  = bbb_probe,
    .remove = bbb_remove,

    .driver = {
        .name = "bbb-driver",
        .of_match_table = bbb_of_match,
    },
};
```

---

# 8. Bus

A bus provides a framework for connecting devices and drivers.

Examples:

```text
Platform Bus
I2C Bus
SPI Bus
USB Bus
PCI Bus
```

Conceptually:

```text
                 Bus
                  |
        +---------+---------+
        |                   |
        v                   v
     Devices             Drivers
```

The bus is responsible for helping Linux determine:

```text
Which driver supports this device?
```

---

# 9. Device-Bus-Driver Relationship

The basic relationship is:

```text
              BUS
               |
       +-------+-------+
       |               |
       v               v
    DEVICE           DRIVER
       |               |
       +-------+-------+
               |
             Match
               |
               v
            probe()
```

For example:

```text
Device Tree
    |
    v
I2C Device
    |
    v
I2C Core
    |
    v
I2C Driver
    |
    v
probe()
```

---

# 10. Device Tree

Device Tree describes hardware to the Linux kernel.

Example:

```dts
uart1: serial@48022000 {
    compatible = "ti,am3352-uart";
    reg = <0x48022000 0x2000>;
    interrupts = <73>;
    status = "okay";
};
```

Important properties include:

```text
compatible
reg
interrupts
clocks
dmas
pinctrl-names
pinctrl-0
status
```

The Device Tree does not normally contain the driver implementation.

It describes the hardware.

```text
Device Tree
     |
     | describes
     v
Hardware
     |
     | matched with
     v
Driver
```

---

# 11. Platform Devices

Many SoC peripherals are represented as platform devices.

Examples:

```text
GPIO Controller
UART Controller
PWM Controller
ADC Controller
CAN Controller
```

A platform device typically represents hardware that is not
enumerated through a discoverable bus such as PCI.

Example:

```text
AM335x
 |
 +-- UART
 |     |
 |     +-- Platform Device
 |
 +-- GPIO
 |     |
 |     +-- Platform Device
 |
 +-- PWM
       |
       +-- Platform Device
```

---

# 12. Platform Drivers

A platform driver is commonly used for SoC-integrated peripherals.

Basic structure:

```c
#include <linux/module.h>
#include <linux/platform_device.h>

static int bbb_probe(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "BBB driver probed\n");

    return 0;
}

static void bbb_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "BBB driver removed\n");
}

static const struct of_device_id bbb_of_match[] = {
    {
        .compatible = "bbb,my-device",
    },
    { }
};

MODULE_DEVICE_TABLE(of, bbb_of_match);

static struct platform_driver bbb_driver = {
    .probe  = bbb_probe,
    .remove = bbb_remove,

    .driver = {
        .name = "bbb-driver",
        .of_match_table = bbb_of_match,
    },
};

module_platform_driver(bbb_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Linux Developer");
MODULE_DESCRIPTION("BeagleBone Black Example Platform Driver");
```

---

# 13. Driver Matching

Driver matching is one of the most important concepts.

Device Tree:

```dts
compatible = "bbb,my-device";
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

Linux compares the compatible strings.

```text
Device Tree
    |
    | "bbb,my-device"
    v
Device
    |
    v
Driver Matching
    |
    | match
    v
bbb_probe()
```

If there is no match:

```text
probe() is not called
```

---

# 14. Probe Function

`probe()` is normally where the driver initializes a matched device.

Typical sequence:

```text
probe()
  |
  +-- Get Device Tree properties
  |
  +-- Get memory resources
  |
  +-- Get IRQ
  |
  +-- Get GPIO
  |
  +-- Get clocks
  |
  +-- Get regulators
  |
  +-- Initialize hardware
  |
  +-- Register subsystem interface
  |
  +-- Create user interface
  |
  +-- Return success
```

Example:

```c
static int bbb_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    dev_info(dev, "Probe started\n");

    /* Resource acquisition */

    /* Hardware initialization */

    /* Subsystem registration */

    dev_info(dev, "Probe successful\n");

    return 0;
}
```

---

# 15. Remove Function

The remove function releases resources when the driver is removed.

Typical cleanup:

```text
remove()
   |
   +-- Stop hardware
   |
   +-- Free IRQ
   |
   +-- Release GPIO
   |
   +-- Disable clocks
   |
   +-- Unregister device
   |
   +-- Free memory
```

Example:

```c
static void bbb_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Driver removed\n");
}
```

Modern Linux kernel APIs increasingly use managed resource
interfaces such as:

```text
devm_kzalloc()
devm_ioremap_resource()
devm_request_irq()
devm_gpiod_get()
```

These automatically release resources when the device is detached.

---

# 16. Linux Driver Registration

There are two important registration operations:

```text
Device Registration
Driver Registration
```

Conceptually:

```text
Kernel
  |
  +---- Device registered
  |
  +---- Driver registered
           |
           v
        Matching
           |
           v
         Probe
```

For platform drivers:

```c
platform_driver_register(&bbb_driver);
```

or:

```c
module_platform_driver(bbb_driver);
```

---

# 17. Module Initialization

A kernel module has an initialization entry point.

Example:

```c
static int __init bbb_init(void)
{
    pr_info("BBB driver loaded\n");

    return platform_driver_register(&bbb_driver);
}

module_init(bbb_init);
```

When the module is loaded:

```bash
sudo insmod bbb_driver.ko
```

Linux calls:

```text
bbb_init()
```

Then the driver is registered.

---

# 18. Module Exit

Example:

```c
static void __exit bbb_exit(void)
{
    platform_driver_unregister(&bbb_driver);

    pr_info("BBB driver unloaded\n");
}

module_exit(bbb_exit);
```

Unload:

```bash
sudo rmmod bbb_driver
```

Flow:

```text
insmod
  |
  v
module_init()
  |
  v
driver_register()
  |
  v
Device Matching
  |
  v
probe()
```

Unload:

```text
rmmod
  |
  v
remove()
  |
  v
driver_unregister()
  |
  v
module_exit()
```

---

# 19. Character Device Drivers

Character devices provide byte-oriented access to hardware.

Typical examples:

```text
UART
GPIO
Sensors
Custom devices
```

Typical user-space interface:

```text
/dev/mydevice
```

A character driver commonly implements:

```text
open()
read()
write()
ioctl()
poll()
release()
```

Architecture:

```text
Application
     |
     v
/dev/mydevice
     |
     v
VFS
     |
     v
Character Driver
     |
     v
Hardware
```

---

# 20. File Operations

Example:

```c
static const struct file_operations bbb_fops = {
    .owner   = THIS_MODULE,
    .open    = bbb_open,
    .read    = bbb_read,
    .write   = bbb_write,
    .release = bbb_release,
};
```

Typical flow:

```text
open()
  |
  v
Driver Open

read()
  |
  v
Driver Read

write()
  |
  v
Driver Write

ioctl()
  |
  v
Driver Control

close()
  |
  v
Driver Release
```

---

# 21. Device Nodes

User space communicates with many character drivers through `/dev`.

Example:

```text
/dev/ttyS1
/dev/i2c-1
/dev/spidev0.0
```

Custom driver:

```text
/dev/bbb_device
```

Check:

```bash
ls -l /dev/bbb_device
```

Conceptually:

```text
User Application
      |
      v
/dev/bbb_device
      |
      v
VFS
      |
      v
Driver
      |
      v
Hardware
```

---

# 22. Major and Minor Numbers

Character devices traditionally use:

```text
Major Number
Minor Number
```

The major number identifies the driver.

The minor number identifies a particular device.

Conceptually:

```text
Major = Driver
Minor = Device
```

Example:

```text
/dev/mydevice0
/dev/mydevice1
/dev/mydevice2
```

All can belong to the same driver but represent different devices.

Modern drivers often use:

```c
alloc_chrdev_region()
```

instead of hard-coded major numbers.

---

# 23. Kernel Subsystems

Linux provides specialized subsystems so drivers do not have to
reinvent common functionality.

Important subsystems for this project:

```text
GPIO
TTY / Serial
I2C
SPI
PWM
IIO
SocketCAN
```

Mapping:

```text
Hardware        Linux Subsystem

GPIO      --->   GPIO subsystem
UART      --->   TTY / Serial
I2C       --->   I2C subsystem
SPI       --->   SPI subsystem
PWM       --->   PWM subsystem
ADC       --->   IIO subsystem
CAN       --->   SocketCAN
```

This is extremely important for production driver development.

---

# 24. GPIO Driver Model

GPIO uses the Linux GPIO subsystem.

Modern Linux uses the GPIO descriptor API.

Typical APIs include:

```c
devm_gpiod_get()
gpiod_set_value()
gpiod_get_value()
gpiod_set_value_cansleep()
```

Example:

```c
struct gpio_desc *led;

led = devm_gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);

if (IS_ERR(led))
    return PTR_ERR(led);

gpiod_set_value(led, 1);
```

Flow:

```text
Device Tree
    |
    v
GPIO Property
    |
    v
GPIO Framework
    |
    v
GPIO Controller Driver
    |
    v
AM335x GPIO Hardware
```

---

# 25. UART Driver Model

UART is normally integrated with the Linux serial/TTY subsystem.

Flow:

```text
User Application
      |
      v
/dev/ttyS*
      |
      v
TTY Layer
      |
      v
Serial Core
      |
      v
UART Driver
      |
      v
AM335x UART
      |
      v
TX/RX Pins
```

Typical application:

```bash
echo "Hello" > /dev/ttyS1
```

The exact UART device name depends on the kernel and board
configuration.

---

# 26. I2C Driver Model

I2C consists of:

```text
I2C Controller
I2C Adapter
I2C Client
I2C Driver
```

Architecture:

```text
             I2C Controller
                    |
                    v
               I2C Adapter
                    |
             +------+------+
             |             |
             v             v
         I2C Client    I2C Client
             |             |
             v             v
          Sensor        EEPROM
             |
             v
        I2C Driver
```

Example driver structure:

```c
static struct i2c_driver bbb_i2c_driver = {
    .driver = {
        .name = "bbb-i2c",
        .of_match_table = bbb_i2c_of_match,
    },

    .probe = bbb_i2c_probe,
    .remove = bbb_i2c_remove,
};
```

I2C driver matching normally uses Device Tree compatible strings.

---

# 27. SPI Driver Model

SPI architecture:

```text
SPI Controller
      |
      v
SPI Master / Controller Driver
      |
      v
SPI Core
      |
      v
SPI Device
      |
      v
SPI Client Driver
```

Typical driver structure:

```c
static struct spi_driver bbb_spi_driver = {
    .driver = {
        .name = "bbb-spi",
        .of_match_table = bbb_spi_of_match,
    },

    .probe = bbb_spi_probe,
    .remove = bbb_spi_remove,
};
```

Data transfer:

```text
Application
    |
    v
Driver
    |
    v
SPI Core
    |
    v
SPI Controller
    |
    v
MOSI / MISO / SCLK / CS
```

---

# 28. PWM Driver Model

PWM uses the Linux PWM framework.

Conceptually:

```text
User / Kernel Consumer
        |
        v
PWM Framework
        |
        v
PWM Controller Driver
        |
        v
AM335x PWM Hardware
        |
        v
PWM Pin
```

Typical configuration concepts:

```text
Period
Duty Cycle
Polarity
Enable
```

A PWM consumer should use the kernel PWM APIs rather than directly
accessing hardware registers.

---

# 29. ADC / IIO Driver Model

ADC devices commonly use the Linux **Industrial I/O (IIO)**
subsystem.

Architecture:

```text
Analog Signal
     |
     v
ADC Hardware
     |
     v
ADC Driver
     |
     v
IIO Framework
     |
     v
IIO Channel
     |
     v
User Space
```

Typical sysfs interface:

```text
/sys/bus/iio/devices/iio:device0/
```

Example:

```text
in_voltage0_raw
```

The exact interface depends on the kernel and driver.

---

# 30. CAN / SocketCAN Driver Model

CAN uses the Linux networking architecture through SocketCAN.

Flow:

```text
Application
     |
     v
SocketCAN
     |
     v
CAN Network Device
     |
     v
CAN Driver
     |
     v
CAN Controller
     |
     v
CAN Transceiver
     |
     v
CAN Bus
```

User-space example:

```bash
candump can0
```

Transmit:

```bash
cansend can0 123#11223344
```

The CAN driver does not normally expose a custom `/dev/can0`
character device.

Instead, it integrates with the networking subsystem.

---

# 31. Interrupts

Hardware can generate interrupts when an event occurs.

Example:

```text
GPIO Button
     |
     v
GPIO Controller
     |
     v
Interrupt Controller
     |
     v
CPU
     |
     v
Interrupt Handler
```

Typical driver flow:

```text
Hardware Event
     |
     v
IRQ
     |
     v
ISR / IRQ Handler
     |
     v
Schedule Work
     |
     v
Bottom Half / Threaded IRQ
     |
     v
Process Event
```

Typical APIs:

```c
devm_request_irq()
devm_request_threaded_irq()
```

Example:

```c
static irqreturn_t bbb_irq_handler(int irq, void *data)
{
    pr_info("Interrupt received\n");

    return IRQ_HANDLED;
}
```

---

# 32. DMA

DMA allows peripherals to transfer data directly to memory without
requiring the CPU to copy every byte.

Without DMA:

```text
Peripheral
    |
    v
CPU
    |
    v
Memory
```

With DMA:

```text
Peripheral
    |
    v
DMA Controller
    |
    v
DDR Memory
```

Typical DMA flow:

```text
Driver
  |
  v
DMA API
  |
  v
DMA Controller
  |
  v
Memory
```

DMA is especially useful for:

```text
UART
SPI
Audio
Network
High-speed ADC
```

---

# 33. Clock and Reset

Many SoC peripherals require clocks and reset control.

Driver initialization can involve:

```text
Get Clock
Enable Clock
Deassert Reset
Initialize Hardware
```

Conceptually:

```text
Driver
 |
 +---- Clock Framework
 |
 +---- Reset Framework
 |
 +---- Hardware
```

Example:

```text
Clock OFF
   |
   v
Driver Probe
   |
   v
Clock Enable
   |
   v
Peripheral Register Access
```

If a peripheral is not clocked, register access or hardware
operation may fail.

---

# 34. Power Management

Linux drivers can participate in system power management.

Typical callbacks:

```text
Suspend
Resume
Runtime Suspend
Runtime Resume
```

Flow:

```text
Normal Operation
      |
      v
Suspend
      |
      v
Hardware Low Power
      |
      v
Resume
      |
      v
Hardware Reinitialized
```

A good production driver should correctly handle power-state changes.

---

# 35. Runtime PM

Runtime Power Management allows an unused device to enter a
low-power state without suspending the entire system.

Conceptually:

```text
Device Active
     |
     | idle
     v
Runtime Suspend
     |
     | activity
     v
Runtime Resume
```

Typical APIs include:

```c
pm_runtime_enable()
pm_runtime_get_sync()
pm_runtime_put()
```

The exact API usage depends on the device and kernel version.

---

# 36. Sysfs

Sysfs exposes kernel objects and attributes to user space.

Main location:

```text
/sys/
```

Important directories:

```text
/sys/bus/
/sys/class/
/sys/devices/
/sys/kernel/
```

For example:

```bash
ls /sys/class/
```

Subsystems can expose information through sysfs.

Examples:

```text
/sys/class/gpio/
/sys/class/pwm/
/sys/bus/iio/
/sys/bus/i2c/
/sys/bus/spi/
```

Sysfs is mainly for configuration and information, not a replacement
for every driver API.

---

# 37. Debugfs

Debugfs provides debugging information for kernel subsystems and
drivers.

Mount:

```bash
mount -t debugfs none /sys/kernel/debug
```

Then:

```bash
ls /sys/kernel/debug/
```

Debugfs may expose:

```text
Driver State
GPIO State
Clock State
Regulator State
DMA State
Tracing Information
```

Debugfs should be considered a debugging interface rather than a
stable application ABI.

---

# 38. Kernel Logs

Driver debugging starts with kernel logs.

Use:

```bash
dmesg
```

or:

```bash
dmesg -w
```

Filter:

```bash
dmesg | grep -i bbb
```

Subsystem-specific:

```bash
dmesg | grep -i gpio
dmesg | grep -i uart
dmesg | grep -i i2c
dmesg | grep -i spi
dmesg | grep -i pwm
dmesg | grep -i adc
dmesg | grep -i can
```

Driver messages can be generated using:

```c
dev_info()
dev_warn()
dev_err()
dev_dbg()
```

Example:

```c
dev_info(&pdev->dev, "Probe successful\n");
```

---

# 39. Driver Probe Debugging

When a driver does not work, debug in this order:

```text
1. Is hardware powered?
        |
        v
2. Is pinmux correct?
        |
        v
3. Is Device Tree node enabled?
        |
        v
4. Is compatible string correct?
        |
        v
5. Is kernel driver enabled?
        |
        v
6. Is module loaded?
        |
        v
7. Did driver match?
        |
        v
8. Was probe() called?
        |
        v
9. Did resource acquisition succeed?
        |
        v
10. Did hardware initialization succeed?
        |
        v
11. Is user interface available?
        |
        v
12. Does hardware actually respond?
```

Useful commands:

```bash
dmesg
lsmod
modinfo <driver>
ls /sys/bus/platform/drivers/
ls /sys/bus/i2c/drivers/
ls /sys/bus/spi/drivers/
```

---

# 40. Complete Driver Flow

The most important flow for this project is:

```text
                    Device Tree
                         |
                         v
                  Hardware Node
                         |
                         v
                   Device Created
                         |
                         v
                  Bus/Subystem
                         |
                         v
                  Driver Registered
                         |
                         v
                  Match compatible
                         |
                    +----+----+
                    |         |
                  Match    No Match
                    |         |
                    v         v
                  probe()   No probe
                    |
                    v
             Get Resources
                    |
        +-----------+-----------+
        |           |           |
       IRQ        GPIO       Clock
        |           |           |
        +-----------+-----------+
                    |
                    v
             Initialize HW
                    |
                    v
             Register with
               Subsystem
                    |
                    v
              User Interface
                    |
                    v
             User Application
```

---

# 41. BeagleBone Black Project Mapping

This project maps the Linux Driver Model to the following
peripherals:

```text
beaglebone-black/
|
+-- device-tree/
|   |
|   +-- gpio/
|   +-- uart/
|   +-- i2c/
|   +-- spi/
|   +-- pwm/
|   +-- adc/
|   +-- can/
|   +-- overlays/
|
+-- drivers/
|   |
|   +-- gpio/
|   +-- uart/
|   +-- i2c/
|   +-- spi/
|   +-- pwm/
|   +-- adc/
|   +-- can/
|
+-- tests/
|   |
|   +-- gpio/
|   +-- uart/
|   +-- i2c/
|   +-- spi/
|   +-- pwm/
|   +-- adc/
|   +-- can/
|
+-- docs/
```

---

# 42. Driver Development Strategy

For this project, develop drivers in the following order:

## Phase 1 - GPIO

```text
Device Tree
    |
    v
GPIO Framework
    |
    v
GPIO Driver
    |
    v
LED / Button
```

Learn:

```text
GPIO descriptors
Input
Output
Interrupts
```

---

## Phase 2 - UART

```text
Device Tree
    |
    v
Serial Core
    |
    v
UART Driver
    |
    v
TX / RX
```

Learn:

```text
TTY
Baud rate
Interrupts
FIFO
DMA
```

---

## Phase 3 - I2C

```text
Device Tree
    |
    v
I2C Core
    |
    v
I2C Controller
    |
    v
I2C Client Driver
    |
    v
Sensor
```

Learn:

```text
I2C adapter
I2C client
I2C transactions
Device matching
```

---

## Phase 4 - SPI

```text
Device Tree
    |
    v
SPI Core
    |
    v
SPI Controller
    |
    v
SPI Device Driver
```

Learn:

```text
SPI transfer
Chip select
Mode
Frequency
Full duplex
```

---

## Phase 5 - PWM

Learn:

```text
PWM Framework
Period
Duty Cycle
Polarity
Enable / Disable
```

---

## Phase 6 - ADC

Learn:

```text
IIO Framework
Channels
Raw values
Scale
Sample rate
```

---

## Phase 7 - CAN

Learn:

```text
SocketCAN
Network Device
CAN Frames
Bitrate
CAN Controller
CAN Transceiver
```

---

# 43. One Board - Multiple Linux Subsystems

The main objective of this project is not simply to write seven
independent drivers.

It is to understand how different Linux subsystems work together.

```text
                   BeagleBone Black
                          |
                       AM335x
                          |
       +------------------+------------------+
       |                  |                  |
       v                  v                  v
     GPIO                UART               I2C
       |                  |                  |
    GPIO Core          TTY Core           I2C Core
       |                  |                  |
       v                  v                  v
    Driver              Driver             Driver

       +------------------+------------------+
                          |
                          v
                         SPI
                          |
                      SPI Core
                          |
                        Driver

                          |
                          v
                         PWM
                          |
                      PWM Core
                          |
                        Driver

                          |
                          v
                         ADC
                          |
                      IIO Core
                          |
                        Driver

                          |
                          v
                         CAN
                          |
                     SocketCAN
                          |
                        Driver
```

This gives practical experience with several major Linux kernel
subsystems on a single embedded platform.

---

# 44. Driver Development vs Application Development

Application:

```text
Application
    |
    v
/dev or Socket or Sysfs
    |
    v
Kernel
```

Driver:

```text
Driver
    |
    v
Kernel Subsystem
    |
    v
Hardware Registers
    |
    v
Peripheral
```

As an Embedded Linux developer, the important skill is understanding
both sides:

```text
Hardware
   |
   v
Driver
   |
   v
Kernel
   |
   v
User Space
```

---

# 45. Important Kernel APIs

The following APIs/frameworks are particularly relevant to this
project:

```text
Platform Driver
platform_driver_register()

Device Tree
of_match_table

Resources
platform_get_resource()
devm_ioremap_resource()

Memory
devm_kzalloc()

GPIO
devm_gpiod_get()

IRQ
devm_request_irq()

I2C
i2c_driver
i2c_transfer()

SPI
spi_driver
spi_sync()

PWM
PWM framework

ADC
IIO framework

CAN
SocketCAN

Power
Runtime PM

Logging
dev_info()
dev_err()
dev_dbg()
```

The exact API signatures can change between kernel versions, so
driver code must be written for the target kernel version.

---

# 46. Driver Debugging Checklist

When `probe()` is not called:

```text
[ ] Device Tree node exists
[ ] status = "okay"
[ ] compatible is correct
[ ] pinctrl is correct
[ ] kernel driver is enabled
[ ] driver module is loaded
[ ] driver is registered
[ ] device is present
[ ] driver/device match exists
```

When `probe()` fails:

```text
[ ] Check return code
[ ] Check dev_err()
[ ] Check IRQ
[ ] Check GPIO
[ ] Check clocks
[ ] Check regulators
[ ] Check memory resource
[ ] Check reset
[ ] Check pinmux
```

When hardware does not respond:

```text
[ ] Power
[ ] Ground
[ ] Wiring
[ ] Voltage
[ ] Clock
[ ] Pinmux
[ ] Register configuration
[ ] Logic analyzer
[ ] Oscilloscope
```

---

# 47. Interview Explanation

A strong interview explanation is:

> "The Linux Driver Model provides a standard framework for
> representing devices, drivers and buses inside the kernel. On the
> BeagleBone Black, the AM335x peripherals are described using Device
> Tree. Linux creates devices from those descriptions and matches them
> with registered drivers using mechanisms such as the Device Tree
> compatible string. Once a match occurs, the driver's probe function
> is called, where resources such as memory, GPIOs, interrupts and
> clocks are acquired and the hardware is initialized. For production
> development, I use Linux subsystems such as GPIO, TTY, I2C, SPI, PWM,
> IIO and SocketCAN instead of implementing independent interfaces from
> scratch."

---

# 48. Practical Driver Flow for This Project

For every peripheral, follow this pattern:

```text
1. Understand Hardware
        |
        v
2. Check Datasheet
        |
        v
3. Identify Registers
        |
        v
4. Identify Pinmux
        |
        v
5. Create Device Tree
        |
        v
6. Enable Kernel Subsystem
        |
        v
7. Write / Configure Driver
        |
        v
8. Build Kernel / Module
        |
        v
9. Boot BBB
        |
        v
10. Check dmesg
        |
        v
11. Verify Probe
        |
        v
12. Verify Device Interface
        |
        v
13. Run User-Space Test
        |
        v
14. Verify Physical Signal
        |
        v
15. Debug / Optimize
```

---

# 49. Final Architecture

The complete project architecture is:

```text
                         USER SPACE
                              |
             +----------------+----------------+
             |                |                |
          GPIO Test       I2C Test         SPI Test
          UART Test       PWM Test         ADC Test
          CAN Test
             |                |                |
             +----------------+----------------+
                              |
                              v
                       Linux Interfaces
                              |
      +-----------+-----------+-----------+-----------+
      |           |           |           |           |
     GPIO        TTY         I2C         SPI         PWM
      |           |           |           |           |
      +-----------+-----------+-----------+-----------+
                              |
                         ADC / IIO
                              |
                        CAN / SocketCAN
                              |
                              v
                     Linux Driver Model
                              |
             +----------------+----------------+
             |                |                |
           Device            Bus             Driver
             |                |                |
             +----------------+----------------+
                              |
                              v
                        Device Tree
                              |
                              v
                         Pinmux / Clocks
                              |
                              v
                            AM335x
                              |
                              v
                 BeagleBone Black Hardware
```

---

# 50. Summary

The key concepts to understand before writing the actual drivers are:

```text
Device
Driver
Bus
Device Tree
Platform Device
Platform Driver
Driver Matching
Probe
Remove
Kernel Module
Character Device
VFS
Device Node
GPIO Framework
TTY / Serial Framework
I2C Framework
SPI Framework
PWM Framework
IIO Framework
SocketCAN
Interrupts
DMA
Clock Framework
Reset Framework
Power Management
Sysfs
Debugfs
```

The most important relationship is:

```text
Device Tree
     |
     v
Device
     |
     v
Bus / Subsystem
     |
     v
Driver
     |
     v
Match
     |
     v
probe()
     |
     v
Hardware Initialization
     |
     v
Linux Subsystem Interface
     |
     v
User Space
```

For this **BeagleBone Black Complete Device Driver Project**, the
next practical step is to implement the Device Tree and driver flow
for each peripheral:

```text
GPIO
  ↓
UART
  ↓
I2C
  ↓
SPI
  ↓
PWM
  ↓
ADC
  ↓
CAN
```

Each driver should be developed using the appropriate Linux kernel
subsystem rather than creating an independent custom interface.

```
```

