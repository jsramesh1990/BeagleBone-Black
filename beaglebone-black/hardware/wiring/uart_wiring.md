# `uart_wiring.md`

````markdown
# BeagleBone Black UART Wiring

## 1. Overview

This document describes the hardware wiring required to test UART
(Universal Asynchronous Receiver/Transmitter) communication on the
BeagleBone Black.

UART is commonly used for:

- Debug console
- GPS/GNSS modules
- Bluetooth modules
- GSM/LTE modems
- RS-232 interfaces
- RS-485 interfaces
- Microcontroller communication
- Industrial devices
- Bootloader console
- Embedded Linux debugging

Basic UART flow:

```text
User Application
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
TX / RX
       |
       v
External UART Device
````

---

# 2. UART Signals

A basic UART connection uses:

```text
TX → Transmit
RX → Receive
GND → Ground
```

Typical connection:

```text
BeagleBone Black              UART Device

TX  ------------------------> RX

RX  <------------------------ TX

GND ------------------------> GND
```

The important rule is:

```text
TX → RX
RX → TX
GND → GND
```

---

# 3. Required Components

For a basic UART test:

```text
1 × BeagleBone Black
1 × USB-to-TTL UART adapter
Jumper wires
```

Optional:

```text
1 × External UART peripheral
Logic analyzer
Oscilloscope
```

A USB-to-TTL adapter is useful when testing the UART interface from a
PC.

---

# 4. UART Pin Selection

Before wiring the UART device, check:

```text
hardware/pinout/uart_pin_map.md
```

Use this file to identify:

```text
UART controller
TX pin
RX pin
Header location
Pinmux mode
```

Do not assume that every header pin is configured for UART.

The AM335x pins are multiplexed between multiple peripheral functions.

---

# 5. Basic UART Wiring

Typical UART connection:

```text
BeagleBone Black          UART Device
----------------          -----------

TX  --------------------> RX

RX  <-------------------- TX

GND --------------------> GND
```

Complete:

```text
              BeagleBone Black
                    |
             +------+------+
             |      |      |
            TX     RX     GND
             |      |      |
             v      v      v
        +----------------------+
        |     UART DEVICE      |
        |                      |
        | RX    TX     GND     |
        +----------------------+
```

---

# 6. UART Wiring Table

| BeagleBone Black | UART Device |
| ---------------- | ----------- |
| TX               | RX          |
| RX               | TX          |
| GND              | GND         |

The exact physical header pins must be taken from:

```text
hardware/pinout/uart_pin_map.md
```

---

# 7. UART Direction

UART communication is crossed:

```text
BeagleBone TX
      |
      v
Device RX


BeagleBone RX
      ^
      |
Device TX
```

Incorrect:

```text
TX → TX
RX → RX
```

Correct:

```text
TX → RX
RX → TX
```

---

# 8. Common UART Parameters

UART communication requires both devices to use compatible settings.

Common parameters:

```text
Baud Rate
Data Bits
Parity
Stop Bits
Flow Control
```

Typical configuration:

```text
115200 baud
8 data bits
No parity
1 stop bit
No hardware flow control
```

Known as:

```text
115200 8N1
```

---

# 9. UART 8N1

`8N1` means:

```text
8 → Data bits
N → No parity
1 → One stop bit
```

Frame concept:

```text
Idle
  |
  v
Start
  |
  v
8 Data Bits
  |
  v
Stop
  |
  v
Idle
```

Conceptually:

```text
       Start       Data Bits              Stop
         |       |              |           |
         v       v              v           v

Idle ___|‾|____|_|_|_|_|_|_|_|_|‾|__________
          1     8 data bits       1
```

---

# 10. UART Baud Rate

Baud rate represents the signaling rate.

Common values:

```text
9600
19200
38400
57600
115200
230400
460800
921600
```

For initial debugging:

```text
115200 8N1
```

is commonly used.

The actual baud rate must match the connected device.

---

# 11. UART Device Tree

Project Device Tree files:

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
AM335x UART Controller
      |
      v
UART Driver
      |
      v
Linux TTY Framework
      |
      v
/dev/tty*
      |
      v
User Application
```

---

# 12. UART Pinmux

The selected physical pins must be configured for UART.

Conceptually:

```text
Physical Pin
     |
     +---- GPIO
     |
     +---- UART
     |
     +---- I2C
     |
     +---- SPI
     |
     +---- PWM
```

The pin must be configured for the required UART TX/RX function.

---

# 13. UART Linux Device

