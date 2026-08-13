# BeagleBone Black UART Pin Map

## 1. Overview

The **BeagleBone Black** uses the TI AM335x processor, which provides
multiple UART peripherals.

UART is commonly used for:

* Debug console
* Bootloader console
* Linux kernel console
* GPS/GNSS
* Bluetooth modules
* GSM/LTE modems
* RS-232/RS-485 transceivers
* Microcontroller communication
* Embedded device diagnostics

Basic architecture:

```text
User Application
       |
       v
   /dev/ttySx
       |
       v
   Linux TTY Layer
       |
       v
   UART Driver
       |
       v
   AM335x UART
       |
       v
   TX / RX
       |
       v
 External Device
```

---

# 2. UART Signals

A basic UART connection requires:

| Signal | Meaning  |
| ------ | -------- |
| TX     | Transmit |
| RX     | Receive  |
| GND    | Ground   |

Connection:

```text
BeagleBone Black             UART Device
----------------             -----------

TX ------------------------> RX

RX <------------------------ TX

GND ------------------------ GND
```

For a simple UART connection:

```text
TX ↔ RX
RX ↔ TX
GND ↔ GND
```

---

# 3. BeagleBone Black UART Interfaces

The AM335x contains multiple UART controllers.

Common UART interfaces exposed through the BeagleBone Black headers include:

```text
UART1
UART2
UART4
UART5
```

The UART0 interface is commonly associated with the board's debug/serial
console circuitry rather than a normal expansion-header UART connection.

---

# 4. UART Pin Mapping

## UART1

| Signal    | Header Pin | Function      |
| --------- | ---------: | ------------- |
| UART1_TXD |      P9.24 | UART Transmit |
| UART1_RXD |      P9.26 | UART Receive  |

```text
P9.24 → UART1_TXD
P9.26 → UART1_RXD
```

---

## UART2

| Signal    | Header Pin | Function      |
| --------- | ---------: | ------------- |
| UART2_TXD |      P9.21 | UART Transmit |
| UART2_RXD |      P9.22 | UART Receive  |

```text
P9.21 → UART2_TXD
P9.22 → UART2_RXD
```

> These pins are multiplexed and may be assigned to other peripheral
> functions depending on the active Device Tree.

---

## UART4

| Signal    | Header Pin | Function      |
| --------- | ---------: | ------------- |
| UART4_TXD |      P9.13 | UART Transmit |
| UART4_RXD |      P9.11 | UART Receive  |

```text
P9.13 → UART4_TXD
P9.11 → UART4_RXD
```

---

## UART5

| Signal    | Header Pin | Function      |
| --------- | ---------: | ------------- |
| UART5_TXD |      P8.37 | UART Transmit |
| UART5_RXD |      P8.38 | UART Receive  |

```text
P8.37 → UART5_TXD
P8.38 → UART5_RXD
```

---

# 5. UART Quick Reference

```text
+-------------+-------------+-------------+
| UART        | TX          | RX          |
+-------------+-------------+-------------+
| UART1       | P9.24       | P9.26       |
| UART2       | P9.21       | P9.22       |
| UART4       | P9.13       | P9.11       |
| UART5       | P8.37       | P8.38       |
+-------------+-------------+-------------+
```

---

# 6. UART Wiring

Example using UART1:

```text
                 BeagleBone Black
                +----------------+
                |                |
P9.24 TX ------>|----------------|---- RX
                |                |
P9.26 RX <------|----------------|---- TX
                |                |
P9.1  GND ------|----------------|---- GND
                |                |
                +----------------+
```

Remember:

```text
BBB TX → Device RX
BBB RX ← Device TX
BBB GND → Device GND
```

---

# 7. UART Voltage

BeagleBone Black expansion-header UART signals use **3.3 V logic**.

```text
BBB UART
   |
   v
3.3 V Logic
   |
   v
UART Device
```

Do **not** connect a traditional RS-232 voltage-level signal directly to
the BeagleBone UART pins.

For RS-232:

```text
BeagleBone UART
      |
      v
RS-232 Transceiver
      |
      v
RS-232 Device
```

For RS-485:

```text
BeagleBone UART
      |
      v
RS-485 Transceiver
      |
      v
A / B Differential Bus
```

---

# 8. UART0 Debug Console

UART0 is commonly used for the BeagleBone Black serial debug console.

Typical boot flow:

```text
U-Boot
  |
  v
Kernel Boot Messages
  |
  v
Linux Console
  |
  v
Login Prompt
```

Conceptually:

```text
BeagleBone Black
      |
      v
UART0
      |
      v
Serial Debug Adapter
      |
      v
PC Terminal
```

Typical terminal configuration:

```text
Baud Rate : 115200
Data Bits : 8
Parity    : None
Stop Bits : 1
Flow Ctrl : None
```

