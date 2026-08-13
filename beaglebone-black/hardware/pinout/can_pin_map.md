# BeagleBone Black CAN Pin Map

## 1. Overview

The **BeagleBone Black** uses the **TI AM335x** SoC, which provides
**DCAN (Controller Area Network)** hardware.

CAN is commonly used in automotive, industrial, robotics, and embedded
systems for reliable communication between multiple controllers.

The Linux architecture is:

```text
CAN Application
      |
      v
   SocketCAN
      |
      v
 Linux CAN Driver
      |
      v
 AM335x DCAN Controller
      |
      v
 CAN Transceiver
      |
      v
 CANH / CANL
      |
      v
   CAN Network
```

> **Important:** The AM335x CAN controller requires an external CAN
> transceiver to connect to a physical CAN bus. Do not connect the
> AM335x CAN TX/RX pins directly to CANH/CANL.

---

# 2. CAN Interface

The AM335x provides two CAN controllers:

```text
CAN0 / DCAN0
CAN1 / DCAN1
```

Depending on the BeagleBone Black hardware configuration and Device Tree,
one or both controllers can be exposed through the expansion headers.

For this project, the CAN interface should be treated as:

```text
AM335x DCAN
     |
     v
CAN Linux Driver
     |
     v
SocketCAN
     |
     v
CAN Transceiver
     |
     v
CAN Bus
```

---

# 3. CAN Pin Mapping

A commonly used BeagleBone Black CAN configuration is:

| CAN Signal | Header Pin | Function |
| ---------- | ---------: | -------- |
| DCAN0_TX   |      P9.19 | CAN TX   |
| DCAN0_RX   |      P9.20 | CAN RX   |
| GND        |       P9.1 | Ground   |

However, **P9.19/P9.20 are also commonly used for I2C2 SCL/SDA**, so
CAN and I2C cannot use those pins simultaneously in the same pinmux
configuration.

Therefore, your Device Tree must explicitly configure the selected
function.

---

# 4. CAN Pinmux Concept

The AM335x uses pin multiplexing.

```text
                  P9.19
                    |
          +---------+---------+
          |                   |
       I2C2_SCL           DCAN0_TX
          |                   |
          +---------+---------+
                    |
                PINMUX
                    |
              Device Tree
```

Similarly:

```text
                  P9.20
                    |
          +---------+---------+
          |                   |
       I2C2_SDA           DCAN0_RX
          |                   |
          +---------+---------+
                    |
                PINMUX
                    |
              Device Tree
```

Only the selected mux function becomes active.

---

# 5. CAN Header Connection

Basic connection:

```text
BeagleBone Black
       |
       | DCAN TX
       v
CAN Transceiver
       |
       +---------------- CANH
       |
       +---------------- CANL
       |
       +---------------- GND
```

Example:

```text
BBB                    CAN Transceiver
---                    ---------------

DCAN0_TX ------------> TXD
DCAN0_RX <------------ RXD
GND ------------------ GND

                         CANH -------- CAN Bus
                         CANL -------- CAN Bus
```

---

# 6. CAN Transceiver

The AM335x provides the CAN controller but does not directly provide
the physical CAN bus interface.

Use an external CAN transceiver such as a suitable 3.3 V CAN transceiver.

Architecture:

```text
             BeagleBone Black
                    |
              AM335x DCAN
              +-----------+
              |           |
           DCAN_TX     DCAN_RX
              |           |
              v           ^
          +-------------------+
          |  CAN Transceiver  |
          +-------------------+
                  |
             +----+----+
             |         |
           CANH       CANL
             |         |
             +----+----+
                  |
               CAN BUS
```

---

# 7. CAN Bus Wiring

A typical two-node CAN network:

```text
      CAN Node 1                         CAN Node 2
+------------------+                +------------------+
| BeagleBone Black |                | CAN Controller   |
| + Transceiver   |                | + Transceiver   |
+------------------+                +------------------+
        | CANH ---------------------------- CANH |
        | CANL ---------------------------- CANL |
        | GND  ---------------------------- GND  |
```

Use proper CAN bus termination.

```text
CANH =================================== CANH
       |                              |
      120Ω                           120Ω
       |                              |
CANL =================================== CANL
```