After booting Linux:

```bash
ls /dev/tty*
```

Possible UART devices include:

```text
/dev/ttyS0
/dev/ttyS1
/dev/ttyS2
```

The exact device name depends on the kernel configuration and board
device-tree configuration.

Do not assume a particular UART number without checking the target
system.

---

# 14. Check UART Kernel Logs

Run:

```bash
dmesg | grep -i uart
```

Also useful:

```bash
dmesg | grep -i tty
```

This can show:

```text
UART controller
TTY device
Console configuration
Driver initialization
```

---

# 15. Check Serial Devices

Use:

```bash
ls -l /dev/ttyS*
```

You can also check:

```bash
ls -l /dev/serial/by-path/
```

and:

```bash
ls -l /dev/serial/by-id/
```

The `/dev/serial` paths are especially useful when USB serial adapters
are connected.

---

# 16. Configure UART With stty

Example:

```bash
sudo stty -F /dev/ttyS1 115200 cs8 -cstopb -parenb
```

This configures:

```text
Baud rate = 115200
Data bits = 8
Parity    = None
Stop bits = 1
```

Check the current configuration:

```bash
stty -F /dev/ttyS1 -a
```

Replace `/dev/ttyS1` with the UART device actually used on your board.

---

# 17. UART Terminal Test

Useful terminal programs include:

```text
minicom
picocom
screen
```

Example with `minicom`:

```bash
sudo minicom -D /dev/ttyS1 -b 115200
```

Example with `picocom`:

```bash
picocom -b 115200 /dev/ttyS1
```

Example with `screen`:

```bash
screen /dev/ttyS1 115200
```

---

# 18. PC USB-to-UART Connection

A common test setup is:

```text
PC
 |
 | USB
 v
USB-to-TTL UART
 |
 +------ TX
 |
 +------ RX
 |
 +------ GND
 |
 v
BeagleBone Black
```

Wiring:

```text
USB-UART TX  --------> BBB RX

USB-UART RX  <-------- BBB TX

USB-UART GND --------- BBB GND
```

---

# 19. UART Voltage Levels

Be careful when selecting a USB-to-UART adapter.

The BeagleBone Black UART interface uses appropriate low-voltage
logic levels. Use a UART adapter compatible with the BeagleBone's I/O
voltage.

Do not connect a traditional RS-232 voltage-level signal directly to a
TTL UART pin.

Incorrect:

```text
RS-232 TX --------> BBB RX
```

Correct architecture:

```text
RS-232 Device
      |
      v
RS-232 Transceiver
      |
      v
TTL/Logic UART
      |
      v
BeagleBone UART
```

---

# 20. TTL UART vs RS-232

These are different electrical interfaces.

### TTL/Logic UART

```text
TX
RX
GND
```

Uses logic-level voltages.

### RS-232

Uses different voltage levels and polarity.

Architecture:

```text
UART Controller
      |
      v
RS-232 Transceiver
      |
      v
RS-232 Connector
```

Never connect RS-232 electrical levels directly to the BeagleBone UART
pins.

---

# 21. UART-to-RS-232 Wiring

For an RS-232 application:

```text
BeagleBone UART
       |
       v
RS-232 Transceiver
       |
       v
DB9 / RS-232 Device
```

Example:

```text
BBB TX
  |
  v
Transceiver
  |
  v
RS-232 TX

BBB RX
  ^
  |
Transceiver
  ^
  |
RS-232 RX
```

The transceiver performs the electrical-level conversion.

---

# 22. UART-to-RS-485

RS-485 is also not directly equivalent to TTL UART.

Typical architecture:

```text
BeagleBone UART
      |
      v
RS-485 Transceiver
      |
      v
A / B Differential Bus
      |
      v
RS-485 Device
```

Typical signals:

```text
UART TX
UART RX
DE
RE
```

The transceiver converts the UART logic signals into the RS-485
differential bus.

---

# 23. UART Loopback Test

A simple hardware test connects:

```text
TX → RX
```

Example:

```text
BeagleBone UART

TX ----------------+
                   |
                   +------ RX
```

Also ensure:

```text
GND
```

is connected appropriately.

---

# 24. UART Loopback Flow

```text
Application
    |
    v
UART Driver
    |
    v
TX
    |
    v
Loopback Wire
    |
    v
RX
    |
    v
UART Driver
    |
    v
Application
```

Example:

```text
TX = "Hello"
RX = "Hello"
```

If the received data matches the transmitted data, the basic UART
transmit/receive path is working.