This is commonly represented as:

```text
115200 8N1
```

---

# 9. Linux UART Device Nodes

UART devices are exposed through the Linux TTY subsystem.

Check:

```bash
ls /dev/ttyS*
```

You may see:

```text
/dev/ttyS0
/dev/ttyS1
/dev/ttyS2
...
```

The exact mapping between physical UART instances and `/dev/ttyS*`
depends on the kernel/BSP configuration.

Check boot messages:

```bash
dmesg | grep -i tty
```

Example:

```text
serial8250: ttyS0 at MMIO ...
serial8250: ttyS1 at MMIO ...
```

---

# 10. UART Device Tree

UART controllers are enabled through Device Tree.

Conceptually:

```dts
&uart1 {
    status = "okay";
};
```

Pinmux configuration is also required:

```dts
&am33xx_pinmux {
    uart1_pins: uart1_pins {
        pinctrl-single,pins = <
            /* UART1 TX/RX pin configuration */
        >;
    };
};
```

Then:

```dts
&uart1 {
    pinctrl-names = "default";
    pinctrl-0 = <&uart1_pins>;
    status = "okay";
};
```

The exact pinctrl properties depend on the kernel and BeagleBone Device
Tree version being used.

---

# 11. Project Device Tree Structure

Your project contains:

```text
beaglebone-black/
└── device-tree/
    └── uart/
        ├── bbb-uart.dts
        ├── bbb-uart.dtsi
        └── README.md
```

Example structure:

```text
bbb-uart.dts
      |
      +---- UART Controller
      |
      +---- Pinmux
      |
      +---- UART Configuration
```

---

# 12. UART Device Tree Flow

```text
bbb-uart.dts
      |
      v
bbb-uart.dtsi
      |
      v
Device Tree Compiler
      |
      v
       DTB
      |
      v
Bootloader
      |
      v
Linux Kernel
      |
      v
UART Driver
      |
      v
/dev/ttySx
```

---

# 13. Linux UART Architecture

```text
                User Application
                       |
                       v
                   /dev/ttySx
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
                   TX / RX
```

---

# 14. UART Driver

The Linux UART driver handles:

```text
1. UART initialization
2. Baud-rate configuration
3. TX handling
4. RX handling
5. Interrupt handling
6. FIFO management
7. Flow control
8. Error handling
9. TTY integration
10. Power management
```

---

# 15. UART Configuration

Typical UART parameters:

```text
Baud Rate
Data Bits
Parity
Stop Bits
Flow Control
```

Example:

```text
115200
8 data bits
No parity
1 stop bit
No hardware flow control
```

Represented as:

```text
115200 8N1
```

---

# 16. UART Baud Rate

Baud rate determines the symbol rate.

Common configurations:

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

Example:

```text
115200 baud
```

is commonly used for embedded Linux debug consoles.

---

# 17. UART Data Format

Example:

```text
8N1
```

means:

```text
8 → 8 Data Bits
N → No Parity
1 → 1 Stop Bit
```

Frame:

```text
       Start       Data              Stop
         |          |                 |
         v          v                 v

       ┌───┐ ┌────────────────────┐ ┌───┐
Idle ──┘   └─┤  D0 D1 ... D7       ├─┘
             └────────────────────┘
```

---

# 18. UART Transmission Flow

```text
Application
     |
     v
write()
     |
     v
TTY Layer
     |
     v
UART Driver
     |
     v
TX FIFO
     |
     v
UART Hardware
     |
     v
TX Pin
```

---

# 19. UART Receive Flow

```text
RX Pin
   |
   v
UART Hardware
   |
   v
RX FIFO
   |
   v
UART Driver
   |
   v
TTY Layer
   |
   v
Application
```

---

# 20. UART Interrupt Flow

UART commonly uses interrupts for receive/transmit events.

```text
UART Hardware
      |
      v
RX/TX Event
      |
      v
Interrupt
      |
      v
UART ISR
      |
      v
UART Driver
      |
      v
TTY Layer
      |
      v
Application
```

---

# 21. UART FIFO

UART hardware normally contains TX/RX FIFOs.

```text
                UART
        +------------------+
        |                  |
TX ---> | TX FIFO          |
        |                  |
RX <--- | RX FIFO          |
        |                  |
        +------------------+
```

FIFO reduces CPU overhead by allowing multiple bytes to be buffered.

---

# 22. UART Linux Test

Identify serial devices:

```bash
ls /dev/ttyS*
```

Check kernel logs:

```bash
dmesg | grep -i tty
```

Check serial-related logs:

```bash
dmesg | grep -i serial
```

---

# 23. UART Terminal Test

A common terminal program is:

```bash
sudo apt install minicom
```

Start:

```bash
sudo minicom -D /dev/ttyS1 -b 115200
```

Configuration:

```text
Device     : /dev/ttyS1
Baud Rate  : 115200
Data       : 8
Parity     : None
Stop       : 1
Flow Ctrl  : None
```

The exact `/dev/ttySx` must be verified for the UART you enabled.

---

# 24. UART Loopback Test

For a basic UART loopback:

```text
TX ─────────┐
            |
            v
           RX
```

Example:

```text
P9.24 TX ───────── P9.26 RX
```

Then send:

```text
Hello BBB
```

Expected:

```text
TX:
Hello BBB

RX:
Hello BBB
```

> Only perform a TX-to-RX loopback when the UART pins are configured
> appropriately and no other external transmitter is driving the RX line.

---

# 25. UART Loopback Architecture

```text
              BeagleBone Black

             +---------------+
             |               |
TX ----------|--------------+
             |              |
             |              |
RX <---------|--------------+
             |               |
             +---------------+
                    |
                    v
              UART Loopback
```

---

# 26. UART Test Using `echo`

For a suitable UART device:

```bash
echo "Hello BBB" | sudo tee /dev/ttyS1
```

Then observe the data using another serial terminal or connected UART
receiver.

---

# 27. UART Test Using `cat`

Receive data:

```bash
cat /dev/ttyS1
```

Then send data from another UART device.

Example:

```text
External PC
    |
    v
UART
    |
    v
BeagleBone
    |
    v
cat /dev/ttyS1
```

---

# 28. UART Configuration with `stty`

Check configuration:

```bash
stty -F /dev/ttyS1 -a
```

Configure:

```bash
sudo stty -F /dev/ttyS1 115200 cs8 -cstopb -parenb
```

Meaning:

```text
115200 → Baud rate
cs8     → 8 data bits
-cstopb → 1 stop bit
-parenb → No parity
```

Disable hardware flow control when appropriate:

```bash
sudo stty -F /dev/ttyS1 -crtscts
```

---

# 29. UART Hardware Flow Control

UART can optionally use:

```text
RTS
CTS
```

Conceptually:

```text
BBB                         Device

TX  ----------------------> RX
RX  <---------------------- TX

RTS ----------------------> CTS
CTS <---------------------- RTS
```

Hardware flow control prevents the sender from transmitting when the
receiver is not ready.

The required pins depend on the particular UART/pinmux configuration.

---

# 30. UART Software Flow Control

Software flow control commonly uses:

```text
XON
XOFF
```

It does not require additional hardware pins.

Conceptually:

```text
TX/RX
 |
 +---- XON  → Continue
 |
 +---- XOFF → Pause
```

It can be configured with `stty` when required.

---

# 31. UART Debugging

Check UART devices:

```bash
ls -l /dev/ttyS*
```

Check kernel:

```bash
dmesg | grep -i uart
```

Check TTY:

```bash
dmesg | grep -i tty
```

Check pinmux:

```bash
sudo cat /sys/kernel/debug/pinctrl/*/pinmux-pins
```

Check Device Tree:

```bash
find /proc/device-tree/ -iname "*uart*" 2>/dev/null
```

---

# 32. UART Pinmux Debugging

If UART is not working:

```text
UART not working
      |
      v
Check /dev/ttySx
      |
      v
Check Device Tree
      |
      v
Check UART status = "okay"
      |
      v
Check pinmux
      |
      v
Check TX/RX wiring
      |
      v
Check GND
      |
      v
Check baud rate
      |
      v
Check 8N1 configuration
      |
      v
Check logic analyzer
```

---

# 33. UART Logic Analyzer

A logic analyzer can verify TX/RX signals.

```text
Logic Analyzer       BeagleBone

CH0 ----------------> TX
CH1 ----------------> RX
GND ----------------> GND
```

For a transmitted character:

```text
TX
    ┌───┐ ┌─┐ ┌─┐ ┌─┐
────┘   └─┘ └─┘ └─┘ └────
     Start   Data    Stop
```

You can verify:

```text
1. Baud rate
2. Start bit
3. Data bits
4. Parity
5. Stop bit
6. Signal voltage
```

---

# 34. UART and Device Tree Overlay

Your project contains:

```text
beaglebone-black/
└── device-tree/
    └── overlays/
        └── bbb-uart-overlay.dts
```

An UART overlay can configure:

```text
1. UART controller
2. Pinmux
3. UART status
4. Optional flow-control pins
```

Conceptually:

```text
bbb-uart-overlay.dts
          |
          v
     Pinmux Setup
          |
          v
      UART Enable
          |
          v
      UART Driver
          |
          v
       /dev/ttySx
```

---

# 35. UART Driver Development

A custom UART peripheral driver normally does not replace the standard
UART controller driver.

Instead:

```text
Application
     |
     v
TTY / Serial Interface
     |
     v
Linux Serial Driver
     |
     v
AM335x UART Hardware
```

