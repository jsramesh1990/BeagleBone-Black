# `uart_test_circuit.md`

````markdown
# BeagleBone Black UART Test Circuit

## 1. Overview

This document describes the hardware test circuit for validating UART
communication on the BeagleBone Black.

UART (Universal Asynchronous Receiver/Transmitter) is commonly used for:

- Debug console
- GPS/GNSS modules
- Bluetooth modules
- GSM/LTE modems
- RS-232/RS-485 transceivers
- Microcontrollers
- Embedded Linux communication
- Bootloader and kernel debugging

The complete path is:

```text
User Application
       |
       v
   /dev/ttyO*
       |
       v
Linux TTY / Serial Framework
       |
       v
UART Driver
       |
       v
AM335x UART Controller
       |
       v
UART Pins
       |
       v
External UART Device
````

---

# 2. UART Test Objectives

This test validates:

* UART TX
* UART RX
* GND
* Baud rate
* Data bits
* Stop bits
* Parity
* UART pinmux
* Device Tree configuration
* Linux UART driver
* `/dev/tty*` device node
* UART loopback
* Serial terminal communication

---

# 3. Required Components

For a basic UART test:

```text
1 × BeagleBone Black
1 × USB-to-TTL UART adapter
Jumper wires
Computer
```

For hardware loopback:

```text
1 × BeagleBone Black
Jumper wire
```

For an actual UART peripheral:

```text
1 × BeagleBone Black
1 × UART sensor/module
Jumper wires
External power supply if required
```

---

# 4. UART Signals

A basic UART connection requires:

```text
TX = Transmit
RX = Receive
GND = Ground
```

Basic connection:

```text
BeagleBone Black              UART Device

       TX  --------------------> RX
       RX  <-------------------- TX
      GND  --------------------- GND
```

TX of one device must connect to RX of the other device.

---

# 5. Complete UART Test Circuit

```text
             BeagleBone Black
             UART Controller
                    |
          +---------+---------+
          |                   |
         TX                  RX
          |                   |
          |                   |
          v                   v
       +--------------------------+
       |      UART Device         |
       |                          |
       | RX                  TX   |
       +--------------------------+
                    |
                   GND
                    |
                    |
             BeagleBone GND
```

---

# 6. UART Wiring

Correct UART wiring:

```text
BBB UART       UART Device
---------------------------
TX       ----> RX
RX       <---- TX
GND      ----- GND
```

Do not connect:

```text
BBB TX ---- TX Device
BBB RX ---- RX Device
```

Correct:

```text
TX → RX
RX → TX
```

---

# 7. UART Voltage Levels

BeagleBone Black UART GPIO signals are typically 3.3 V logic.

For a 3.3 V UART device:

```text
BBB TX  ---> Device RX
BBB RX  <--- Device TX
GND     ---> Device GND
```

Do not directly connect RS-232 voltage levels to the BeagleBone UART
pins.

RS-232 requires a suitable level translator/transceiver.

For example:

```text
BeagleBone UART
       |
       v
UART-to-RS232 Transceiver
       |
       v
RS-232 Device
```

---

# 8. UART-to-USB Test Circuit

A USB-to-TTL UART adapter is convenient for testing.

```text
             BeagleBone Black
                    |
                    |
             +------+------+
             |             |
            TX            RX
             |             |
             v             v
       +-----------------------+
       | USB-to-TTL UART       |
       |                       |
       | RX                TX  |
       +-----------------------+
                |
                v
             USB Cable
                |
                v
             PC / Laptop
```

Connections:

```text
BBB TX  → Adapter RX
BBB RX  → Adapter TX
BBB GND → Adapter GND
```

---

# 9. UART Loopback Test

The simplest hardware test connects TX directly to RX.

```text
BeagleBone Black

       TX
        |
        |
        +----------------+
                         |
                         v
                        RX
```

This creates a UART loopback.

Data transmitted by the UART is received back by the same UART.

---

# 10. UART Loopback Circuit

```text
             +----------------------+
             |  BeagleBone Black    |
             |                      |
             |       UART           |
             |                      |
             | TX ------------+     |
             |               |      |
             |               +---- RX|
             |                      |
             | GND                  |
             +----------------------+
```

No external UART device is required.

---

# 11. UART Communication Parameters

Typical UART configuration:

```text
Baud Rate : 115200
Data Bits : 8
Parity    : None
Stop Bits : 1
Flow Ctrl : None
```

This is commonly written as:

```text
115200 8N1
```

Meaning:

```text
115200 → Baud rate
8      → Data bits
N      → No parity
1      → Stop bit
```

The exact configuration must match the connected device.

---

# 12. UART Frame

A typical UART frame contains:

```text
Start Bit
    |
    v
Data Bits
    |
    v
