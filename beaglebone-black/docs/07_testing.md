Create this file:

```text
beaglebone-black/docs/07_testing.md
```

````markdown
# 07 - Testing Linux Device Drivers on BeagleBone Black

## Table of Contents

- [1. Overview](#1-overview)
- [2. Testing Objectives](#2-testing-objectives)
- [3. Testing Architecture](#3-testing-architecture)
- [4. Test Environment](#4-test-environment)
- [5. Pre-Test Checklist](#5-pre-test-checklist)
- [6. Kernel and Device Tree Validation](#6-kernel-and-device-tree-validation)
- [7. Driver Module Testing](#7-driver-module-testing)
- [8. Driver Probe Testing](#8-driver-probe-testing)
- [9. GPIO Testing](#9-gpio-testing)
- [10. UART Testing](#10-uart-testing)
- [11. I2C Testing](#11-i2c-testing)
- [12. SPI Testing](#12-spi-testing)
- [13. PWM Testing](#13-pwm-testing)
- [14. ADC Testing](#14-adc-testing)
- [15. CAN Testing](#15-can-testing)
- [16. Interrupt Testing](#16-interrupt-testing)
- [17. Device Tree Testing](#17-device-tree-testing)
- [18. Pinmux Testing](#18-pinmux-testing)
- [19. Error Handling Tests](#19-error-handling-tests)
- [20. Remove and Reload Testing](#20-remove-and-reload-testing)
- [21. Stress Testing](#21-stress-testing)
- [22. Concurrency Testing](#22-concurrency-testing)
- [23. Memory Testing](#23-memory-testing)
- [24. Performance Testing](#24-performance-testing)
- [25. Power Management Testing](#25-power-management-testing)
- [26. Reboot Testing](#26-reboot-testing)
- [27. Fault Injection Testing](#27-fault-injection-testing)
- [28. Automated Testing](#28-automated-testing)
- [29. Test Scripts](#29-test-scripts)
- [30. Test Result Format](#30-test-result-format)
- [31. Peripheral Test Matrix](#31-peripheral-test-matrix)
- [32. Complete Test Flow](#32-complete-test-flow)
- [33. Test Checklist](#33-test-checklist)
- [34. Interview Explanation](#34-interview-explanation)
- [35. Summary](#35-summary)

---

# 1. Overview

This document defines the testing strategy for the BeagleBone Black
Linux Device Driver project.

The project covers:

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
Kernel Modules
Linux Driver Model
````

Testing is performed at multiple levels:

```text
Hardware
   ↓
Pinmux
   ↓
Device Tree
   ↓
Kernel
   ↓
Driver
   ↓
Linux Subsystem
   ↓
User Space
   ↓
Application
```

The purpose of testing is to verify:

* Device detection
* Driver registration
* Driver probe
* Hardware initialization
* Data transfer
* Interrupt handling
* Error handling
* Resource cleanup
* Stress behavior
* Performance
* Reliability

---

# 2. Testing Objectives

The primary objectives are:

```text
1. Verify hardware connectivity
2. Verify Device Tree configuration
3. Verify kernel configuration
4. Verify driver compilation
5. Verify driver registration
6. Verify driver-device matching
7. Verify probe()
8. Verify peripheral functionality
9. Verify interrupt handling
10. Verify error handling
11. Verify remove()
12. Verify repeated load/unload
13. Verify stress behavior
14. Verify memory safety
15. Verify performance
16. Verify system stability
```

---

# 3. Testing Architecture

The complete testing architecture is:

```text
                    +------------------+
                    |   Test Scripts  |
                    +--------+---------+
                             |
                             v
                    +------------------+
                    |   User Space     |
                    +--------+---------+
                             |
                             v
                    +------------------+
                    | Linux Subsystem  |
                    +--------+---------+
                             |
                             v
                    +------------------+
                    | Device Driver    |
                    +--------+---------+
                             |
                             v
                    +------------------+
                    |   Device Tree    |
                    +--------+---------+
                             |
                             v
                    +------------------+
                    |    Pinmux        |
                    +--------+---------+
                             |
                             v
                    +------------------+
                    |    Hardware      |
                    +------------------+
```

---

# 4. Test Environment

## Target Board

```text
Board:
    BeagleBone Black

Processor:
    TI Sitara AM335x

Architecture:
    ARM

Operating System:
    Embedded Linux
```

## Host Machine

Recommended:

```text
Ubuntu Linux
Git
GCC
Cross Compiler
Device Tree Compiler
Make
Python
Serial Terminal
```

## Hardware Tools

Recommended:

```text
USB-UART adapter
Multimeter
Logic analyzer
Oscilloscope
CAN analyzer
I2C devices
SPI devices
UART device
GPIO LEDs
PWM device
Analog signal source
```

---

# 5. Pre-Test Checklist

Before starting peripheral tests:

```text
[ ] Board powers on
[ ] Serial console works
[ ] Linux boots
[ ] Network/SSH works
[ ] Correct kernel is running
[ ] Correct Device Tree is loaded
[ ] Required kernel modules exist
[ ] Debug filesystem is available
[ ] Test tools are installed
```

Check kernel:

```bash
uname -a
```

Check boot messages:

```bash
dmesg | head -50
```

Check system:

```bash
cat /proc/version
```

---

# 6. Kernel and Device Tree Validation

## Check Kernel

```bash
uname -r
```

Verify that the expected kernel version is running.

## Check Device Tree

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

## Check Kernel Messages

```bash
dmesg | tail -100
```

Look for:

```text
error
failed
timeout
warning
probe
```

Example:

```bash
dmesg | grep -i "error\|fail\|timeout"
```

---

# 7. Driver Module Testing

Build the driver:

```bash
make
```

Load:

```bash
sudo insmod bbb_driver.ko
```

Check:

```bash
lsmod | grep bbb
```

Check logs:

```bash
dmesg | tail -30
```

Remove:

```bash
sudo rmmod bbb_driver
```

Check:

```bash
dmesg | tail -30
```

Expected:

```text
Driver loaded
Driver probe successful
Driver removed
```

---

# 8. Driver Probe Testing

The probe function is the first major driver test.

Example:

```c
static int bbb_probe(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "BBB probe started\n");

    /*
     * Resource initialization
     */

    dev_info(&pdev->dev, "BBB probe successful\n");

    return 0;
}
```

Check:

```bash
dmesg | grep -i "probe"
```

Expected:

```text
BBB probe started
BBB probe successful
```

Test cases:

```text
TC-PROBE-001  Device exists
TC-PROBE-002  Driver matches
TC-PROBE-003  probe() called
TC-PROBE-004  Resources acquired
TC-PROBE-005  Hardware initialized
TC-PROBE-006  Probe failure handled
```

---

# 9. GPIO Testing

GPIO testing verifies:

```text
Input
Output
Direction
Interrupt
Pinmux
GPIO resource management
```

## 9.1 Detect GPIO Controllers

```bash
gpiodetect
```

Example:

```text
gpiochip0
gpiochip1
```

## 9.2 Display GPIO Lines

```bash
gpioinfo
```

Check:

```text
line number
name
direction
consumer
```

## 9.3 GPIO Output Test

Use an appropriate GPIO character-device tool:

```bash
gpioset <gpiochip> <line>=1
```

Then:

```bash
gpioset <gpiochip> <line>=0
```

Verify the physical output with:

```text
LED
Multimeter
Logic Analyzer
```

## 9.4 GPIO Input Test

Read:

```bash
gpioget <gpiochip> <line>
```

Test:

```text
Input LOW
Input HIGH
```

## 9.5 GPIO Test Cases

| ID       | Test                   | Expected Result      |
| -------- | ---------------------- | -------------------- |
| GPIO-001 | Detect GPIO controller | Controller available |
| GPIO-002 | Read GPIO              | Correct value        |
| GPIO-003 | Set GPIO HIGH          | Pin HIGH             |
| GPIO-004 | Set GPIO LOW           | Pin LOW              |
| GPIO-005 | Input transition       | Correct state        |
| GPIO-006 | GPIO interrupt         | IRQ generated        |
| GPIO-007 | Invalid GPIO           | Error handled        |
| GPIO-008 | Busy GPIO              | Error handled        |

---

# 10. UART Testing

UART testing verifies:

```text
TX
RX
Baud rate
Parity
Stop bits
Flow control
Interrupts
Error handling
```

## 10.1 Find UART Devices

```bash
ls /dev/tty*
```

Search:

```bash
dmesg | grep -i tty
```

## 10.2 Configure UART

Example:

```bash
stty -F /dev/ttyS1 115200 cs8 -cstopb -parenb
```

Configuration:

```text
Baud    = 115200
Data    = 8 bits
Parity  = None
Stop    = 1
```

## 10.3 TX Test

```bash
echo "Hello BBB" > /dev/ttyS1
```

Verify on the receiving device.

## 10.4 RX Test

```bash
cat /dev/ttyS1
```

Send data from another UART device.

Expected:

```text
Received data == transmitted data
```

## 10.5 UART Loopback

Connect:

```text
TX ---- RX
```

Then send:

```bash
echo "UART TEST" > /dev/ttyS1
```

Expected:

```text
UART TEST
```

## 10.6 UART Test Cases

| ID       | Test                  | Expected              |
| -------- | --------------------- | --------------------- |
| UART-001 | Device detection      | UART exists           |
| UART-002 | TX                    | Data transmitted      |
| UART-003 | RX                    | Data received         |
| UART-004 | Loopback              | Same data received    |
| UART-005 | 115200 baud           | Correct communication |
| UART-006 | Parity test           | Correct behavior      |
| UART-007 | Large transfer        | No corruption         |
| UART-008 | Invalid configuration | Error handled         |

---

# 11. I2C Testing

I2C testing verifies:

```text
Bus detection
Slave address
Read
Write
Register access
Repeated transactions
Error handling
```

## 11.1 List I2C Controllers

```bash
i2cdetect -l
```

Expected:

```text
i2c-0
i2c-1
...
```

## 11.2 Scan Bus

```bash
sudo i2cdetect -y 1
```

Expected:

```text
Device address appears
```

Example:

```text
50
```

## 11.3 Read Register

```bash
sudo i2cget -y 1 0x50 0x00
```

## 11.4 Write Register

```bash
sudo i2cset -y 1 0x50 0x00 0x12
```

Only perform writes when the connected device and register map
support them.

## 11.5 I2C Hardware Test

Verify:

```text
SDA
SCL
GND
Power
Pull-up resistors
```

Use:

```text
Logic Analyzer
Oscilloscope
```

## 11.6 I2C Test Cases

| ID      | Test                 | Expected              |
| ------- | -------------------- | --------------------- |
| I2C-001 | Controller detection | Controller available  |
| I2C-002 | Bus scan             | Slave detected        |
| I2C-003 | Register read        | Correct value         |
| I2C-004 | Register write       | Correct value         |
| I2C-005 | Repeated read        | Stable result         |
| I2C-006 | Invalid address      | Error                 |
| I2C-007 | No device            | Timeout/error         |
| I2C-008 | Bus recovery         | Bus returns to normal |

---

# 12. SPI Testing

SPI testing verifies:

```text
SCLK
MOSI
MISO
CS
SPI mode
Clock frequency
Data transfer
```

## 12.1 Check SPI Devices

```bash
ls /sys/bus/spi/devices/
```

Check logs:

```bash
dmesg | grep -i spi
```

## 12.2 SPI Loopback

Connect:

```text
MOSI ---- MISO
```

Use an SPI test application or appropriate kernel/userspace interface.

Send:

```text
0x55
0xAA
0x12
0x34
```

Expected:

```text
Received == Transmitted
```

## 12.3 SPI Mode Test

Test:

```text
Mode 0
Mode 1
Mode 2
Mode 3
```

Use the mode supported by the target peripheral.

## 12.4 Logic Analyzer Test

Verify:

```text
CS
SCLK
MOSI
MISO
```

Check:

```text
Clock frequency
CPOL
CPHA
Data bits
Chip select timing
```

## 12.5 SPI Test Cases

| ID      | Test                 | Expected              |
| ------- | -------------------- | --------------------- |
| SPI-001 | Controller detection | SPI available         |
| SPI-002 | Device binding       | Driver bound          |
| SPI-003 | TX                   | Data transmitted      |
| SPI-004 | RX                   | Data received         |
| SPI-005 | Loopback             | Data matches          |
| SPI-006 | Mode                 | Correct communication |
| SPI-007 | High-speed transfer  | No corruption         |
| SPI-008 | CS                   | Correct selection     |

---

# 13. PWM Testing

PWM testing verifies:

```text
Period
Duty cycle
Polarity
Enable
Disable
```

Check PWM:

```bash
ls /sys/class/pwm/
```

Depending on the kernel version and configuration, the PWM interface
may differ.

## Test Parameters

Example:

```text
Frequency = 1 kHz
Period    = 1 ms
Duty      = 50%
```

Test:

```text
0%
25%
50%
75%
100%
```

Verify with:

```text
Oscilloscope
Logic Analyzer
```

Expected:

```text
Measured frequency == configured frequency
Measured duty cycle ~= configured duty cycle
```

## PWM Test Cases

| ID      | Test                 | Expected            |
| ------- | -------------------- | ------------------- |
| PWM-001 | Controller detection | PWM available       |
| PWM-002 | Enable               | PWM output active   |
| PWM-003 | Disable              | PWM output inactive |
| PWM-004 | 25% duty             | Correct duty        |
| PWM-005 | 50% duty             | Correct duty        |
| PWM-006 | 75% duty             | Correct duty        |
| PWM-007 | Frequency            | Correct frequency   |
| PWM-008 | Invalid values       | Error handled       |

---

# 14. ADC Testing

ADC is tested through the Industrial I/O subsystem when the selected
kernel/driver exposes the ADC through IIO.

Check:

```bash
ls /sys/bus/iio/devices/
```

Example:

```text
iio:device0
```

Check channels:

```bash
ls /sys/bus/iio/devices/iio:device0/
```

Find raw values:

```bash
find /sys/bus/iio/devices/iio:device0 \
    -name "*raw*"
```

Read:

```bash
cat /sys/bus/iio/devices/iio:device0/in_voltage*_raw
```

## ADC Test

Apply known input voltages within the board's supported ADC input
range.

Example:

```text
0 V
0.5 V
1.0 V
1.5 V
```

Record:

```text
Input voltage
Raw ADC value
Converted voltage
```

Expected:

```text
ADC result changes proportionally with input voltage
```

## ADC Test Cases

| ID      | Test                  | Expected           |
| ------- | --------------------- | ------------------ |
| ADC-001 | IIO device            | Device available   |
| ADC-002 | Channel detection     | Channel exists     |
| ADC-003 | 0 V                   | Near minimum       |
| ADC-004 | Known voltage         | Expected ADC range |
| ADC-005 | Repeated sample       | Stable result      |
| ADC-006 | Invalid channel       | Error              |
| ADC-007 | Maximum allowed input | Valid reading      |

---

# 15. CAN Testing

CAN testing verifies:

```text
CAN controller
SocketCAN
Bitrate
TX
RX
Error handling
Bus recovery
```

## 15.1 Check CAN

```bash
ip link
```

Check:

```bash
ip -details link show can0
```

## 15.2 Configure CAN

Example:

```bash
sudo ip link set can0 up type can bitrate 500000
```

## 15.3 Receive

```bash
candump can0
```

## 15.4 Transmit

```bash
cansend can0 123#DEADBEEF
```

Expected:

```text
CAN frame received correctly
```

## CAN Test Cases

| ID      | Test                | Expected           |
| ------- | ------------------- | ------------------ |
| CAN-001 | Interface detection | can0 available     |
| CAN-002 | Configure bitrate   | Interface up       |
| CAN-003 | TX                  | Frame transmitted  |
| CAN-004 | RX                  | Frame received     |
| CAN-005 | Loopback            | Frame matches      |
| CAN-006 | Invalid bitrate     | Error              |
| CAN-007 | Bus error           | Error detected     |
| CAN-008 | Recovery            | Interface recovers |

---

# 16. Interrupt Testing

Interrupt testing is critical for:

```text
GPIO
UART
I2C
SPI
CAN
ADC
Timers
```

Check:

```bash
cat /proc/interrupts
```

Record interrupt count:

```text
Before event
After event
```

Trigger hardware.

Run:

```bash
cat /proc/interrupts
```

again.

Expected:

```text
IRQ count increases
```

Example:

```text
Before:
GPIO IRQ = 100

After:
GPIO IRQ = 101
```

This confirms that an interrupt was generated and accounted for.

---

# 17. Device Tree Testing

Every Device Tree change must be tested.

Verify:

```text
[ ] Node exists
[ ] compatible correct
[ ] status correct
[ ] pinctrl correct
[ ] GPIO correct
[ ] IRQ correct
[ ] clocks correct
[ ] address correct
```

Check:

```bash
ls /sys/firmware/devicetree/base/
```

Search:

```bash
find /sys/firmware/devicetree/base \
    -iname "*gpio*"
```

Kernel log:

```bash
dmesg | grep -i "pinctrl\|gpio\|i2c\|spi\|uart"
```

---

# 18. Pinmux Testing

A peripheral cannot work correctly if its pins are configured for
another function.

Test:

```text
UART pins
I2C pins
SPI pins
PWM pins
GPIO pins
```

Verify Device Tree:

```text
pinctrl-names
pinctrl-0
```

Check runtime pinctrl information when available:

```bash
mount -t debugfs none /sys/kernel/debug
ls /sys/kernel/debug/pinctrl/
```

Physical validation:

```text
Logic analyzer
Oscilloscope
Multimeter
```

---

# 19. Error Handling Tests

A production-quality driver must be tested with invalid conditions.

Examples:

```text
Invalid GPIO
Invalid I2C address
Invalid SPI configuration
Invalid UART configuration
Missing hardware
Busy resource
Memory allocation failure
Timeout
Invalid user input
```

Expected behavior:

```text
Driver returns appropriate error
No kernel crash
Resources are released
System remains stable
```

Example:

```c
if (!device)
    return -ENODEV;
```

Test:

```text
Request invalid device
Observe return code
Check dmesg
Verify no memory/resource leak
```

---

# 20. Remove and Reload Testing

A driver must correctly clean up its resources.

Load:

```bash
sudo insmod bbb_driver.ko
```

Remove:

```bash
sudo rmmod bbb_driver
```

Repeat:

```bash
for i in $(seq 1 100)
do
    sudo insmod bbb_driver.ko
    sudo rmmod bbb_driver
done
```

Check:

```bash
dmesg
```

Expected:

```text
No kernel crash
No resource leak
No stale device
No duplicate registration
```

For drivers built into the kernel, test the relevant bind/unbind or
device lifecycle mechanisms where supported instead of using module
load/unload.

---

# 21. Stress Testing

Functional testing only proves that the driver works under normal
conditions.

Stress testing checks long-term behavior.

Example:

```text
1000 I2C transfers
10000 SPI transfers
10000 UART messages
Continuous GPIO interrupts
Continuous CAN traffic
Continuous ADC sampling
```

Monitor:

```bash
dmesg -w
```

and:

```bash
top
```

Look for:

```text
kernel errors
timeouts
memory growth
CPU overload
missed interrupts
data corruption
```

---

# 22. Concurrency Testing

Test simultaneous operations.

Example:

```text
Thread 1 -> I2C
Thread 2 -> GPIO
Thread 3 -> UART
Thread 4 -> SPI
```

For a shared driver resource:

```text
Thread A
    |
    +----> Driver
             |
Thread B ----+
```

Verify:

```text
No race condition
No deadlock
No data corruption
No kernel crash
```

Use:

```text
lockdep
KCSAN
ftrace
```

when available.

---

# 23. Memory Testing

Driver memory should be tested for:

```text
Allocation
Free
Double free
Use-after-free
Buffer overflow
Memory leak
```

Tools:

```text
KASAN
kmemleak
slab debugging
```

Test sequence:

```text
Load driver
   ↓
Allocate resources
   ↓
Use resources
   ↓
Remove driver
   ↓
Free resources
   ↓
Load again
```

Repeat many times.

---

# 24. Performance Testing

Performance metrics include:

```text
CPU usage
Interrupt latency
Data throughput
Transfer rate
Memory usage
I/O latency
```

Useful commands:

```bash
top
vmstat 1
free -h
```

Use `perf` where supported:

```bash
perf top
```

For driver execution timing, use:

```text
ftrace
tracepoints
perf
```

Example measurements:

```text
I2C transaction time
SPI transfer rate
UART throughput
CAN frames/sec
GPIO interrupt latency
```

---

# 25. Power Management Testing

Test driver behavior during:

```text
Suspend
Resume
Reboot
Shutdown
Idle
```

If the board/kernel configuration supports the relevant power states,
test:

```bash
cat /sys/power/state
```

Then perform the supported suspend operation.

Verify:

```text
Device resumes
Driver state restored
Peripheral works
Interrupts work
No resource leak
```

For a driver with PM callbacks:

```c
static int bbb_suspend(struct device *dev)
{
    return 0;
}

static int bbb_resume(struct device *dev)
{
    return 0;
}
```

Test both paths.

---

# 26. Reboot Testing

A driver must survive repeated system reboot.

Run:

```bash
sudo reboot
```

After reboot:

```bash
dmesg | grep -i bbb
```

Verify:

```text
Driver loads
Device probes
Peripheral works
No stale state
```

Stress reboot:

```text
Boot
 ↓
Test
 ↓
Reboot
 ↓
Boot
 ↓
Test
 ↓
Repeat
```

---

# 27. Fault Injection Testing

Fault injection intentionally creates failures to verify error paths.

Examples:

```text
Memory allocation failure
I/O failure
Timeout
Invalid parameter
Missing resource
Hardware disconnect
```

The purpose is to verify:

```text
Error is detected
Error is reported
Resources are cleaned up
System remains stable
```

Example driver path:

```text
Resource A acquired
      ↓
Resource B acquired
      ↓
Resource C fails
      ↓
Release B
      ↓
Release A
      ↓
Return error
```

This is especially important for complex `probe()` functions.

---

# 28. Automated Testing

Manual testing is useful during development.

For regression testing, automate:

```text
Build
Deploy
Load driver
Run test
Collect logs
Unload driver
Report result
```

Recommended structure:

```text
tests/
|
+-- gpio/
|   +-- test_gpio.sh
|
+-- uart/
|   +-- test_uart.sh
|
+-- i2c/
|   +-- test_i2c.sh
|
+-- spi/
|   +-- test_spi.sh
|
+-- pwm/
|   +-- test_pwm.sh
|
+-- adc/
|   +-- test_adc.sh
|
+-- can/
|   +-- test_can.sh
```

---

# 29. Test Scripts

## 29.1 Generic Test Script

Create:

```text
tests/common/test_environment.sh
```

```bash
#!/bin/bash

set -e

echo "================================"
echo "BeagleBone Black Test Environment"
echo "================================"

echo
echo "Kernel:"
uname -a

echo
echo "Kernel version:"
uname -r

echo
echo "CPU:"
cat /proc/cpuinfo | head -20

echo
echo "Memory:"
free -h

echo
echo "Loaded modules:"
lsmod

echo
echo "Interrupts:"
cat /proc/interrupts

echo
echo "Environment check completed."
```

Run:

```bash
chmod +x tests/common/test_environment.sh
./tests/common/test_environment.sh
```

---

# 30. GPIO Test Script

Create:

```text
tests/gpio/test_gpio.sh
```

```bash
#!/bin/bash

set -e

echo "================================"
echo "GPIO Test"
echo "================================"

echo
echo "GPIO controllers:"
gpiodetect

echo
echo "GPIO lines:"
gpioinfo

echo
echo "GPIO test environment ready."
```

For actual output/input testing, specify the GPIO chip and line
according to the board configuration used by this project.

---

# 31. I2C Test Script

Create:

```text
tests/i2c/test_i2c.sh
```

```bash
#!/bin/bash

set -e

BUS="${1:-1}"

echo "================================"
echo "I2C Test"
echo "================================"

echo
echo "I2C adapters:"
i2cdetect -l

echo
echo "Scanning I2C bus: $BUS"

sudo i2cdetect -y "$BUS"

echo
echo "I2C test completed."
```

Run:

```bash
chmod +x tests/i2c/test_i2c.sh
```

Then:

```bash
./tests/i2c/test_i2c.sh 1
```

---

# 32. SPI Test Script

Create:

```text
tests/spi/test_spi.sh
```

```bash
#!/bin/bash

set -e

echo "================================"
echo "SPI Test"
echo "================================"

echo
echo "SPI devices:"
ls -l /sys/bus/spi/devices/ || true

echo
echo "SPI kernel messages:"
dmesg | grep -i spi | tail -30 || true

echo
echo "SPI test completed."
```

---

# 33. UART Test Script

Create:

```text
tests/uart/test_uart.sh
```

```bash
#!/bin/bash

set -e

UART_DEVICE="${1:-/dev/ttyS1}"

echo "================================"
echo "UART Test"
echo "================================"

if [ ! -e "$UART_DEVICE" ]; then
    echo "UART device not found: $UART_DEVICE"
    exit 1
fi

echo "UART device: $UART_DEVICE"

stty -F "$UART_DEVICE" 115200 cs8 -cstopb -parenb

echo "UART configured."

echo "UART test completed."
```

Use the actual UART device enabled by the project's Device Tree.

---

# 34. PWM Test Script

Create:

```text
tests/pwm/test_pwm.sh
```

```bash
#!/bin/bash

set -e

echo "================================"
echo "PWM Test"
echo "================================"

echo
echo "PWM controllers:"
ls -la /sys/class/pwm/ 2>/dev/null || true

echo
echo "PWM test environment ready."
```

Actual PWM channel testing should use the channel exposed by the
kernel configuration.

---

# 35. ADC Test Script

Create:

```text
tests/adc/test_adc.sh
```

```bash
#!/bin/bash

set -e

echo "================================"
echo "ADC Test"
echo "================================"

echo
echo "IIO devices:"
ls -la /sys/bus/iio/devices/ 2>/dev/null || true

for dev in /sys/bus/iio/devices/iio:device*
do
    if [ -d "$dev" ]; then
        echo
        echo "Device: $dev"
        cat "$dev/name" 2>/dev/null || true
    fi
done

echo
echo "ADC test environment ready."
```

---

# 36. CAN Test Script

Create:

```text
tests/can/test_can.sh
```

```bash
#!/bin/bash

set -e

INTERFACE="${1:-can0}"

echo "================================"
echo "CAN Test"
echo "================================"

echo
echo "Network interfaces:"
ip link show

echo
echo "CAN interface:"
ip -details link show "$INTERFACE" 2>/dev/null || true

echo
echo "CAN test environment ready."
```

For an actual TX/RX test, use two CAN nodes or a supported CAN
loopback configuration.

---

# 37. Test Result Format

Each test should record:

```text
Test ID
Test Name
Date
Kernel Version
Device Tree Version
Hardware Revision
Expected Result
Actual Result
PASS/FAIL
Comments
```

Example:

```text
Test ID       : I2C-003
Test Name     : I2C Register Read
Kernel        : 6.x
Device Tree   : BBB-I2C-v1
Expected      : Register returns 0x55
Actual        : Register returns 0x55
Result        : PASS
Comments      : Stable across 100 reads
```

---

# 38. Test Result Markdown Template

Create:

```text
docs/test-results/
```

Example:

```markdown
# I2C Test Report

## Environment

| Parameter | Value |
|---|---|
| Board | BeagleBone Black |
| Kernel | 6.x |
| Device Tree | BBB-I2C-v1 |
| Date | YYYY-MM-DD |

## Test

| ID | Test | Expected | Actual | Result |
|---|---|---|---|---|
| I2C-001 | Detect adapter | Adapter available | Available | PASS |
| I2C-002 | Scan bus | Device detected | Detected | PASS |
| I2C-003 | Register read | Correct value | Correct | PASS |
| I2C-004 | Register write | Correct value | Correct | PASS |

## Conclusion

I2C driver passed all functional tests.
```

---

# 39. Peripheral Test Matrix

| Peripheral | Detection |  TX |  RX | Interrupt | Stress | Hardware Validation |
| ---------- | --------: | --: | --: | --------: | -----: | ------------------: |
| GPIO       |         ✓ |   ✓ |   ✓ |         ✓ |      ✓ |      Logic Analyzer |
| UART       |         ✓ |   ✓ |   ✓ |         ✓ |      ✓ |        Oscilloscope |
| I2C        |         ✓ |   ✓ |   ✓ |         ✓ |      ✓ |      Logic Analyzer |
| SPI        |         ✓ |   ✓ |   ✓ |         ✓ |      ✓ |      Logic Analyzer |
| PWM        |         ✓ |   ✓ | N/A |       N/A |      ✓ |        Oscilloscope |
| ADC        |         ✓ | N/A |   ✓ |  Optional |      ✓ |          Multimeter |
| CAN        |         ✓ |   ✓ |   ✓ |         ✓ |      ✓ |        CAN Analyzer |

---

# 40. Functional vs Stress Testing

## Functional Testing

Verifies:

```text
Does the driver work?
```

Example:

```text
I2C device detected
Register read works
Register write works
```

## Stress Testing

Verifies:

```text
Does the driver continue working under load?
```

Example:

```text
100,000 I2C transfers
Continuous UART traffic
Continuous CAN traffic
Repeated driver reload
```

Both are required.

---

# 41. Regression Testing

Whenever the driver or Device Tree changes:

```text
Build
 ↓
Deploy
 ↓
Boot
 ↓
Run smoke tests
 ↓
Run peripheral tests
 ↓
Run stress tests
 ↓
Collect logs
 ↓
Compare results
```

Regression testing prevents a change to one peripheral from breaking
another.

Example:

```text
Modify SPI Device Tree
        |
        v
Run SPI tests
        |
        v
Run GPIO tests
        |
        v
Run I2C tests
        |
        v
Run UART tests
```

This is important because pinmux resources can be shared or
conflicting.

---

# 42. Smoke Test

After every new kernel/Device Tree build, perform a quick smoke test:

```bash
uname -r
dmesg | tail -50
ls /dev/
gpiodetect
i2cdetect -l
ls /sys/bus/spi/devices/
ls /sys/class/pwm/
ls /sys/bus/iio/devices/
ip link
cat /proc/interrupts
```

Expected:

```text
No critical kernel errors
Required peripherals available
Driver probes successful
```

---

# 43. Full Regression Test

A complete regression should follow:

```text
                    BOOT
                      |
                      v
             Kernel Validation
                      |
                      v
            Device Tree Validation
                      |
                      v
              Driver Validation
                      |
        +-------------+-------------+
        |             |             |
        v             v             v
      GPIO          UART           I2C
        |             |             |
        +-------------+-------------+
                      |
                      v
                     SPI
                      |
                      v
                     PWM
                      |
                      v
                     ADC
                      |
                      v
                     CAN
                      |
                      v
                Interrupt Test
                      |
                      v
                Stress Testing
                      |
                      v
              Memory Validation
                      |
                      v
              Reboot Validation
                      |
                      v
               FINAL REPORT
```

---

# 44. Testing Logs

Store test logs:

```text
logs/
|
+-- boot.log
+-- gpio.log
+-- uart.log
+-- i2c.log
+-- spi.log
+-- pwm.log
+-- adc.log
+-- can.log
+-- interrupt.log
+-- stress.log
```

Example:

```bash
dmesg > logs/boot.log
```

I2C:

```bash
i2cdetect -l > logs/i2c.log
```

Interrupts:

```bash
cat /proc/interrupts > logs/interrupt.log
```

---

# 45. Pass/Fail Criteria

## PASS

A test is PASS when:

```text
Expected behavior occurs
No kernel error
No unexpected warning
No resource leak
No data corruption
Hardware signal is correct
```

## FAIL

A test is FAIL when:

```text
Driver does not probe
Device unavailable
Incorrect data
Timeout
Kernel warning
Kernel Oops
Kernel panic
Memory leak
Hardware signal incorrect
```

---

# 46. Example Complete I2C Test

```text
STEP 1
Check kernel
        |
        v
STEP 2
Check Device Tree
        |
        v
STEP 3
Check I2C adapter
        |
        v
STEP 4
Scan bus
        |
        v
STEP 5
Detect slave
        |
        v
STEP 6
Read register
        |
        v
STEP 7
Write register
        |
        v
STEP 8
Read back
        |
        v
STEP 9
Repeat 1000 times
        |
        v
STEP 10
Check dmesg
        |
        v
PASS
```

---

# 47. Example Complete SPI Test

```text
Check SPI controller
        |
        v
Check Device Tree
        |
        v
Check pinmux
        |
        v
Check CS
        |
        v
Configure SPI
        |
        v
Transmit data
        |
        v
Receive data
        |
        v
Compare data
        |
        v
Logic analyzer
        |
        v
Stress transfer
        |
        v
PASS
```

---

# 48. Example Complete UART Test

```text
Check UART device
        |
        v
Configure 115200 8N1
        |
        v
TX test
        |
        v
RX test
        |
        v
Loopback
        |
        v
Large data transfer
        |
        v
Interrupt verification
        |
        v
Stress test
        |
        v
PASS
```

---

# 49. Example Complete GPIO Test

```text
Check pinmux
        |
        v
Check GPIO line
        |
        v
Configure output
        |
        v
Set HIGH
        |
        v
Measure pin
        |
        v
Set LOW
        |
        v
Measure pin
        |
        v
Configure input
        |
        v
Read input
        |
        v
Generate interrupt
        |
        v
Verify /proc/interrupts
        |
        v
PASS
```

---

# 50. Test Coverage

The project should eventually cover:

```text
+-----------------------------+
| Test Coverage               |
+-----------------------------+
| Build                       |
| Boot                        |
| Device Tree                 |
| Driver Probe                |
| GPIO                        |
| UART                        |
| I2C                         |
| SPI                         |
| PWM                         |
| ADC                         |
| CAN                         |
| Interrupt                   |
| Error Handling              |
| Resource Cleanup            |
| Concurrency                 |
| Memory                      |
| Performance                 |
| Power Management            |
| Reboot                      |
| Stress                      |
+-----------------------------+
```

---

# 51. Recommended Test Directory

The repository should contain:

```text
beaglebone-black/
|
+-- docs/
|   |
|   +-- 01_architecture.md
|   +-- 02_hardware_setup.md
|   +-- 03_linux_driver_model.md
|   +-- 04_device_tree.md
|   +-- 05_kernel_build.md
|   +-- 06_debugging.md
|   +-- 07_testing.md
|
+-- tests/
|   |
|   +-- common/
|   |   +-- test_environment.sh
|   |
|   +-- gpio/
|   |   +-- test_gpio.sh
|   |
|   +-- uart/
|   |   +-- test_uart.sh
|   |
|   +-- i2c/
|   |   +-- test_i2c.sh
|   |
|   +-- spi/
|   |   +-- test_spi.sh
|   |
|   +-- pwm/
|   |   +-- test_pwm.sh
|   |
|   +-- adc/
|   |   +-- test_adc.sh
|   |
|   +-- can/
|       +-- test_can.sh
|
+-- logs/
|
+-- test-results/
```

---

# 52. Development Test Cycle

Every driver change should follow:

```text
Code
  ↓
Compile
  ↓
Deploy
  ↓
Boot
  ↓
Check dmesg
  ↓
Check probe
  ↓
Functional Test
  ↓
Hardware Test
  ↓
Stress Test
  ↓
Regression Test
  ↓
Document Result
```

Do not consider a driver complete just because it compiles.

---

# 53. Driver Validation Levels

## Level 1 - Build Test

```text
Driver compiles successfully
```

## Level 2 - Load Test

```text
Module loads successfully
```

## Level 3 - Probe Test

```text
Driver binds to device
```

## Level 4 - Functional Test

```text
Peripheral performs expected operation
```

## Level 5 - Hardware Test

```text
Electrical signals are correct
```

## Level 6 - Stress Test

```text
Driver survives continuous operation
```

## Level 7 - Regression Test

```text
Existing functionality remains intact
```

## Level 8 - Production Test

```text
Reliable operation under expected field conditions
```

---

# 54. Interview Explanation

### 30-second answer

> "For device-driver testing on BeagleBone Black, I use a layered
> approach. First I verify the kernel, Device Tree and driver probe.
> Then I perform subsystem-specific functional testing for GPIO, UART,
> I2C, SPI, PWM, ADC and CAN. I use tools such as `dmesg`, GPIO
> character-device tools, `i2cdetect`, SocketCAN utilities and
> `/proc/interrupts`. For physical validation I use a logic analyzer,
> oscilloscope and multimeter. After functional validation I perform
> stress, concurrency, memory, error-handling and reboot tests. Finally,
> I automate the tests and maintain regression reports so Device Tree
> or kernel changes do not break existing peripherals."

---

# 55. Key Testing Commands

```bash
# Kernel
uname -a
dmesg
dmesg -w

# Modules
lsmod
modinfo <module>
sudo insmod <module>.ko
sudo rmmod <module>

# Device Tree
ls /sys/firmware/devicetree/base/

# GPIO
gpiodetect
gpioinfo
gpioget <chip> <line>
gpioset <chip> <line>=1

# UART
ls /dev/tty*
stty -F /dev/ttyS* -a

# I2C
i2cdetect -l
i2cdetect -y <bus>
i2cget -y <bus> <address> <register>
i2cset -y <bus> <address> <register> <value>

# SPI
ls /sys/bus/spi/devices/
dmesg | grep -i spi

# PWM
ls /sys/class/pwm/

# ADC
ls /sys/bus/iio/devices/

# CAN
ip link
ip -details link show can0
candump can0
cansend can0 123#DEADBEEF

# Interrupts
cat /proc/interrupts

# Debug filesystem
mount -t debugfs none /sys/kernel/debug

# Tracing
ls /sys/kernel/debug/tracing/
```

---

# 56. Final Testing Checklist

```text
BOOT
[ ] Board boots
[ ] Serial console works
[ ] Correct kernel running
[ ] Correct Device Tree loaded

DRIVER
[ ] Driver builds
[ ] Module loads
[ ] Device matches
[ ] probe() succeeds
[ ] remove() succeeds

GPIO
[ ] Input
[ ] Output
[ ] Interrupt
[ ] Physical verification

UART
[ ] TX
[ ] RX
[ ] Loopback
[ ] Baud rate
[ ] Stress

I2C
[ ] Bus detection
[ ] Device detection
[ ] Read
[ ] Write
[ ] Stress

SPI
[ ] Controller
[ ] CS
[ ] TX
[ ] RX
[ ] Loopback
[ ] Logic analyzer

PWM
[ ] Enable
[ ] Disable
[ ] Frequency
[ ] Duty cycle
[ ] Oscilloscope

ADC
[ ] IIO device
[ ] Channel
[ ] Known voltage
[ ] Repeated samples

CAN
[ ] Interface
[ ] Bitrate
[ ] TX
[ ] RX
[ ] Stress

SYSTEM
[ ] Interrupts
[ ] Error handling
[ ] Memory
[ ] Concurrency
[ ] Power management
[ ] Reboot
[ ] Stress
[ ] Regression

DOCUMENTATION
[ ] Test results recorded
[ ] Logs saved
[ ] Failures documented
[ ] Fix verified
```

---

# 57. Summary

The testing strategy for this BeagleBone Black device-driver project is:

```text
                  Build Test
                      |
                      v
                  Boot Test
                      |
                      v
                Device Tree Test
                      |
                      v
                  Probe Test
                      |
        +-------------+-------------+
        |             |             |
       GPIO          UART          I2C
        |             |             |
        +-------------+-------------+
                      |
                     SPI
                      |
                     PWM
                      |
                     ADC
                      |
                     CAN
                      |
                      v
               Interrupt Test
                      |
                      v
              Error Path Testing
                      |
                      v
              Memory/Concurrency
                      |
                      v
                Stress Testing
                      |
                      v
               Reboot Testing
                      |
                      v
             Regression Testing
                      |
                      v
               FINAL REPORT
```

The final objective is not simply:

```text
"Driver works"
```

It is:

```text
"Driver works correctly,
handles errors,
cleans up resources,
survives stress,
communicates correctly with hardware,
and does not break other peripherals."
```

This testing methodology can be applied consistently to all device
drivers in the project:

```text
GPIO → UART → I2C → SPI → PWM → ADC → CAN
```