The **120 Ω termination resistors should normally be placed at the two
physical ends of the CAN bus**, not at every node.

---

# 8. CAN Pin Configuration

The Device Tree configures the AM335x pins for CAN operation.

Conceptually:

```text
P9.19
  |
  +---- DCAN0_TX
  |
  v
Pin Controller
  |
  v
Device Tree
```

```text
P9.20
  |
  +---- DCAN0_RX
  |
  v
Pin Controller
  |
  v
Device Tree
```

Project files:

```text
beaglebone-black/
└── device-tree/
    └── can/
        ├── bbb-can.dts
        ├── bbb-can.dtsi
        └── README.md
```

---

# 9. Linux CAN Driver Architecture

The Linux CAN stack uses **SocketCAN**.

```text
              User Application
                     |
                     v
                SocketCAN
                     |
                     v
              CAN Network Core
                     |
                     v
              AM335x CAN Driver
                     |
                     v
                DCAN Hardware
                     |
                     v
              CAN Transceiver
                     |
                     v
                  CAN Bus
```

The CAN interface is normally exposed as:

```text
can0
can1
```

depending on which controller(s) are enabled.

---

# 10. Check CAN Interfaces

Run:

```bash
ip link show
```

Look for:

```text
can0
```

or:

```text
can1
```

Example:

```text
5: can0: <NOARP,UP,LOWER_UP> mtu 16 qdisc pfifo_fast state UP
```

---

# 11. Configure CAN Bitrate

Example for **500 kbit/s**:

```bash
sudo ip link set can0 type can bitrate 500000
```

Bring the interface up:

```bash
sudo ip link set can0 up
```

Verify:

```bash
ip -details link show can0
```

Expected information includes:

```text
bitrate 500000
```

---

# 12. Bring CAN Interface Down

```bash
sudo ip link set can0 down
```

Reconfigure when required:

```bash
sudo ip link set can0 type can bitrate 250000
```

Then:

```bash
sudo ip link set can0 up
```

---

# 13. CAN Transmit Test

The `can-utils` package provides useful SocketCAN testing tools.

Install:

```bash
sudo apt install can-utils
```

Transmit a CAN frame:

```bash
cansend can0 123#11223344
```

Meaning:

```text
CAN ID : 0x123
DATA   : 11 22 33 44
```

---

# 14. CAN Receive Test

Run:

```bash
candump can0
```

Example output:

```text
can0  123   [4]  11 22 33 44
```

This means:

```text
Interface : can0
CAN ID    : 0x123
DLC       : 4
Data      : 11 22 33 44
```

---

# 15. CAN Loopback Test

For driver-level testing, CAN loopback can be useful when external
hardware is not available.

Configure:

```bash
sudo ip link set can0 type can bitrate 500000 loopback on
```

Bring interface up:

```bash
sudo ip link set can0 up
```

Start receiver:

```bash
candump can0
```

Send:

```bash
cansend can0 123#DEADBEEF
```

Expected:

```text
can0  123   [4]  DE AD BE EF
```

---

# 16. CAN Driver Probe Flow

```text
             Bootloader
                 |
                 v
            Device Tree
                 |
                 v
             Pinmux
                 |
                 v
          AM335x DCAN Node
                 |
                 v
          CAN Platform Driver
                 |
                 v
             CAN Core
                 |
                 v
             SocketCAN
                 |
                 v
              can0
                 |
                 v
          User Application
```

---

# 17. Device Tree Flow

```text
bbb-can.dts
     |
     v
bbb-can.dtsi
     |
     v
Device Tree Compiler
     |
     v
bbb.dtb
     |
     v
Linux Kernel
     |
     v
DCAN Driver
     |
     v
SocketCAN
     |
     v
can0
```

---

# 18. Kernel Configuration

Check whether CAN support is enabled:

```bash
zcat /proc/config.gz | grep CAN
```

Typical configuration options include:

```text
CONFIG_CAN=y
CONFIG_CAN_RAW=y
CONFIG_CAN_DEV=y
```

The AM335x CAN controller driver configuration may also be required,
depending on the kernel version and driver organization.

Check available CAN modules:

```bash
lsmod | grep can
```

---

# 19. CAN Kernel Messages