Optional Parity
    |
    v
Stop Bit
```

Conceptually:

```text
Idle   Start       Data            Stop
HIGH    LOW     D0 D1 D2 ... D7     HIGH
 |       |          |                |
-+-------+----------+----------------+-----
```

UART is asynchronous, so there is no separate clock signal.

---

# 13. UART Architecture

```text
                    User Application
                           |
                           v
                    Serial Device
                    /dev/ttyO*
                           |
                           v
                    Linux TTY Layer
                           |
                           v
                    UART Driver
                           |
                           v
                   AM335x UART HW
                           |
                           v
                       Pinmux
                           |
                           v
                    UART TX / RX
                           |
                           v
                     UART Device
```

---

# 14. Device Tree

UART controllers and pinmux are configured through Device Tree.

Project files:

```text
beaglebone-black/
└── device-tree/
    └── uart/
        ├── bbb-uart.dts
        ├── bbb-uart.dtsi
        └── README.md
```

Conceptual flow:

```text
bbb-uart.dts
      |
      v
Pinmux Configuration
      |
      v
UART Controller
      |
      v
Linux UART Driver
      |
      v
TTY Framework
      |
      v
/dev/tty*
```

---

# 15. UART Pin Selection

Select the required UART pins from:

```text
hardware/pinout/uart_pin_map.md
```

Before wiring:

```text
1. Select UART instance
2. Verify TX pin
3. Verify RX pin
4. Verify GND
5. Check pinmux
6. Check peripheral conflicts
7. Verify voltage levels
```

---

# 16. Check UART Device Nodes

After booting Linux:

```bash
ls /dev/tty*
```

Common serial device names may include:

```text
/dev/ttyO0
/dev/ttyO1
/dev/ttyO2
```

or, depending on kernel/device-tree configuration:

```text
/dev/ttyS0
/dev/ttyS1
```

The exact naming depends on the Linux kernel and serial driver
configuration.

---

# 17. Find UART Devices

Run:

```bash
dmesg | grep -i tty
```

Example:

```text
serial8250: ttyS1 at MMIO ...
```

Also:

```bash
ls -l /dev/ttyO*
```

or:

```bash
ls -l /dev/ttyS*
```

---

# 18. Configure UART Using `stty`

Example:

```bash
stty -F /dev/ttyO1 115200 cs8 -cstopb -parenb
```

This configures:

```text
Baud rate = 115200
Data bits = 8
Stop bits = 1
Parity    = None
```

Disable hardware flow control where required:

```bash
stty -F /dev/ttyO1 -crtscts
```

---

# 19. Check UART Configuration

Run:

```bash
stty -F /dev/ttyO1 -a
```

Example configuration:

```text
speed 115200 baud;
cs8;
-clocal;
-crtscts;
-parenb;
-cstopb;
```

The exact output depends on the kernel and terminal configuration.

---

# 20. UART Transmit Test

Send a string:

```bash
echo "Hello UART" > /dev/ttyO1
```

For a device connected to the UART, it should receive:

```text
Hello UART
```

---

# 21. UART Receive Test

Read from the UART:

```bash
cat /dev/ttyO1
```

If another device transmits:

```text
Hello BeagleBone
```

the terminal should display:

```text
Hello BeagleBone
```

Press:

```text
Ctrl+C
```

to stop `cat`.

---

# 22. UART Loopback Test

Connect:

```text
TX ↔ RX
```

Configure:

```bash
stty -F /dev/ttyO1 115200 cs8 -cstopb -parenb -crtscts
```

Start receiver:

```bash
cat /dev/ttyO1
```

In another terminal:

```bash
echo "UART LOOPBACK TEST" > /dev/ttyO1
```

Expected:

```text
UART LOOPBACK TEST
```

This verifies the transmit and receive path.

---

# 23. Better Loopback Test

For binary-safe testing, use a small Python test program:

```python
import serial

port = "/dev/ttyO1"

ser = serial.Serial(
    port=port,
    baudrate=115200,
    bytesize=8,
    parity=serial.PARITY_NONE,
    stopbits=1,
    timeout=1
)

tx = b"UART LOOPBACK TEST\r\n"

ser.write(tx)

rx = ser.read(len(tx))

print("TX:", tx)
print("RX:", rx)

if rx == tx:
    print("UART LOOPBACK PASS")
else:
    print("UART LOOPBACK FAIL")