---

# 25. UART Loopback Wiring

| UART Signal | Connection |
| ----------- | ---------- |
| TX          | RX         |
| RX          | TX         |
| GND         | GND        |

For a physical loopback:

```text
TX --------+
           |
           +-------- RX
```

---

# 26. UART Loopback Test With Terminal

Configure the UART:

```bash
sudo stty -F /dev/ttyS1 115200 cs8 -cstopb -parenb
```

Connect:

```text
TX → RX
```

Then write:

```bash
echo "Hello UART" > /dev/ttyS1
```

For more controlled binary/text testing, use:

```bash
printf "Hello UART\r\n" > /dev/ttyS1
```

Read using another terminal/process:

```bash
cat /dev/ttyS1
```

Exact behavior depends on terminal settings and echo configuration.

---

# 27. UART External Device Test

Example GPS module:

```text
BeagleBone              GPS

TX -------------------> RX

RX <------------------- TX

GND ------------------> GND
```

The GPS may continuously transmit:

```text
NMEA sentences
```

Example:

```text
$GPGGA,...
$GPRMC,...
```

The actual sentence types depend on the GNSS module configuration.

---

# 28. UART GPS Flow

```text
GPS Module
    |
    | UART TX
    v
BeagleBone RX
    |
    v
UART Driver
    |
    v
TTY
    |
    v
GPS Application
```

---

# 29. UART Bluetooth Module

Typical wiring:

```text
BeagleBone TX ------------> Bluetooth RX

BeagleBone RX <------------ Bluetooth TX

BeagleBone GND ------------ Bluetooth GND

Appropriate VCC -----------> Bluetooth VCC
```

The module's voltage requirements must be checked before connecting it.

---

# 30. UART Modem

Typical architecture:

```text
BeagleBone
    |
    | UART
    v
Cellular Modem
    |
    v
LTE / 5G Network
```

UART signals:

```text
TX
RX
GND
```

Some modems also use:

```text
RTS
CTS
DTR
RI
```

depending on the modem and application.

---

# 31. Hardware Flow Control

Hardware flow control uses:

```text
RTS → Request To Send
CTS → Clear To Send
```

Typical connection:

```text
BBB RTS ----------------> Device CTS

BBB CTS <---------------- Device RTS
```

Full connection:

```text
TX
RX
RTS
CTS
GND
```

Do not enable hardware flow control unless the wiring and peripheral
support it.

---

# 32. UART With RTS/CTS

Conceptual flow:

```text
             UART Device
                  ^
                  |
                CTS
                  |
BBB RTS ----------+

             UART Device
                  |
                  v
                RTS
                  |
BBB CTS <---------+
```

The purpose is to prevent buffer overflow during high-rate transfers.

---

# 33. UART Interrupt Flow

UART data is commonly handled using interrupts.

Conceptual flow:

```text
UART Hardware
      |
      | RX interrupt
      v
Interrupt Controller
      |
      v
UART Driver
      |
      v
TTY Layer
      |
      v
User Application
```

For transmission:

```text
Application
    |
    v
TTY
    |
    v
UART Driver
    |
    v
UART TX FIFO
    |
    v
TX Pin
```

---

# 34. UART FIFO

The UART controller may contain FIFO buffers.

Conceptually:

```text
RX Pin
  |
  v
RX FIFO
  |
  v
UART Driver
  |
  v
TTY Buffer
```

For TX:

```text
Application
  |
  v
TTY Buffer
  |
  v
UART Driver
  |
  v
TX FIFO
  |
  v
TX Pin
```

FIFO reduces CPU overhead and helps handle bursts of serial data.

---

# 35. UART Device Tree Overlay

Project overlay:

```text
beaglebone-black/
└── device-tree/
    └── overlays/
        └── bbb-uart-overlay.dts
```

Conceptual flow:

```text
UART Overlay
     |
     v
Pinmux
     |
     v
UART Controller
     |
     v
Linux UART Driver
     |
     v
/dev/tty*
```

---

# 36. UART Debugging Flow

```text
                    UART Failure
                         |
                         v
                  /dev/tty* exists?
                    /          \
                  NO            YES
                  |              |
                  v              v
          Check Device Tree   Check settings
          Check pinmux       Baud rate
          Check kernel       Data bits
                              Parity
                              Stop bits
                                |
                                v
                          Data received?
                           /          \
                         NO            YES
                         |              |
                         v              v
                   Check TX/RX      UART OK
                   Check GND
                   Check voltage
```

---

