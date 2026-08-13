# BeagleBone Black CAN Demo

## Overview

This directory contains SocketCAN-based CAN demonstration programs
for the BeagleBone Black.

Programs:

- `can_demo` - CAN transmit and receive demonstration
- `can_filter` - CAN ID filtering demonstration

The programs use the Linux SocketCAN API.

---

## Directory Structure

```text
09_can/
├── can_demo.c
├── can_demo.h
├── can_filter.c
├── can_filter.h
├── Makefile
└── README.md
CAN Architecture
                 User Application
                       |
             +---------+---------+
             |                   |
             v                   v
         can_demo           can_filter
             |                   |
             +---------+---------+
                       |
                       v
                  SocketCAN
                       |
                       v
                  CAN_RAW Socket
                       |
                       v
                 Linux CAN Core
                       |
                       v
                 CAN Controller
                       |
                       v
                    CAN Bus
Build
make

Expected output:

can_demo
can_filter

Check:

ls -l can_demo can_filter
CAN Interface Setup

Check CAN interfaces:

ip link show

Check CAN controller:

ip -details link show can0

Configure CAN bitrate:

sudo ip link set can0 type can bitrate 500000

Enable the interface:

sudo ip link set can0 up

Verify:

ip -details link show can0
CAN Transmission

Run:

./can_demo

The demo transmits:

CAN ID : 0x100
DLC    : 8
DATA   : 11 22 33 44 55 66 77 88

You can also transmit using the Linux cansend utility:

cansend can0 100#1122334455667788
CAN Reception

Use:

candump can0

Example:

can0  100   [8]  11 22 33 44 55 66 77 88
CAN Filter

Run:

./can_filter

The program configures:

CAN ID   = 0x200
MASK     = 0x7FF

Therefore, only standard CAN frames with ID 0x200 are accepted.

Send a matching frame:

cansend can0 200#0102030405060708

The filter program should receive it.

A different ID such as:

cansend can0 201#0102030405060708

will not match the configured filter.

CAN Filter Logic

SocketCAN uses:

(frame.can_id & filter.can_mask)
        ==
(filter.can_id & filter.can_mask)

For:

Filter ID  = 0x200
Mask       = 0x7FF

only the exact 11-bit CAN ID 0x200 is accepted.

CAN Frame

A standard CAN frame contains:

+-----------+-----+--------------------------+
| CAN ID    | DLC | Data                     |
+-----------+-----+--------------------------+
| 11 bits   | 0-8 | 0-8 bytes                |
+-----------+-----+--------------------------+

Example:

ID   = 0x100
DLC  = 8
DATA = 11 22 33 44 55 66 77 88
CAN TX Flow
Application
     |
     v
socket()
     |
     v
CAN_RAW
     |
     v
bind(can0)
     |
     v
write()
     |
     v
SocketCAN
     |
     v
CAN Controller
     |
     v
CAN TX
     |
     v
CAN Bus
CAN RX Flow
CAN Bus
   |
   v
CAN Controller
   |
   v
SocketCAN
   |
   v
CAN_RAW Socket
   |
   v
read()
   |
   v
Application
Filter Flow
CAN Bus
   |
   v
SocketCAN
   |
   v
CAN_RAW Filter
   |
   +---- ID 0x200 ----> Application
   |
   +---- Other IDs ---> Rejected
Useful Commands

Check CAN interface:

ip link show can0

Show CAN details:

ip -details link show can0

Enable:

sudo ip link set can0 up

Disable:

sudo ip link set can0 down

Set bitrate:

sudo ip link set can0 type can bitrate 500000

Receive:

candump can0

Transmit:

cansend can0 100#11223344

Show CAN statistics:

ip -statistics link show can0
CAN Error Monitoring

Use:

candump -e can0

This displays CAN error frames.

Check interface statistics:

ip -statistics link show can0

Look for:

RX packets
TX packets
errors
dropped
overruns
Testing
Terminal 1
candump can0
Terminal 2
cansend can0 100#1122334455667788

Terminal 1 should show:

can0  100   [8]  11 22 33 44 55 66 77 88
CAN Filter Test
Terminal 1
./can_filter
Terminal 2

Send matching ID:

cansend can0 200#0102030405060708

Expected:

Filtered CAN frame:
ID   : 0x200
DLC  : 8
DATA : 01 02 03 04 05 06 07 08

Send non-matching ID:

cansend can0 201#0102030405060708

The filter application should not display that frame.

Important Note

This is a user-space SocketCAN demonstration, not a replacement
for the BeagleBone Black CAN controller kernel driver.

The normal Linux architecture is:

CAN Application
      |
      v
SocketCAN API
      |
      v
CAN RAW Socket
      |
      v
Linux CAN Core
      |
      v
SoC CAN Driver
      |
      v
CAN Controller Hardware
      |
      v
CAN Transceiver
      |
      v
CAN Bus

For production systems, the CAN controller and pinmux should be
configured through the appropriate Device Tree and existing Linux
CAN driver.


### Final structure

```text
beaglebone-black/
└── drivers/
    └── 09_can/
        ├── can_demo.c
        ├── can_demo.h
        ├── can_filter.c
        ├── can_filter.h
        ├── Makefile
        └── README.md

Build:

cd beaglebone-black/drivers/09_can
make

This CAN directory is intentionally different from the SPI/UART/ADC/PWM directories: CAN is normally accessed through SocketCAN, so the application talks to the Linux CAN subsystem rather than directly creating a custom character-device CAN driver.