For a custom external UART device connected over another bus, such as
SPI, the driver architecture would instead be:

```text
Application
     |
     v
Custom Driver
     |
     v
SPI / I2C
     |
     v
External UART Device
```

---

# 36. UART Driver Responsibilities

For a UART controller driver:

```text
+--------------------------------+
| UART Driver                    |
+--------------------------------+
| Probe / Remove                 |
| Clock Configuration            |
| Baud Rate Configuration        |
| TX Handling                    |
| RX Handling                    |
| FIFO Handling                  |
| Interrupt Handling             |
| Error Handling                 |
| Flow Control                   |
| Power Management               |
| TTY Integration                |
+--------------------------------+
```

---

# 37. UART Error Conditions

UART hardware can report:

```text
Overrun Error
Framing Error
Parity Error
Break Condition
```

### Overrun

RX FIFO/data was not serviced quickly enough.

```text
Incoming Data
      |
      v
RX FIFO FULL
      |
      v
New Data
      |
      v
Overrun
```

### Framing Error

Usually indicates incorrect baud/format or signal integrity.

```text
Expected:
8N1

Received:
Incorrect timing/stop bit
```

### Parity Error

Occurs when parity checking fails.

---

# 38. UART Applications

### GPS/GNSS

```text
GNSS Module
     |
     | UART
     v
BeagleBone Black
     |
     v
GPS Application
```

### Bluetooth

```text
Bluetooth Module
       |
       | UART
       v
BeagleBone Black
```

### Modem

```text
LTE Modem
    |
    | UART
    v
BeagleBone Black
```

### Microcontroller

```text
MCU
 |
 | UART
 v
BeagleBone Black
```

---

# 39. UART Pin Conflict

UART pins are multiplexed with other peripherals.

Conceptually:

```text
P9.24
  |
  +---- UART1_TXD
  |
  +---- Alternate Function
  |
  +---- GPIO
```

Therefore, enabling UART requires the appropriate pinmux.

Do not simultaneously configure the same physical pin for:

```text
UART
SPI
I2C
PWM
GPIO
```

unless the hardware/software design intentionally supports switching
between those functions.

---

# 40. UART Pin Quick Reference

```text
+-------------+-------------+------------------+
| UART        | TX          | RX               |
+-------------+-------------+------------------+
| UART1       | P9.24       | P9.26            |
| UART2       | P9.21       | P9.22            |
| UART4       | P9.13       | P9.11            |
| UART5       | P8.37       | P8.38            |
+-------------+-------------+------------------+
```

---

# 41. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   └── pinout/
│       └── uart_pin_map.md
│
├── device-tree/
│   ├── uart/
│   │   ├── bbb-uart.dts
│   │   ├── bbb-uart.dtsi
│   │   └── README.md
│   │
│   └── overlays/
│       ├── bbb-uart-overlay.dts
│       └── README.md
│
├── drivers/
│   └── uart/
│       ├── README.md
│       └── ...
│
└── tests/
    └── uart/
        ├── uart_loopback_test.sh
        ├── uart_tx_test.sh
        ├── uart_rx_test.sh
        └── README.md
```

---

# 42. Complete UART Architecture

```text
                         BeagleBone Black
                                |
                                v
                          P8 / P9 Header
                                |
                                v
                             Pinmux
                                |
                                v
                          Device Tree
                                |
                                v
                         AM335x UART HW
                                |
                                v
                          UART Driver
                                |
                                v
                           TTY Layer
                                |
                                v
                           /dev/ttySx
                                |
                 +--------------+--------------+
                 |              |              |
                 v              v              v
                GPS          Modem           MCU
```

---

# 43. Complete UART Development Flow

```text
Hardware Pin Mapping
        |
        v
Device Tree
        |
        v
Pinmux Configuration
        |
        v
UART Controller Enable
        |
        v
Linux UART Driver
        |
        v
TTY Device
        |
        v
/dev/ttySx
        |
        v
UART Test
        |
        +---- Loopback
        |
        +---- TX/RX
        |
        +---- Logic Analyzer
        |
        +---- External Device
```

---

# 44. UART Testing Checklist

```text
[ ] UART controller enabled
[ ] Device Tree configured
[ ] Pinmux configured
[ ] UART driver loaded
[ ] /dev/ttySx available
[ ] TX tested
[ ] RX tested
[ ] GND connected
[ ] 3.3 V logic verified
[ ] Baud rate verified
[ ] Data bits verified
[ ] Parity verified
[ ] Stop bits verified
[ ] Flow control verified
[ ] Loopback tested
[ ] Logic analyzer tested
[ ] Kernel logs checked
[ ] External UART device tested
```

## Project File

```text
beaglebone-black/hardware/pinout/uart_pin_map.md
```