ser.close()
```

Install PySerial if required:

```bash
python3 -m pip install pyserial
```

---

# 24. USB-to-TTL PC Test

Connect:

```text
BBB TX  → USB-UART RX
BBB RX  ← USB-UART TX
BBB GND → USB-UART GND
```

On the PC, identify the adapter:

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

---

# 25. PC Serial Terminal

Useful Linux terminal applications include:

```text
minicom
picocom
screen
```

Example:

```bash
sudo minicom -D /dev/ttyUSB0 -b 115200
```

Or:

```bash
picocom -b 115200 /dev/ttyUSB0
```

The settings must match the BeagleBone UART:

```text
115200 8N1
```

---

# 26. UART Logic Analyzer Test

Connect a logic analyzer to:

```text
TX
RX
GND
```

Example:

```text
BeagleBone Black

TX  -----------------> Logic Analyzer CH1
RX  -----------------> Logic Analyzer CH2
GND -----------------> Logic Analyzer GND
```

Configure the analyzer:

```text
Protocol : UART / Async Serial
Baud     : 115200
Data     : 8 bits
Parity   : None
Stop     : 1
```

---

# 27. Expected UART Waveform

UART idle state is normally HIGH.

Conceptually:

```text
Idle
HIGH ─────────┐
              |
Start         v
LOW           └───┐
                  |
Data bits         |
                  └───────────────
                                     
Stop              ┌───────────────
HIGH              |
──────────────────┘
```

The logic analyzer should decode the transmitted characters.

---

# 28. UART Baud Rate Test

For:

```text
115200 baud
```

the approximate bit time is:

```text
1 / 115200 ≈ 8.68 µs
```

Using a logic analyzer, verify that the UART bit timing is
approximately correct.

---

# 29. UART Driver Flow

```text
User Application
       |
       v
/dev/ttyO*
       |
       v
Linux TTY Layer
       |
       v
Serial Core
       |
       v
UART Driver
       |
       v
AM335x UART Controller
       |
       v
UART TX/RX Pins
       |
       v
External UART Device
```

---

# 30. UART Device Tree Concept

Conceptually:

```text
UART Controller
    |
    +-- pinctrl
    |
    +-- status = "okay"
    |
    +-- UART Configuration
```

Pinmux:

```text
UART TX → TX function
UART RX → RX function
```

The exact Device Tree properties depend on the kernel and board
Device Tree implementation.

---

# 31. UART Debugging

If the UART device does not appear:

```text
UART Not Available
       |
       +--> Check Device Tree
       |
       +--> Check UART controller
       |
       +--> Check pinmux
       |
       +--> Check kernel configuration
       |
       +--> Check serial driver
       |
       +--> Check /dev/tty*
```

---

# 32. Check Kernel Logs

Run:

```bash
dmesg | grep -i serial
```

Also:

```bash
dmesg | grep -i tty
```

And:

```bash
dmesg | grep -i uart
```

---

# 33. Check UART Pinmux

If the device exists but there is no physical output:

```text
[ ] Correct UART selected
[ ] TX pin configured for UART TX
[ ] RX pin configured for UART RX
[ ] Pinmux loaded
[ ] No conflicting peripheral
[ ] Correct header pin
```

---

# 34. UART Wiring Errors

### Error 1 — TX connected to TX

Incorrect:

```text
BBB TX -------- TX Device
```

Correct:

```text
BBB TX -------- RX Device
```

### Error 2 — RX connected to RX

Incorrect:

```text
BBB RX -------- RX Device
```

Correct:

```text
BBB RX -------- TX Device
```

### Error 3 — Missing ground

Always provide a common signal reference:

```text
BBB GND -------- Device GND
```

---

# 35. UART Parameter Mismatch

If garbage characters are displayed:

```text
UART Parameters
      |
      +--> Check baud rate
      |
      +--> Check data bits
      |
      +--> Check parity
      |
      +--> Check stop bits
      |
      +--> Check flow control
```

For example, both sides should use:

```text
115200 8N1
```

---

# 36. Hardware Flow Control

Some UART devices use:

```text
RTS
CTS
```

Architecture:

```text
BBB RTS  ------------> Device CTS
BBB CTS  <------------ Device RTS
```

For basic UART testing:

```text
Hardware Flow Control = Disabled
```

Unless the peripheral explicitly requires RTS/CTS.

---

# 37. UART Test With Actual Sensor

Example architecture:

```text
                 BeagleBone Black
                        |
                        |
                   UART TX/RX
                        |
                        v
                +---------------+
                | UART Sensor   |
                +---------------+
                        |
                        v
                   Sensor Data
                        |
                        v
                 Linux Application
```

Typical flow:

```text
Application
    |
    v
open("/dev/tty...")
    |
    v
configure UART
    |
    v
write command
    |
    v
read response
    |
    v