# 37. UART Device Not Available

Check:

```text
[ ] UART controller enabled
[ ] Device Tree
[ ] Pinmux
[ ] Kernel UART driver
[ ] Console configuration
[ ] Correct UART device
```

Run:

```bash
dmesg | grep -i uart
```

and:

```bash
dmesg | grep -i tty
```

---

# 38. UART No Data

If the UART device exists but no data is received:

```text
Check:

[ ] TX → RX
[ ] RX → TX
[ ] GND
[ ] Baud rate
[ ] Data bits
[ ] Parity
[ ] Stop bits
[ ] Flow control
[ ] Voltage level
```

---

# 39. Garbage Characters

Example:

```text
������������
```

Possible causes:

```text
[ ] Wrong baud rate
[ ] Wrong data bits
[ ] Wrong parity
[ ] Wrong stop bits
[ ] Clock configuration
[ ] Electrical noise
[ ] Incorrect voltage levels
```

Start with:

```text
115200 8N1
```

and configure both ends identically.

---

# 40. TX Works but RX Does Not

Check:

```text
[ ] External TX → BBB RX
[ ] Correct RX pin
[ ] Pinmux
[ ] Device TX enabled
[ ] Common GND
[ ] Voltage compatibility
```

Use a logic analyzer to determine whether the external device is
actually transmitting.

---

# 41. RX Works but TX Does Not

Check:

```text
[ ] BBB TX pin
[ ] Pinmux
[ ] UART driver
[ ] External RX
[ ] Correct voltage
[ ] Application write operation
```

Monitor TX using:

```text
Logic Analyzer
```

or:

```text
Oscilloscope
```

---

# 42. UART Logic Analyzer

Connect:

```text
Analyzer CH0 → TX
Analyzer CH1 → RX
Analyzer GND → GND
```

Configure the decoder:

```text
Protocol = UART
Baud Rate = 115200
Data Bits = 8
Parity = None
Stop Bits = 1
```

Then transmit:

```text
Hello UART
```

The analyzer should decode the characters.

---

# 43. UART Waveform

Conceptually:

```text
Idle      Start       Data Bits        Stop

HIGH       LOW       D0 D1 D2 D3...       HIGH
  |         |          |            |        |
  |_________|‾|_|‾|_|‾|_|‾|_|‾|_|‾|_|‾|____|
```

The exact waveform depends on the transmitted byte.

---

# 44. UART Test Procedure

## Step 1 — Select UART

Check:

```text
hardware/pinout/uart_pin_map.md
```

## Step 2 — Verify Pinmux

Configure:

```text
TX
RX
```

## Step 3 — Connect UART

```text
TX → RX
RX → TX
GND → GND
```

## Step 4 — Boot Linux

## Step 5 — Check Device

```bash
ls -l /dev/ttyS*
```

## Step 6 — Check Logs

```bash
dmesg | grep -i uart
```

## Step 7 — Configure UART

```bash
sudo stty -F /dev/ttyS1 115200 cs8 -cstopb -parenb
```

## Step 8 — Perform Loopback

```text
TX → RX
```

## Step 9 — Test External Device

## Step 10 — Verify With Logic Analyzer

---

# 45. UART Test With USB-to-TTL Adapter

Connection:

```text
                 USB-UART Adapter
                        |
                 +------+------+ 
                 |      |      |
                TX     RX     GND
                 |      |      |
                 v      v      v
              +-------------------+
              | BeagleBone Black  |
              |                   |
              | RX      TX    GND  |
              +-------------------+
```

Cross:

```text
Adapter TX → BBB RX
Adapter RX → BBB TX
Adapter GND → BBB GND
```

---

# 46. UART Terminal Configuration

Example:

```bash
picocom -b 115200 /dev/ttyUSB0
```

For a board UART:

```bash
picocom -b 115200 /dev/ttyS1
```

The correct device path depends on the connected hardware.

---

# 47. UART Console

One UART may be configured as the Linux console.

Conceptual boot flow:

```text
U-Boot
   |
   v
Linux Kernel
   |
   v
Console UART
   |
   v
Login / Kernel Messages
```

If a UART is being used as the system console, do not use it for an
independent application without first understanding and changing the
console configuration.

---

# 48. UART Console Check

Check kernel boot parameters:

```bash
cat /proc/cmdline
```

Look for console configuration such as:

```text
console=ttySx,...
```

The exact UART name and baud rate depend on the board configuration.

---

# 49. UART Application Flow

A typical application:

```text
User Application
      |
      v
open()
      |
      v
/dev/ttySx
      |
      v
termios configuration
      |
      v
read() / write()
      |
      v
Linux TTY
      |
      v
UART Driver
      |
      v
UART Hardware
```

---

# 50. UART Driver Flow

Kernel-level architecture:

```text
                 Device Tree
                      |
                      v
                UART Controller
                      |
                      v
                  UART Driver
                      |
                      v
                  Serial Core
                      |
                      v
                  TTY Layer
                      |
                      v
                /dev/ttySx
                      |
                      v
                User Program
```

---

# 51. UART Driver Responsibilities

A UART driver generally handles:

```text
Hardware initialization
Baud rate configuration
TX operation
RX operation
FIFO configuration
Interrupt handling
Flow control
Power management
TTY integration
Error handling
```

---

# 52. UART Errors

UART hardware can report errors such as:

```text
Overrun
Framing error
Parity error
Break condition
```

Conceptually:

```text
UART Hardware
      |
      v
Error Status
      |
      v
UART Driver
      |
      v
TTY / Application
```

---

# 53. UART Overrun

An overrun can occur when new RX data arrives before previously received
data has been processed.

Possible causes:

```text
[ ] Application too slow
[ ] High baud rate
[ ] Small buffers
[ ] CPU load
[ ] Interrupt latency
```

Solutions may include:

```text
FIFO tuning
Larger buffers
Flow control
Lower baud rate
Improved application processing
```

---

# 54. UART Wiring Checklist

```text
[ ] UART controller selected
[ ] TX pin verified
[ ] RX pin verified
[ ] Header pins verified
[ ] Pinmux configured
[ ] TX connected to RX
[ ] RX connected to TX
[ ] GND connected
[ ] Voltage level verified
[ ] Baud rate verified
[ ] Data bits verified
[ ] Parity verified
[ ] Stop bits verified
[ ] Flow control verified
[ ] /dev/tty* checked
[ ] Loopback test completed
[ ] External device tested
[ ] Logic analyzer test completed
```

---

# 55. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── uart_pin_map.md
│   │
│   └── wiring/
│       └── uart_wiring.md
│
├── hardware/
│   └── schematics/
│       └── uart/
│           └── uart_test_circuit.md
│
├── device-tree/
│   ├── uart/
│   │   ├── bbb-uart.dts
│   │   ├── bbb-uart.dtsi
│   │   └── README.md
│   │
│   └── overlays/
│       └── bbb-uart-overlay.dts
│
├── drivers/
│   └── uart/
│       └── README.md
│
└── tests/
    └── uart/
        ├── uart_loopback_test.sh
        ├── uart_terminal_test.sh
        └── README.md
```

---

# 56. Complete UART Hardware Flow

```text
                         User Application
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
                       AM335x UART Hardware
                                |
                +---------------+---------------+
                |                               |
                v                               v
               TX                              RX
                |                               |
                v                               ^
         External Device ------------------------+
```

---

# 57. Complete UART Bring-Up

```text
UART Pin Mapping
       |
       v
Device Tree
       |
       v
Pinmux
       |
       v
UART Controller
       |
       v
UART Driver
       |
       v
TTY Framework
       |
       v
/dev/ttySx
       |
       v
stty / picocom / Application
       |
       v
External UART Device
```

---

# 58. Final Test Objective

The objective of this wiring test is to validate the complete UART path:

```text
UART Configuration
      ↓
Device Tree / Pinmux
      ↓
AM335x UART Controller
      ↓
Linux UART Driver
      ↓
Linux TTY Framework
      ↓
/dev/ttySx
      ↓
TX / RX
      ↓
External UART Device
```

Recommended validation sequence:

```text
1. Verify UART pin mapping
2. Verify Device Tree
3. Verify pinmux
4. Check UART controller
5. Check /dev/ttyS*
6. Configure 115200 8N1
7. Perform TX/RX loopback
8. Connect USB-to-TTL adapter
9. Test terminal communication
10. Test external UART peripheral
11. Verify baud rate
12. Verify TX/RX with logic analyzer
13. Test driver/application communication
```

> **Important:** BeagleBone Black UART pins are logic-level interfaces.
> Do not connect RS-232 or other higher-voltage electrical interfaces
> directly to the UART pins. Use an appropriate level-shifting or
> transceiver circuit. Always verify the voltage levels and pin
> assignments before powering the circuit.

````

**File location:**

```text
beaglebone-black/hardware/wiring/uart_wiring.md
````

