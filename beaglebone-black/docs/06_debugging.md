# `06_debugging.md`

Create:

```text
beaglebone-black/docs/06_debugging.md
```

````markdown
# 06 - Debugging Linux Device Drivers on BeagleBone Black

## Table of Contents

- [1. Overview](#1-overview)
- [2. Debugging Architecture](#2-debugging-architecture)
- [3. Debugging Workflow](#3-debugging-workflow)
- [4. Serial Console](#4-serial-console)
- [5. Kernel Logs with dmesg](#5-kernel-logs-with-dmesg)
- [6. printk and pr_* APIs](#6-printk-and-pr_apis)
- [7. Dynamic Debug](#7-dynamic-debug)
- [8. Device Tree Debugging](#8-device-tree-debugging)
- [9. Device Tree Compilation Errors](#9-device-tree-compilation-errors)
- [10. Verify Device Tree at Runtime](#10-verify-device-tree-at-runtime)
- [11. Driver Probe Debugging](#11-driver-probe-debugging)
- [12. Driver Matching Debugging](#12-driver-matching-debugging)
- [13. Check Driver Binding](#13-check-driver-binding)
- [14. Module Debugging](#14-module-debugging)
- [15. GPIO Debugging](#15-gpio-debugging)
- [16. UART Debugging](#16-uart-debugging)
- [17. I2C Debugging](#17-i2c-debugging)
- [18. SPI Debugging](#18-spi-debugging)
- [19. PWM Debugging](#19-pwm-debugging)
- [20. ADC Debugging](#20-adc-debugging)
- [21. CAN Debugging](#21-can-debugging)
- [22. Pinmux Debugging](#22-pinmux-debugging)
- [23. Interrupt Debugging](#23-interrupt-debugging)
- [24. Clock Debugging](#24-clock-debugging)
- [25. Sysfs Debugging](#25-sysfs-debugging)
- [26. Debugfs](#26-debugfs)
- [27. Ftrace](#27-ftrace)
- [28. Tracepoints](#28-tracepoints)
- [29. Function Tracer](#29-function-tracer)
- [30. Kernel Oops](#30-kernel-oops)
- [31. Kernel Panic](#31-kernel-panic)
- [32. NULL Pointer Dereference](#32-null-pointer-dereference)
- [33. Use-After-Free](#33-use-after-free)
- [34. Kernel Stack Overflow](#34-kernel-stack-overflow)
- [35. Race Condition Debugging](#35-race-condition-debugging)
- [36. Deadlock Debugging](#36-deadlock-debugging)
- [37. Lockdep](#37-lockdep)
- [38. KASAN](#38-kasan)
- [39. KCSAN](#39-kcsan)
- [40. kmemleak](#40-kmemleak)
- [41. GDB Debugging](#41-gdb-debugging)
- [42. KGDB](#42-kgdb)
- [43. JTAG Debugging](#43-jtag-debugging)
- [44. Crash Dump Analysis](#44-crash-dump-analysis)
- [45. Performance Debugging](#45-performance-debugging)
- [46. Common Driver Failures](#46-common-driver-failures)
- [47. Peripheral Debugging Matrix](#47-peripheral-debugging-matrix)
- [48. Complete Driver Debugging Flow](#48-complete-driver-debugging-flow)
- [49. Recommended Debugging Checklist](#49-recommended-debugging-checklist)
- [50. Interview Explanation](#50-interview-explanation)
- [51. Summary](#51-summary)

---

# 1. Overview

Debugging is one of the most important parts of Linux device-driver
development.

In this BeagleBone Black project, debugging is required for:

```text
GPIO
UART
I2C
SPI
PWM
ADC
CAN
Device Tree
Interrupts
DMA
Clocks
Power
Kernel Modules
Custom Drivers
````

A driver can compile successfully but still fail at runtime.

Typical failures include:

```text
Driver does not probe
Device not found
Wrong Device Tree configuration
Pinmux conflict
I2C communication failure
SPI transfer failure
Interrupt not triggered
GPIO not responding
Kernel crash
Memory corruption
Race condition
Deadlock
```

The debugging process should therefore start from the hardware and
continue through the complete Linux software stack.

---

# 2. Debugging Architecture

The complete debugging path is:

```text
+----------------------+
|      Hardware        |
+----------+-----------+
           |
           v
+----------------------+
|      Pinmux          |
+----------+-----------+
           |
           v
+----------------------+
|    Device Tree       |
+----------+-----------+
           |
           v
+----------------------+
| Linux Device Model   |
+----------+-----------+
           |
           v
+----------------------+
| Driver Matching      |
+----------+-----------+
           |
           v
+----------------------+
|      probe()         |
+----------+-----------+
           |
           v
+----------------------+
| Linux Subsystem      |
+----------+-----------+
           |
           v
+----------------------+
| User-Space Test      |
+----------------------+
```

Debugging should follow the same direction.

---

# 3. Debugging Workflow

Use this general workflow:

```text
Problem
   |
   v
Check Hardware
   |
   v
Check Pinmux
   |
   v
Check Device Tree
   |
   v
Check Kernel Configuration
   |
   v
Check Driver Registration
   |
   v
Check Device/Driver Matching
   |
   v
Check probe()
   |
   v
Check Runtime Interface
   |
   v
Check Hardware Communication
   |
   v
Check User-Space Application
```

The first question should always be:

> Where does the failure occur?

For example:

```text
Device Tree
      |
      X
   Failure

```

or:

```text
Device Tree
      |
      v
Driver Match
      |
      X
   Failure
```

or:

```text
Driver Probe
      |
      v
Hardware Init
      |
      X
   Failure
```

---

# 4. Serial Console

The serial console is one of the most important debugging interfaces
for an Embedded Linux board.

Typical connection:

```text
PC
 |
 | USB-UART
 |
 v
BeagleBone Black
 |
 v
UART Console
```

Find the serial device:

```bash
ls /dev/ttyUSB*
```

or:

```bash
ls /dev/ttyACM*
```

Example:

```text
/dev/ttyUSB0
```

Open the console:

```bash
screen /dev/ttyUSB0 115200
```

Typical serial configuration:

```text
Baud Rate : 115200
Data Bits : 8
Parity    : None
Stop Bits : 1
Flow Ctrl : None
```

The serial console allows you to observe:

```text
U-Boot
Kernel boot
Device Tree messages
Driver probe
Kernel warnings
Kernel Oops
Kernel panic
```

---

# 5. Kernel Logs with dmesg

`dmesg` is the first debugging tool to use for kernel-driver issues.

Display all kernel messages:

```bash
dmesg
```

Display the latest messages:

```bash
dmesg | tail -50
```

Follow messages:

```bash
dmesg -w
```

Search for GPIO:

```bash
dmesg | grep -i gpio
```

Search for I2C:

```bash
dmesg | grep -i i2c
```

Search for SPI:

```bash
dmesg | grep -i spi
```

Search for UART:

```bash
dmesg | grep -i uart
```

Search for PWM:

```bash
dmesg | grep -i pwm
```

Search for CAN:

```bash
dmesg | grep -i can
```

Search for a custom driver:

```bash
dmesg | grep -i bbb
```

---

# 6. printk and pr_* APIs

Kernel drivers can print debugging information using kernel logging
APIs.

Basic:

```c
printk(KERN_INFO "BBB driver initialized\n");
```

Preferred convenience APIs include:

```c
pr_info("BBB driver initialized\n");
pr_err("BBB driver error\n");
pr_warn("BBB driver warning\n");
pr_debug("BBB debug message\n");
```

For device-specific messages:

```c
dev_info(&pdev->dev, "Device initialized\n");
dev_err(&pdev->dev, "Hardware initialization failed\n");
dev_warn(&pdev->dev, "Unexpected device state\n");
```

Example:

```c
static int bbb_probe(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "BBB driver probe started\n");

    return 0;
}
```

Check:

```bash
dmesg | tail
```

---

# 7. Dynamic Debug

Dynamic debug allows debug messages to be enabled at runtime without
rebuilding the entire kernel.

Check whether dynamic debug is available:

```bash
mount -t debugfs none /sys/kernel/debug
```

Check:

```bash
ls /sys/kernel/debug/dynamic_debug/
```

A module's debug statements can then be selectively enabled.

Example:

```bash
echo 'module bbb_driver +p' \
    > /sys/kernel/debug/dynamic_debug/control
```

View dynamic debug configuration:

```bash
cat /sys/kernel/debug/dynamic_debug/control
```

This is useful when a driver produces too many messages.

---

# 8. Device Tree Debugging

Device Tree problems are extremely common in embedded Linux.

Typical problems:

```text
Wrong node
Wrong compatible
Wrong pinctrl
Wrong GPIO
Wrong interrupt
Wrong clock
Wrong address
Wrong status
Wrong bus
Wrong Device Tree overlay
```

Example:

```dts
demo_device {
    compatible = "bbb,demo";
    status = "okay";
};
```

The driver must contain a matching string:

```c
static const struct of_device_id bbb_of_match[] = {
    {
        .compatible = "bbb,demo",
    },
    { }
};
```

If the strings do not match:

```text
Device Tree
     |
     | compatible = "bbb,demo"
     |
     X
Driver expects "bbb,test"
```

The driver will not bind.

---

# 9. Device Tree Compilation Errors

Compile a DTS:

```bash
dtc -I dts -O dtb \
    -o bbb-test.dtb \
    bbb-test.dts
```

If the Device Tree contains an error, `dtc` reports it.

Example:

```text
Error: bbb-test.dts:20.5-6 syntax error
```

Check:

```text
line number
column number
node name
property syntax
```

For kernel Device Trees, prefer building through the kernel build
system:

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- dtbs
```

---

# 10. Verify Device Tree at Runtime

Linux exposes the active Device Tree through:

```text
/sys/firmware/devicetree/base/
```

List nodes:

```bash
ls /sys/firmware/devicetree/base/
```

Check model:

```bash
tr -d '\0' < /sys/firmware/devicetree/base/model
echo
```

Check compatible:

```bash
tr '\0' '\n' \
    < /sys/firmware/devicetree/base/compatible
```

Search for a node:

```bash
find /sys/firmware/devicetree/base \
    -iname "*i2c*"
```

This helps verify what Device Tree was actually loaded by the kernel.

---

# 11. Driver Probe Debugging

The `probe()` function is one of the most important points in a
Linux driver.

Example:

```c
static int bbb_probe(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "probe started\n");

    return 0;
}
```

Add messages at every important stage:

```c
static int bbb_probe(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "probe started\n");

    dev_info(&pdev->dev, "getting resources\n");

    dev_info(&pdev->dev, "initializing hardware\n");

    dev_info(&pdev->dev, "creating interface\n");

    dev_info(&pdev->dev, "probe successful\n");

    return 0;
}
```

If you see:

```text
probe started
getting resources
```

but not:

```text
initializing hardware
```

the failure is between those stages.

---

# 12. Driver Matching Debugging

Linux matches devices and drivers using mechanisms such as:

```text
Device Tree compatible
Platform ID
I2C device ID
SPI device ID
USB device ID
PCI device ID
ACPI ID
```

For Device Tree:

```text
compatible
     |
     v
of_match_table
     |
     v
probe()
```

Example:

```c
static const struct of_device_id bbb_of_match[] = {
    {
        .compatible = "bbb,gpio-device",
    },
    { }
};

MODULE_DEVICE_TABLE(of, bbb_of_match);
```

If the driver is not probing, verify the `compatible` string first.

---

# 13. Check Driver Binding

Check platform devices:

```bash
ls /sys/bus/platform/devices/
```

Check platform drivers:

```bash
ls /sys/bus/platform/drivers/
```

For a particular driver:

```bash
ls /sys/bus/platform/drivers/<driver-name>/
```

A bound device commonly appears through a `driver` symlink.

Example:

```bash
ls -l /sys/bus/platform/devices/<device-name>/driver
```

If the device has no driver binding, investigate:

```text
compatible
driver registration
kernel configuration
probe errors
Device Tree status
```

---

# 14. Module Debugging

List loaded modules:

```bash
lsmod
```

Load a module:

```bash
sudo insmod bbb_driver.ko
```

or:

```bash
sudo modprobe bbb_driver
```

Check:

```bash
dmesg | tail -30
```

Remove:

```bash
sudo rmmod bbb_driver
```

Check module information:

```bash
modinfo bbb_driver.ko
```

Important information includes:

```text
filename
license
description
author
depends
alias
```

If module loading fails:

```bash
dmesg | tail -50
```

---

# 15. GPIO Debugging

GPIO debugging should start with:

```text
Pinmux
   |
   v
GPIO controller
   |
   v
GPIO descriptor
   |
   v
Driver
```

Check GPIO controllers:

```bash
ls /sys/class/gpio/
```

Modern GPIO interfaces use the GPIO character-device subsystem.

Check:

```bash
gpiodetect
```

List GPIO lines:

```bash
gpioinfo
```

Read a GPIO line:

```bash
gpioget <chip> <line>
```

Set a GPIO:

```bash
gpioset <chip> <line>=1
```

The exact GPIO chip and line depend on the board's current GPIO
configuration.

Debug Device Tree:

```text
gpio-controller
gpio-cells
gpios
pinctrl
```

---

# 16. UART Debugging

Check serial devices:

```bash
ls /dev/tty*
```

Check kernel messages:

```bash
dmesg | grep -i tty
```

Check serial configuration:

```bash
stty -F /dev/ttyS* -a
```

Test transmit:

```bash
echo "Hello BBB" > /dev/ttyS*
```

Test receive using:

```bash
cat /dev/ttyS*
```

For a custom UART driver, debug:

```text
probe()
UART registers
clock
baud rate
FIFO
interrupt
TX
RX
```

Use an oscilloscope or logic analyzer when electrical-level
verification is required.

---

# 17. I2C Debugging

Check I2C controllers:

```bash
ls /dev/i2c-*
```

Example:

```text
/dev/i2c-0
/dev/i2c-1
```

Install I2C tools if required:

```bash
sudo apt install i2c-tools
```

List adapters:

```bash
i2cdetect -l
```

Scan a bus:

```bash
sudo i2cdetect -y 1
```

Read a register:

```bash
sudo i2cget -y 1 0x50 0x00
```

Write a register:

```bash
sudo i2cset -y 1 0x50 0x00 0x12
```

Check kernel messages:

```bash
dmesg | grep -i i2c
```

Debug sequence:

```text
I2C controller
     |
     v
Pinmux
     |
     v
SDA/SCL
     |
     v
Slave address
     |
     v
Driver
     |
     v
I2C transfer
```

Hardware-level debugging should check:

```text
SCL waveform
SDA waveform
START
Address
ACK
Data
STOP
```

A logic analyzer is especially useful for this.

---

# 18. SPI Debugging

Check SPI devices:

```bash
ls /sys/bus/spi/devices/
```

Check kernel messages:

```bash
dmesg | grep -i spi
```

Check Device Tree:

```text
spi@...
    |
    +-- status
    +-- pinctrl
    +-- cs-gpios
    +-- child device
```

Important SPI signals:

```text
SCLK
MOSI
MISO
CS
```

Debug:

```text
SPI Controller
      |
      v
Pinmux
      |
      v
Chip Select
      |
      v
SPI Mode
      |
      v
Clock
      |
      v
Transfer
```

SPI mode parameters:

```text
Mode 0
Mode 1
Mode 2
Mode 3
```

If data is incorrect, verify:

```text
CPOL
CPHA
clock frequency
bit order
word length
chip select
```

Use a logic analyzer to confirm the physical transaction.

---

# 19. PWM Debugging

Check PWM framework:

```bash
ls /sys/class/pwm/
```

Depending on kernel version and userspace configuration, PWM
controllers and channels may appear differently.

Debug:

```text
PWM Controller
      |
      v
Channel
      |
      v
Period
      |
      v
Duty Cycle
      |
      v
Enable
```

Typical parameters:

```text
period
duty_cycle
enable
polarity
```

A PWM signal can be verified with:

```text
Oscilloscope
Logic Analyzer
```

Check kernel messages:

```bash
dmesg | grep -i pwm
```

---

# 20. ADC Debugging

ADC functionality is commonly exposed through the Industrial I/O
subsystem.

Check:

```bash
ls /sys/bus/iio/devices/
```

Example:

```text
iio:device0
```

List channels:

```bash
ls /sys/bus/iio/devices/iio:device0/
```

Check available raw channels:

```bash
find /sys/bus/iio/devices/iio:device0 \
    -name "*raw*"
```

Read a value:

```bash
cat /sys/bus/iio/devices/iio:device0/in_voltage*_raw
```

Debug sequence:

```text
ADC Hardware
     |
     v
ADC Controller
     |
     v
IIO Driver
     |
     v
IIO Device
     |
     v
sysfs
```

Always verify the voltage range and input electrical configuration
before applying signals to the ADC.

---

# 21. CAN Debugging

Check CAN interfaces:

```bash
ip link show
```

Search:

```bash
ip link show | grep can
```

Bring interface up:

```bash
sudo ip link set can0 up type can bitrate 500000
```

Check:

```bash
ip -details link show can0
```

Transmit:

```bash
cansend can0 123#DEADBEEF
```

Receive:

```bash
candump can0
```

Install CAN tools if required:

```bash
sudo apt install can-utils
```

Debug sequence:

```text
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
      |
      v
CAN Transceiver
      |
      v
CAN Bus
```

Important physical checks:

```text
CANH
CANL
GND
termination
bitrate
transceiver
```

---

# 22. Pinmux Debugging

A peripheral may fail because the physical pin is configured for a
different function.

Example:

```text
Pin
 |
 +-- GPIO
 |
 +-- UART
 |
 +-- SPI
 |
 +-- I2C
 |
 +-- PWM
```

Only the correct function should be selected.

Check Device Tree:

```text
pinctrl
pinctrl-names
pinctrl-0
```

Check kernel messages:

```bash
dmesg | grep -i pinctrl
```

If debugfs exposes pinctrl information:

```bash
mount -t debugfs none /sys/kernel/debug
```

Then inspect:

```bash
ls /sys/kernel/debug/pinctrl/
```

Pinmux debugging should be done before assuming the driver itself is
broken.

---

# 23. Interrupt Debugging

Check interrupt information:

```bash
cat /proc/interrupts
```

Example:

```text
           CPU0       CPU1
GPIO        120          0
I2C          45          0
```

Trigger the hardware event.

Then run:

```bash
cat /proc/interrupts
```

again.

If the counter increases:

```text
Interrupt is being generated
```

If it does not:

```text
Check IRQ configuration
Check GPIO
Check Device Tree
Check hardware signal
Check interrupt controller
```

Typical interrupt flow:

```text
Hardware
   |
   v
GPIO/Peripheral
   |
   v
Interrupt Controller
   |
   v
Kernel IRQ Handler
   |
   v
Driver ISR
   |
   v
Bottom Half / Workqueue
```

---

# 24. Clock Debugging

Many peripherals require clocks.

A driver can fail if the required clock is:

```text
disabled
incorrect frequency
missing
wrong parent
```

If the kernel exposes the clock debug interface:

```bash
mount -t debugfs none /sys/kernel/debug
```

Check:

```bash
ls /sys/kernel/debug/clk/
```

Depending on kernel configuration and platform support, clock
information can be inspected through debugfs.

Driver code commonly uses the kernel clock framework:

```c
clk = devm_clk_get(dev, NULL);
```

If this fails, check:

```text
Device Tree clock property
clock provider
clock name
kernel configuration
```

---

# 25. Sysfs Debugging

Sysfs exposes kernel objects to userspace.

Important locations:

```text
/sys/class/
/sys/bus/
/sys/devices/
/sys/firmware/
/sys/kernel/
```

Check platform devices:

```bash
ls /sys/bus/platform/devices/
```

Check I2C:

```bash
ls /sys/bus/i2c/devices/
```

Check SPI:

```bash
ls /sys/bus/spi/devices/
```

Check GPIO:

```bash
ls /sys/class/gpio/
```

Check PWM:

```bash
ls /sys/class/pwm/
```

Sysfs is useful for understanding how the kernel represents devices.

---

# 26. Debugfs

Debugfs provides kernel debugging information.

Mount:

```bash
sudo mount -t debugfs none /sys/kernel/debug
```

Check:

```bash
ls /sys/kernel/debug/
```

Possible debugging subsystems include:

```text
pinctrl
gpio
clk
tracing
dma_buf
```

Availability depends on kernel configuration and platform support.

---

# 27. Ftrace

Ftrace is a kernel tracing framework.

Check:

```bash
ls /sys/kernel/debug/tracing/
```

If the path is unavailable, verify debugfs and kernel tracing
configuration.

Enable function tracing:

```bash
echo function \
    > /sys/kernel/debug/tracing/current_tracer
```

View trace:

```bash
cat /sys/kernel/debug/tracing/trace
```

Stop tracing:

```bash
echo nop \
    > /sys/kernel/debug/tracing/current_tracer
```

Ftrace is useful for understanding:

```text
function calls
driver execution
scheduler activity
interrupt behavior
latency
```

---

# 28. Tracepoints

Tracepoints provide structured kernel instrumentation.

The tracing directory contains available events.

Example:

```bash
find /sys/kernel/debug/tracing/events \
    -maxdepth 2 -type d
```

Enable a specific event:

```bash
echo 1 > /sys/kernel/debug/tracing/events/<subsystem>/<event>/enable
```

Read:

```bash
cat /sys/kernel/debug/tracing/trace
```

Tracepoints are generally preferable to adding large numbers of
temporary print statements for complex tracing.

---

# 29. Function Tracer

Enable function tracing:

```bash
echo function \
    > /sys/kernel/debug/tracing/current_tracer
```

Clear previous trace:

```bash
echo > /sys/kernel/debug/tracing/trace
```

Start:

```bash
echo 1 > /sys/kernel/debug/tracing/tracing_on
```

Stop:

```bash
echo 0 > /sys/kernel/debug/tracing/tracing_on
```

Read:

```bash
cat /sys/kernel/debug/tracing/trace
```

This can help answer:

```text
Did probe() execute?
Which function executed next?
Where did execution stop?
How frequently is a function called?
```

---

# 30. Kernel Oops

A kernel Oops indicates a serious kernel fault.

Example:

```text
Unable to handle kernel NULL pointer dereference
```

Typical output includes:

```text
PC
LR
Call Trace
Registers
Process
Kernel version
```

Important information:

```text
PC
Call Trace
fault address
function names
line information
```

Do not reboot immediately if you need the complete serial log.

Save the Oops output.

---

# 31. Kernel Panic

A kernel panic is a fatal kernel failure from which the current
kernel cannot safely continue.

Example:

```text
Kernel panic - not syncing
```

Possible causes:

```text
NULL pointer
memory corruption
invalid access
deadlock
filesystem failure
hardware failure
driver bug
```

Debugging sequence:

```text
Kernel Panic
     |
     v
Capture Serial Log
     |
     v
Find Call Trace
     |
     v
Identify Faulting Function
     |
     v
Check Source Code
     |
     v
Reproduce
     |
     v
Fix Driver
```

---

# 32. NULL Pointer Dereference

Example bad code:

```c
struct bbb_device *dev = NULL;

dev->value = 10;
```

This can cause:

```text
NULL pointer dereference
```

Protect pointers:

```c
if (!dev)
    return -EINVAL;
```

For kernel debugging:

```bash
dmesg
```

Look for:

```text
Unable to handle kernel NULL pointer dereference
```

Then inspect the call trace.

---

# 33. Use-After-Free

A use-after-free occurs when memory is accessed after it has been
released.

Bad sequence:

```text
allocate
   |
   v
use
   |
   v
free
   |
   v
use again    <-- BUG
```

Typical causes:

```text
incorrect remove()
asynchronous work
interrupt handler
workqueue
timer
reference counting
```

Important tools:

```text
KASAN
kmemleak
slab debugging
```

---

# 34. Kernel Stack Overflow

Kernel stack space is limited.

Avoid large local arrays:

```c
void function(void)
{
    char buffer[10000];
}
```

Prefer dynamically allocated memory when appropriate:

```c
buffer = kmalloc(size, GFP_KERNEL);
```

or managed allocation:

```c
buffer = devm_kzalloc(dev, size, GFP_KERNEL);
```

Kernel stack problems can be difficult to diagnose because they may
appear as unrelated memory corruption.

---

# 35. Race Condition Debugging

A race condition occurs when multiple execution contexts access shared
data without correct synchronization.

Example:

```text
CPU0                    CPU1
 |                       |
 | read variable         |
 |                       |
 |                       | write variable
 |                       |
 | write variable        |
```

Possible synchronization mechanisms:

```text
mutex
spinlock
semaphore
completion
atomic operations
wait queues
RCU
```

For interrupt context:

```c
spin_lock_irqsave()
spin_unlock_irqrestore()
```

For process context:

```c
mutex_lock()
mutex_unlock()
```

Choose synchronization according to the execution context and
requirements.

---

# 36. Deadlock Debugging

Deadlock example:

```text
Thread A                Thread B

lock(A)                 lock(B)

lock(B)                 lock(A)

    ^                       ^
    |                       |
   WAIT                    WAIT
```

Both threads wait forever.

Typical causes:

```text
incorrect lock ordering
recursive locking
sleeping in inappropriate contexts
missing unlock
```

Debug using:

```text
lockdep
ftrace
kernel logs
stack traces
```

---

# 37. Lockdep

Lockdep detects many locking dependency problems.

It can identify:

```text
deadlocks
lock inversion
recursive locking
invalid lock usage
```

Enable appropriate lock debugging options in kernel configuration,
then inspect kernel logs.

Search:

```bash
dmesg | grep -i lockdep
```

A lockdep report should be analyzed from the first reported locking
relationship rather than only the final warning.

---

# 38. KASAN

KASAN detects memory safety bugs such as:

```text
out-of-bounds access
use-after-free
invalid memory access
```

Enable KASAN through:

```bash
make menuconfig
```

The exact configuration location and availability depend on the
kernel version and architecture.

KASAN adds runtime overhead and is therefore primarily useful during
debug builds.

After boot:

```bash
dmesg | grep -i kasan
```

---

# 39. KCSAN

KCSAN is designed to detect certain data-race bugs.

It is useful for:

```text
concurrent access
missing synchronization
shared variable races
```

Enable the appropriate KCSAN configuration options in the kernel.

Then reproduce the problem and inspect:

```bash
dmesg
```

KCSAN is especially useful for difficult multithreading bugs.

---

# 40. kmemleak

`kmemleak` can help detect memory that appears to have been allocated
but is no longer reachable.

If enabled:

```bash
mount -t debugfs none /sys/kernel/debug
```

Check:

```bash
cat /sys/kernel/debug/kmemleak
```

Scan:

```bash
echo scan > /sys/kernel/debug/kmemleak
```

Then:

```bash
cat /sys/kernel/debug/kmemleak
```

Use this during development rather than enabling heavyweight memory
debugging on production systems without a reason.

---

# 41. GDB Debugging

GDB can be used for user-space applications and, with the appropriate
kernel debugging setup, kernel debugging.

For a user-space application:

```bash
gdb ./test_app
```

Useful commands:

```gdb
break main
run
next
step
continue
backtrace
print variable
info registers
```

Example:

```gdb
break bbb_test
run
```

For kernel debugging, the kernel should normally be built with
debugging symbols.

---

# 42. KGDB

KGDB allows kernel debugging using a debugger connection.

Typical architecture:

```text
+----------------+
| Development PC |
|                |
| GDB            |
+-------+--------+
        |
        | Debug connection
        |
        v
+----------------+
| BeagleBone     |
|                |
| Linux Kernel   |
+----------------+
```

KGDB can allow:

```text
breakpoints
single stepping
register inspection
memory inspection
call stack inspection
```

KGDB requires kernel configuration and an appropriate transport/debug
setup.

---

# 43. JTAG Debugging

JTAG provides low-level hardware debugging.

Typical architecture:

```text
JTAG Debugger
      |
      v
BeagleBone Black
      |
      v
CPU
      |
      v
Kernel
```

JTAG can be useful when:

```text
kernel hangs
early boot fails
serial console unavailable
severe kernel crash
low-level hardware debugging required
```

Typical debugging levels:

```text
Application
     |
     v
Kernel
     |
     v
Bootloader
     |
     v
CPU
     |
     v
Hardware
```

JTAG becomes particularly valuable near the bottom of this stack.

---

# 44. Crash Dump Analysis

For severe kernel failures, crash dumps can preserve information for
post-mortem analysis.

A typical workflow is:

```text
Kernel Crash
     |
     v
Crash Dump
     |
     v
Collect vmcore
     |
     v
Analyze with crash
     |
     v
Call Trace
     |
     v
Faulting Driver
```

The exact crash-dump configuration depends on the kernel and target
system.

Useful information includes:

```text
CPU registers
call trace
process state
kernel memory
loaded modules
```

---

# 45. Performance Debugging

Driver problems are not always functional failures.

You may also need to debug:

```text
latency
CPU utilization
interrupt frequency
memory usage
I/O performance
DMA throughput
```

Useful tools include:

```text
top
htop
vmstat
iostat
perf
ftrace
tracepoints
```

Example:

```bash
top
```

Check memory:

```bash
free -h
```

Check CPU:

```bash
vmstat 1
```

For performance analysis:

```bash
perf top
```

---

# 46. Common Driver Failures

## Failure 1 - Driver Does Not Probe

Check:

```text
compatible
CONFIG option
module loaded
driver registration
Device Tree status
```

Commands:

```bash
dmesg
lsmod
ls /sys/bus/platform/devices/
```

---

## Failure 2 - `probe()` Returns Error

Add logs:

```c
dev_info(dev, "probe stage 1\n");
dev_info(dev, "probe stage 2\n");
dev_info(dev, "probe stage 3\n");
```

Check the exact error code.

Example:

```c
return -ENODEV;
```

means the driver indicates that the device is not available.

---

## Failure 3 - I2C Device Not Detected

Check:

```text
SDA
SCL
pull-ups
address
pinmux
controller
Device Tree
```

Then:

```bash
i2cdetect -l
```

and:

```bash
i2cdetect -y <bus>
```

---

## Failure 4 - SPI Data Incorrect

Check:

```text
CPOL
CPHA
frequency
CS
MISO
MOSI
bit order
```

Use a logic analyzer.

---

## Failure 5 - GPIO Does Not Toggle

Check:

```text
pinmux
GPIO number/descriptor
Device Tree
direction
permissions
hardware connection
```

---

## Failure 6 - Interrupt Not Triggering

Check:

```bash
cat /proc/interrupts
```

Then verify:

```text
IRQ number
interrupt flags
GPIO
hardware signal
Device Tree
```

---

## Failure 7 - Module Will Not Load

Run:

```bash
sudo insmod bbb_driver.ko
```

Then:

```bash
dmesg | tail -50
```

Check:

```text
kernel version
module architecture
module dependencies
symbol errors
license
```

---

# 47. Peripheral Debugging Matrix

| Peripheral | First Check        | Kernel Interface | Hardware Tool             |
| ---------- | ------------------ | ---------------- | ------------------------- |
| GPIO       | Pinmux + GPIO line | GPIO subsystem   | Multimeter/logic analyzer |
| UART       | `/dev/tty*` + logs | TTY              | USB-UART/scope            |
| I2C        | `i2cdetect`        | I2C              | Logic analyzer            |
| SPI        | SPI devices/logs   | SPI              | Logic analyzer            |
| PWM        | PWM device         | PWM              | Oscilloscope              |
| ADC        | IIO device         | IIO              | Multimeter                |
| CAN        | `ip link`          | SocketCAN        | CAN analyzer              |
| IRQ        | `/proc/interrupts` | IRQ subsystem    | Scope/logic analyzer      |

---

# 48. Complete Driver Debugging Flow

For every device in this project, follow this flow:

```text
                     Hardware
                         |
                         v
                  Check Wiring
                         |
                         v
                    Check Pinmux
                         |
                         v
                  Check Device Tree
                         |
                         v
                 Check Kernel Config
                         |
                         v
                Check Driver Loaded
                         |
                         v
                Check Device Created
                         |
                         v
                 Check Driver Match
                         |
                         v
                      probe()
                         |
              +----------+----------+
              |                     |
              v                     v
           Success                Failure
              |                     |
              v                     v
        Hardware Init          Check Error
              |                     |
              v                     v
        Runtime Interface      dmesg / trace
              |                     |
              v                     |
          User Test <---------------+
              |
              v
        Hardware Validation
```

---

# 49. Recommended Debugging Checklist

## Boot

```text
[ ] U-Boot starts
[ ] Kernel loads
[ ] Device Tree loads
[ ] Linux starts
[ ] Login prompt appears
```

## Kernel

```text
[ ] Correct kernel version
[ ] Correct .config
[ ] Required subsystem enabled
[ ] No kernel panic
[ ] No unexpected Oops
```

## Device Tree

```text
[ ] Correct node
[ ] compatible is correct
[ ] status = "okay"
[ ] pinctrl correct
[ ] GPIO correct
[ ] IRQ correct
[ ] clocks correct
[ ] addresses correct
```

## Driver

```text
[ ] Driver compiled
[ ] Module loads
[ ] Driver registered
[ ] Device matched
[ ] probe() called
[ ] Resources acquired
[ ] Hardware initialized
[ ] remove() works
```

## Runtime

```text
[ ] Device appears in Linux
[ ] Correct subsystem interface exists
[ ] Data transfer works
[ ] Interrupt works
[ ] Error handling works
[ ] Stress testing completed
```

---

# 50. Recommended Debug Message Pattern

For this project, use structured debug messages.

Example:

```c
static int bbb_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    dev_info(dev, "BBB probe: start\n");

    dev_info(dev, "BBB probe: getting resources\n");

    /*
     * Resource initialization
     */

    dev_info(dev, "BBB probe: initializing hardware\n");

    /*
     * Hardware initialization
     */

    dev_info(dev, "BBB probe: creating interfaces\n");

    /*
     * Interface creation
     */

    dev_info(dev, "BBB probe: success\n");

    return 0;
}
```

For errors:

```c
ret = bbb_hw_init(dev);

if (ret) {
    dev_err(dev,
            "hardware initialization failed: %d\n",
            ret);
    return ret;
}
```

This makes `dmesg` much easier to analyze.

---

# 51. Recommended Error Handling

Avoid ignoring return values.

Bad:

```c
bbb_hw_init(dev);
```

Better:

```c
ret = bbb_hw_init(dev);

if (ret)
    return ret;
```

Add context:

```c
ret = bbb_hw_init(dev);

if (ret) {
    dev_err(dev,
            "bbb_hw_init() failed: %d\n",
            ret);
    return ret;
}
```

Always investigate unexpected negative error codes.

---

# 52. Debugging With Error Codes

Common kernel error codes include:

```text
-EINVAL
-ENOMEM
-ENODEV
-ENXIO
-EBUSY
-EIO
-ETIMEDOUT
-EACCES
```

Examples:

```text
-EINVAL
Invalid argument

-ENOMEM
Memory allocation failure

-ENODEV
Device not available

-EBUSY
Resource already in use

-EIO
I/O error

-ETIMEDOUT
Operation timed out
```

When debugging, determine where the error was generated and what
resource caused it.

---

# 53. Debugging a Device Tree + Driver Example

Device Tree:

```dts
bbb_demo: demo@0 {
    compatible = "bbb,demo-device";
    status = "okay";
};
```

Driver:

```c
static const struct of_device_id bbb_demo_of_match[] = {
    {
        .compatible = "bbb,demo-device",
    },
    { }
};

MODULE_DEVICE_TABLE(of, bbb_demo_of_match);
```

Driver:

```c
static int bbb_demo_probe(struct platform_device *pdev)
{
    dev_info(&pdev->dev,
             "BBB demo device detected\n");

    return 0;
}
```

Boot:

```bash
dmesg | grep -i "BBB demo"
```

Expected:

```text
BBB demo device detected
```

This confirms:

```text
Device Tree
     |
     v
Device creation
     |
     v
Driver matching
     |
     v
probe()
```

---

# 54. Debugging a Failed Probe

Suppose:

```text
probe started
```

appears but:

```text
probe successful
```

does not.

Add:

```c
dev_info(dev, "stage 1\n");

resource = ...

dev_info(dev, "stage 2\n");

clk = ...

dev_info(dev, "stage 3\n");

gpio = ...

dev_info(dev, "stage 4\n");

hw_init();

dev_info(dev, "stage 5\n");
```

Then:

```bash
dmesg | grep "stage"
```

Output:

```text
stage 1
stage 2
stage 3
```

The failure is likely around the GPIO/resource stage.

This is a simple but highly effective driver-debugging technique.

---

# 55. Debugging Philosophy

Do not immediately modify driver code when hardware does not work.

Use:

```text
1. Hardware
2. Pinmux
3. Device Tree
4. Kernel Configuration
5. Driver Registration
6. Driver Matching
7. Probe
8. Runtime Interface
9. Protocol
10. Application
```

This prevents debugging the wrong layer.

---

# 56. Project Debugging Structure

The repository can maintain debugging documentation and scripts:

```text
beaglebone-black/
|
+-- docs/
|   |
|   +-- 06_debugging.md
|
+-- scripts/
|   |
|   +-- build_kernel.sh
|   +-- build_dtbs.sh
|   +-- collect_logs.sh
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
+-- drivers/
|   |
|   +-- gpio/
|   +-- uart/
|   +-- i2c/
|   +-- spi/
|   +-- pwm/
|   +-- adc/
|   +-- can/
```

---

# 57. Recommended Log Collection Script

Create:

```text
scripts/collect_logs.sh
```

```bash
#!/bin/bash

set -e

OUT="${1:-bbb-debug-logs}"

mkdir -p "$OUT"

echo "Collecting BeagleBone Black debug information..."

uname -a > "$OUT/uname.txt"
cat /proc/version > "$OUT/kernel-version.txt"
dmesg > "$OUT/dmesg.txt"
cat /proc/interrupts > "$OUT/interrupts.txt"
lsmod > "$OUT/modules.txt"

echo "Collecting Device Tree information..."

find /sys/firmware/devicetree/base \
    -maxdepth 2 \
    -type d \
    > "$OUT/device-tree-nodes.txt" 2>/dev/null || true

echo "Collecting bus information..."

ls /sys/bus/platform/devices/ \
    > "$OUT/platform-devices.txt" 2>/dev/null || true

ls /sys/bus/i2c/devices/ \
    > "$OUT/i2c-devices.txt" 2>/dev/null || true

ls /sys/bus/spi/devices/ \
    > "$OUT/spi-devices.txt" 2>/dev/null || true

echo "Debug information stored in:"
echo "$OUT"
```

Run:

```bash
chmod +x scripts/collect_logs.sh
```

Then:

```bash
./scripts/collect_logs.sh
```

This creates:

```text
bbb-debug-logs/
|
+-- uname.txt
+-- kernel-version.txt
+-- dmesg.txt
+-- interrupts.txt
+-- modules.txt
+-- device-tree-nodes.txt
+-- platform-devices.txt
+-- i2c-devices.txt
+-- spi-devices.txt
```

---

# 58. Practical Debugging Example

Suppose the I2C sensor is not detected.

Start:

```bash
i2cdetect -l
```

If no adapter exists:

```text
Check kernel configuration
Check Device Tree
Check controller
```

If adapter exists:

```bash
sudo i2cdetect -y 1
```

If device does not appear:

```text
Check:
    |
    +-- SDA
    +-- SCL
    +-- Pull-ups
    +-- Address
    +-- Power
    +-- Pinmux
```

If device appears but driver does not load:

```text
Check:
    |
    +-- compatible
    +-- driver
    +-- module
    +-- probe()
```

If driver probes but communication fails:

```text
Check:
    |
    +-- I2C transactions
    +-- register address
    +-- timing
    +-- device power
```

Use a logic analyzer if required.

---

# 59. Practical GPIO Debugging Example

Problem:

```text
GPIO output does not toggle.
```

Debug:

```text
1. Identify physical pin
2. Check pinmux
3. Check Device Tree
4. Check GPIO controller
5. Check GPIO line
6. Configure output
7. Toggle GPIO
8. Measure physical pin
```

Software:

```bash
gpioinfo
```

Then use the appropriate GPIO character-device utility.

Hardware:

```text
Multimeter
    or
Logic Analyzer
```

This separates a software configuration issue from a physical
electrical issue.

---

# 60. Practical Interrupt Debugging Example

Problem:

```text
Sensor interrupt is not received.
```

Check:

```bash
cat /proc/interrupts
```

Trigger the sensor.

Check again:

```bash
cat /proc/interrupts
```

If count does not change:

```text
Hardware signal
       |
       v
GPIO pin
       |
       v
Pinmux
       |
       v
Device Tree IRQ
       |
       v
Interrupt controller
```

If count increases but driver does not process the event:

```text
IRQ handler
    |
    v
Driver state
    |
    v
Workqueue/thread
    |
    v
Application
```

---

# 61. Production Debugging vs Development Debugging

During development:

```text
pr_debug()
dev_dbg()
ftrace
KASAN
KCSAN
lockdep
kmemleak
dynamic debug
```

can be enabled as appropriate.

For production:

```text
Reduce debug logging
Disable unnecessary tracing
Disable expensive debugging features
Keep error reporting
Keep recovery mechanisms
```

The production kernel should not blindly use a development debug
configuration.

---

# 62. Final Debugging Checklist

```text
BOOT
[ ] Serial console working
[ ] U-Boot verified
[ ] Kernel verified
[ ] Device Tree verified

KERNEL
[ ] Correct configuration
[ ] Required subsystem enabled
[ ] No panic
[ ] No Oops

DEVICE TREE
[ ] compatible correct
[ ] status correct
[ ] pinctrl correct
[ ] GPIO correct
[ ] IRQ correct
[ ] clocks correct

DRIVER
[ ] Module loads
[ ] Driver registers
[ ] Device matches
[ ] probe() executes
[ ] Resources acquired
[ ] Hardware initialized

RUNTIME
[ ] Device appears
[ ] Interface exists
[ ] Data transfer works
[ ] Interrupt works
[ ] Error handling works

HARDWARE
[ ] Voltage correct
[ ] Ground connected
[ ] Pin wiring correct
[ ] Signal verified
[ ] Timing verified
```

---

# 63. Interview Explanation

### 30-second answer

> "For Linux device-driver debugging on BeagleBone Black, I start from
> the bottom of the stack by checking hardware connections and pinmux,
> then verify the Device Tree and kernel configuration. I check
> whether the device is created and whether the driver matches the
> Device Tree compatible string. During probe, I use `dev_info`,
> `dev_err`, and dynamic debug to identify the failing stage. At
> runtime I use `dmesg`, sysfs, debugfs, `/proc/interrupts`, ftrace and
> subsystem-specific tools such as `i2cdetect`, GPIO tools and
> SocketCAN utilities. For severe kernel issues I use tools such as
> KASAN, lockdep, KGDB or JTAG depending on the problem."

---

# 64. Key Commands

Keep these commands available during driver development:

```bash
# Kernel information
uname -a
uname -r

# Kernel logs
dmesg
dmesg -w
dmesg | tail -50

# Modules
lsmod
modinfo <module>
insmod <module>.ko
rmmod <module>

# Device Tree
ls /sys/firmware/devicetree/base/
find /sys/firmware/devicetree/base/

# Devices
ls /sys/bus/platform/devices/
ls /sys/bus/i2c/devices/
ls /sys/bus/spi/devices/

# Interrupts
cat /proc/interrupts

# GPIO
gpiodetect
gpioinfo

# I2C
i2cdetect -l
i2cdetect -y <bus>

# CAN
ip link
ip -details link show can0
candump can0
cansend can0 123#DEADBEEF

# Tracing
mount -t debugfs none /sys/kernel/debug
ls /sys/kernel/debug/tracing/
```

---

# 65. Summary

The most important debugging principle in this project is:

```text
Do not guess.
Measure each layer.
```

Use this hierarchy:

```text
Hardware
   ↓
Pinmux
   ↓
Device Tree
   ↓
Kernel Configuration
   ↓
Device Creation
   ↓
Driver Matching
   ↓
probe()
   ↓
Hardware Initialization
   ↓
Linux Subsystem
   ↓
User-Space Test
```

For normal driver debugging, the most important tools are:

```text
dmesg
dev_info()
dev_err()
dynamic debug
sysfs
debugfs
/proc/interrupts
ftrace
gdb
KGDB
JTAG
KASAN
KCSAN
lockdep
kmemleak
```

For this BeagleBone Black project, every peripheral should follow the
same debugging methodology:

```text
GPIO
UART
I2C
SPI
PWM
ADC
CAN
```

The goal is not only to make the driver work, but to be able to
identify **which layer failed, why it failed, and how to prove the
failure using Linux debugging tools and hardware measurements**.

```
```