parse sensor data
```

---

# 38. UART C Test Application

Example:

```c
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int main(void)
{
    int fd;
    struct termios tty;

    fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    memset(&tty, 0, sizeof(tty));

    tcgetattr(fd, &tty);

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;

    tcsetattr(fd, TCSANOW, &tty);

    const char *msg = "UART TEST\r\n";

    write(fd, msg, strlen(msg));

    close(fd);

    return 0;
}
```

Compile:

```bash
gcc uart_test.c -o uart_test
```

Run:

```bash
sudo ./uart_test
```

The actual UART device node must match the board configuration.

---

# 39. UART Test Script

```bash
#!/bin/bash

UART=/dev/ttyO1

stty -F ${UART} 115200 cs8 -cstopb -parenb -crtscts

echo "UART configuration:"
stty -F ${UART} -a

echo "Sending UART test message..."

echo "HELLO FROM BEAGLEBONE BLACK" > ${UART}
```

---

# 40. UART Test Procedure

## Step 1

Power off the board before changing wiring.

## Step 2

Connect:

```text
TX → RX
RX → TX
GND → GND
```

## Step 3

Boot Linux.

## Step 4

Find UART:

```bash
dmesg | grep -i tty
```

## Step 5

Check device:

```bash
ls -l /dev/tty*
```

## Step 6

Configure UART:

```bash
stty -F /dev/ttyO1 115200 cs8 -cstopb -parenb -crtscts
```

## Step 7

Start receive:

```bash
cat /dev/ttyO1
```

## Step 8

Transmit:

```bash
echo "UART TEST" > /dev/ttyO1
```

## Step 9

Verify:

```text
UART TEST
```

---

# 41. UART Debug Flow

```text
                   UART Failure
                        |
                        v
                 Device Exists?
                   /        \
                 NO          YES
                 |            |
                 v            v
            Check DT      Check Pinmux
                              |
                              v
                         Check Wiring
                              |
                              v
                        Check Baud Rate
                              |
                              v
                        Check 8N1
                              |
                              v
                       Check Flow Control
                              |
                              v
                       Logic Analyzer
                              |
                              v
                         Verify TX/RX
```

---

# 42. UART Hardware Verification

```text
[ ] UART-capable pins identified
[ ] Correct TX pin
[ ] Correct RX pin
[ ] GND connected
[ ] Voltage level verified
[ ] No RS-232 signal connected directly
[ ] Device Tree configured
[ ] Pinmux configured
[ ] UART controller enabled
[ ] UART driver loaded
[ ] /dev/tty* exists
[ ] Baud rate configured
[ ] 8N1 configured
[ ] Loopback completed
[ ] PC serial test completed
[ ] Logic analyzer verified
```

---

# 43. UART Loopback Checklist

```text
[ ] TX connected to RX
[ ] Common GND
[ ] UART device node available
[ ] 115200 baud
[ ] 8 data bits
[ ] No parity
[ ] 1 stop bit
[ ] Flow control disabled
[ ] Test message transmitted
[ ] Test message received
```

---

# 44. UART Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── uart_pin_map.md
│   │
│   └── schematics/
│       └── uart/
│           └── uart_test_circuit.md
│
├── device-tree/
│   └── uart/
│       ├── bbb-uart.dts
│       ├── bbb-uart.dtsi
│       └── README.md
│
├── drivers/
│   └── uart/
│       └── README.md
│
└── tests/
    └── uart/
        ├── uart_loopback_test.sh
        ├── uart_test.c
        ├── uart_test.py
        └── README.md
```

---

# 45. Complete UART Bring-Up

```text
                     UART Hardware
                          |
                          v
                     UART Pin Map
                          |
                          v
                     Test Circuit
                          |
                          v
                       Pinmux
                          |
                          v
                     Device Tree
                          |
                          v
                      UART Driver
                          |
                          v
                      TTY Layer
                          |
                          v
                      /dev/tty*
                          |
                          v
                   UART Configuration
                          |
                          v
                   TX / RX Transfer
                          |
              +-----------+-----------+
              |                       |
              v                       v
             TX                      RX
              |                       |
              +-----------+-----------+
                          |
                          v
                  External Device
```

---

# 46. Final Objective

The purpose of this test circuit is to validate the complete UART
communication path on the BeagleBone Black:

```text
User Application
       ↓
/dev/tty*
       ↓
Linux TTY Framework
       ↓
UART Driver
       ↓
AM335x UART Controller
       ↓
Pinmux
       ↓
UART TX / RX
       ↓
External UART Device
```

The **loopback test** validates the UART TX/RX path.

The **USB-to-TTL test** validates communication with a PC.

The **logic-analyzer test** validates the physical UART waveform and
baud rate.

The **real-peripheral test** validates the complete Device Tree,
kernel-driver, UART, and application communication path.

> For this BeagleBone Black driver project, validate UART in three
> stages: **loopback → USB-to-TTL/PC terminal → real UART peripheral**.

````

**File location:**

```text
beaglebone-black/hardware/schematics/uart/uart_test_circuit.md
````