After boot or driver initialization:

```bash
dmesg | grep -i can
```

Also check:

```bash
dmesg | grep -i dcan
```

Useful information includes:

```text
CAN controller detected
CAN interface registered
can0 created
```

---

# 20. CAN Error Debugging

Check interface statistics:

```bash
ip -details -statistics link show can0
```

Look for:

```text
RX packets
TX packets
RX errors
TX errors
bus errors
```

A CAN interface can enter different bus states:

```text
ERROR-ACTIVE
ERROR-PASSIVE
BUS-OFF
```

If the interface enters `BUS-OFF`, investigate:

```text
1. CANH/CANL wiring
2. CAN termination
3. Bitrate mismatch
4. CAN transceiver power
5. Ground connection
6. Incorrect pinmux
7. Missing second CAN node
```

---

# 21. CAN Testing Setup

Recommended test setup:

```text
             BeagleBone Black
                    |
                    v
              AM335x DCAN
                    |
                    v
              CAN Transceiver
                    |
             +------+------+
             |             |
           CANH           CANL
             |             |
             +------+------+
                    |
             CAN Analyzer /
             Second CAN Node
```

---

# 22. Test Procedure

### Step 1 — Verify interface

```bash
ip link show
```

### Step 2 — Configure bitrate

```bash
sudo ip link set can0 type can bitrate 500000
```

### Step 3 — Enable interface

```bash
sudo ip link set can0 up
```

### Step 4 — Start receiver

```bash
candump can0
```

### Step 5 — Transmit frame

```bash
cansend can0 123#11223344
```

### Step 6 — Check statistics

```bash
ip -details -statistics link show can0
```

### Step 7 — Check kernel logs

```bash
dmesg | grep -i can
```

---

# 23. CAN Driver Test Checklist

```text
[ ] CAN controller enabled
[ ] Device Tree configured
[ ] Pinmux configured
[ ] CAN driver loaded
[ ] SocketCAN available
[ ] can0 created
[ ] CAN bitrate configured
[ ] CAN interface UP
[ ] CAN transceiver connected
[ ] CANH connected
[ ] CANL connected
[ ] GND connected
[ ] 120-ohm termination verified
[ ] Second CAN node available
[ ] cansend tested
[ ] candump tested
[ ] RX verified
[ ] TX verified
[ ] Error counters checked
[ ] BUS-OFF recovery tested
```

---

# 24. CAN Pin Map Quick Reference

```text
+----------------+-------------+------------------+
| Signal         | BBB Header  | Function         |
+----------------+-------------+------------------+
| DCAN0_TX       | P9.19       | CAN Transmit     |
| DCAN0_RX       | P9.20       | CAN Receive      |
| GND            | P9.1        | Ground           |
+----------------+-------------+------------------+
```

> **Pinmux note:** P9.19 and P9.20 are multiplexed pins and are commonly
> used by I2C2. Your Device Tree must select the CAN function before
> using them for DCAN.

---

# 25. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   └── pinout/
│       └── can_pin_map.md
│
├── device-tree/
│   └── can/
│       ├── bbb-can.dts
│       ├── bbb-can.dtsi
│       └── README.md
│
├── drivers/
│   └── can/
│
└── tests/
    └── can/
```

The complete CAN development flow is:

```text
Hardware Wiring
      |
      v
CAN Pin Mapping
      |
      v
Device Tree / Pinmux
      |
      v
Linux CAN Driver
      |
      v
SocketCAN
      |
      v
can0
      |
      v
can-utils
      |
      v
CAN TX/RX Testing
```

---

# 26. Final CAN Architecture

```text
                 BEAGLEBONE BLACK
                        |
                        v
                 +-------------+
                 |   AM335x    |
                 |   DCAN0     |
                 +-------------+
                    |       |
                 TX |       | RX
                    |       |
                    v       ^
              +----------------+
              | CAN Transceiver|
              +----------------+
                     |
                +----+----+
                |         |
              CANH       CANL
                |         |
                +----+----+
                     |
                 CAN BUS
                     |
          +----------+----------+
          |                     |
      CAN Node 2            CAN Analyzer
```

**Project file:**

```text
beaglebone-black/hardware/pinout/can_pin_map.md
```

