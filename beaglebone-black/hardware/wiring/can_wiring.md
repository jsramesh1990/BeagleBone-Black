# `can_wiring.md`

````markdown
# BeagleBone Black CAN Wiring

## 1. Overview

This document describes the hardware wiring required to test CAN
(Controller Area Network) communication on the BeagleBone Black.

The BeagleBone Black uses the AM335x SoC, which provides DCAN
controllers. The DCAN peripheral provides the CAN controller, while an
external CAN transceiver is required to convert the controller's logic
signals to the physical CAN bus.

Typical CAN applications include:

- Automotive systems
- Robotics
- Industrial automation
- Motor controllers
- Battery management systems
- Embedded control systems
- ECU communication
- Industrial sensors

The complete path is:

```text
User Application
       |
       v
CAN SocketCAN
       |
       v
Linux CAN Framework
       |
       v
DCAN Controller Driver
       |
       v
AM335x DCAN
       |
       v
CAN TX / RX
       |
       v
CAN Transceiver
       |
       v
CANH / CANL
       |
       v
CAN Bus
````

---

# 2. Important: CAN Controller vs CAN Transceiver

The BeagleBone Black's SoC contains the CAN controller, but the
controller's TX/RX logic signals are not directly connected to the
physical CANH/CANL bus.

An external CAN transceiver is required.

```text
BeagleBone Black
       |
       | CAN_TX
       v
+------------------+
| CAN Transceiver  |
+------------------+
       |
       +------ CANH
       |
       +------ CANL
```

Do **not** connect CANH or CANL directly to a BeagleBone Black GPIO or
DCAN TX/RX pin.

---

# 3. Required Components

For a basic CAN test:

```text
1 × BeagleBone Black
1 × 3.3 V-compatible CAN transceiver/module
120 Ω termination resistor
Jumper wires
CAN cable
```

For a two-node test:

```text
2 × CAN-capable boards
2 × CAN transceivers
2 × 120 Ω termination resistors
CAN wiring
```

Example CAN transceiver types may include devices from the
SN65HVD/TJA-series families. Select a transceiver that is electrically
compatible with the BeagleBone Black and your CAN bus.

---

# 4. CAN Signals

The CAN controller uses:

```text
CAN_TX
CAN_RX
```

The physical CAN bus uses:

```text
CANH
CANL
```

The relationship is:

```text
BeagleBone DCAN
      |
      | CAN_TX
      v
CAN Transceiver
      |
      +------ CANH
      |
      +------ CANL
      |
      ^
      |
      | CAN_RX
      |
BeagleBone DCAN
```

---

# 5. Basic CAN Wiring

```text
             BeagleBone Black
                    |
             +------+------+
             |             |
          CAN_TX         CAN_RX
             |             |
             v             ^
       +-----------------------+
       |   CAN Transceiver     |
       |                       |
       | TXD              RXD  |
       |                       |
       | CANH             CANL |
       +---+---------------+---+
           |               |
           |               |
         CANH             CANL
           |               |
           v               v
        CAN BUS        CAN BUS
```

---

# 6. Controller-to-Transceiver Wiring

The controller side is:

```text
BBB DCAN TX  -------->  Transceiver TXD
BBB DCAN RX  <--------  Transceiver RXD
BBB GND      ---------  Transceiver GND
```

Physical bus side:

```text
Transceiver CANH  -----> CANH
Transceiver CANL  -----> CANL
```

Power:

```text
BBB / Supply 3.3 V -----> Transceiver VCC
GND --------------------> Transceiver GND
```

The transceiver's required supply voltage must be checked against its
datasheet.

---

# 7. CAN Wiring Table

| BeagleBone / System | CAN Transceiver |
| ------------------- | --------------- |
| DCAN TX             | TXD             |
| DCAN RX             | RXD             |
| GND                 | GND             |
| Appropriate VCC     | VCC             |
| —                   | CANH            |
| —                   | CANL            |

The exact BeagleBone header pins for CAN TX/RX should be taken from:

```text
hardware/pinout/can_pin_map.md
```

---

# 8. CANH and CANL

CAN uses a differential physical layer.

```text
CANH
  |
  |------------------------------+
  |                              |
  v                              v
Node 1                         Node 2

CANL
  |
  |------------------------------+
  |                              |
  v                              v
Node 1                         Node 2
```

Both CANH and CANL are required for normal differential CAN
communication.

---

# 9. Two-Node CAN Test

A proper CAN communication test should normally use two CAN nodes.

```text
              CAN BUS

Node 1                                  Node 2
------                                  ------

BBB                                     CAN Device
 |                                         |
 v                                         v
CAN Transceiver                       CAN Transceiver
 |                                         |
 +------ CANH -----------------------------+
 |                                         |
 +------ CANL -----------------------------+
```

The two nodes share:

```text
CANH
CANL
```

and should have a common signal reference where appropriate for the
transceiver/system design.

---

# 10. Complete Two-Node Circuit

```text
             NODE 1                         NODE 2
        BeagleBone Black                CAN Device
              |                              |
         CAN Controller                  CAN Controller
              |                              |
              v                              v
       +-------------+                +-------------+
       | Transceiver |                | Transceiver |
       +------+------+                +------+------+
              |                              |
             CANH============================CANH
              |                              |
             CANL============================CANL
              |                              |
            120 Ω                          120 Ω
              |                              |
             +--------------------------------+
```

The 120 Ω termination resistors are placed at the **two physical ends
of the CAN bus**, not at every node.

---

# 11. CAN Bus Termination

A typical high-speed CAN bus uses:

```text
120 Ω termination
```

at each end.

Conceptually:

```text
              120 Ω                    120 Ω
CANH ---------/\/\----------------------/\/\---------
              |                         |
             CANL                      CANL
```

More accurately:

```text
CANH =================================================
       |                                            |
      120 Ω                                        120 Ω
       |                                            |
CANL =================================================
```

The two termination resistors appear in parallel from the bus
perspective, giving approximately:

```text
120 Ω || 120 Ω = 60 Ω
```

when measured between CANH and CANL with the bus powered down and no
other parallel loading.

---

# 12. Termination Placement

Correct:

```text
120 Ω
  |
Node 1 ============================ Node 2
                                      |
                                     120 Ω
```

Incorrect:

```text
Node 1 -- 120 Ω -- Node 2 -- 120 Ω -- Node 3 -- 120 Ω
```

Termination belongs at the physical ends of the bus.

---

# 13. CAN Bus Topology

Recommended topology:

```text
Node 1          Node 2          Node 3
  |               |               |
  |               |               |
  +---------------+---------------+
          CANH / CANL BUS
```

Avoid long star connections for a conventional high-speed CAN bus.

Prefer:

```text
Node 1 ---- Node 2 ---- Node 3 ---- Node 4
```

with short node stubs.

---

# 14. CAN Physical Wiring

```text
                  CAN BUS

Node 1                    Node 2                    Node 3
  |                         |                         |
  |                         |                         |
CANH========================CANH======================CANH
  |                         |                         |
CANL========================CANL======================CANL
```

The bus should be routed as a controlled differential pair where
practical.

---

# 15. CAN Ground

For a basic bench test:

```text
Node 1 GND ---------------- Node 2 GND
```

The exact grounding arrangement in a deployed system depends on the
transceiver, isolation, grounding strategy, and system architecture.

For non-isolated laboratory testing, a common reference is generally
recommended.

---

# 16. CAN Transceiver Enable / Standby

Some CAN transceivers have additional pins such as:

```text
EN
STB
RS
Silent
Slope
```

Example:

```text
BeagleBone
    |
    +---- CAN_EN ----> Transceiver EN
```

Check the transceiver datasheet.

If the transceiver is in standby or silent mode:

```text
CAN Controller
      |
      v
Transceiver
      |
      X
CAN Bus
```

Communication will not work correctly.

---

# 17. CAN Power

Typical concept:

```text
BeagleBone / Power Supply
          |
          v
     Transceiver VCC
          |
          v
       CAN PHY
```

Verify:

```text
[ ] Transceiver supply voltage
[ ] Logic-level compatibility
[ ] Ground
[ ] Enable/standby state
```

Do not assume every CAN transceiver uses the same supply voltage.

---

# 18. CAN Device Tree

Project Device Tree files:

```text
beaglebone-black/
└── device-tree/
    └── can/
        ├── bbb-can.dts
        ├── bbb-can.dtsi
        └── README.md
```

Conceptual flow:

```text
bbb-can.dts
      |
      v
Pinmux
      |
      v
DCAN Controller
      |
      v
Linux CAN Driver
      |
      v
SocketCAN
      |
      v
can0
```

---

# 19. CAN Driver Architecture

```text
+--------------------------------+
| User Application               |
+---------------+----------------+
                |
                v
+--------------------------------+
| SocketCAN                      |
+---------------+----------------+
                |
                v
+--------------------------------+
| Linux CAN Framework            |
+---------------+----------------+
                |
                v
+--------------------------------+
| CAN Controller Driver          |
+---------------+----------------+
                |
                v
+--------------------------------+
| AM335x DCAN                    |
+---------------+----------------+
                |
                v
+--------------------------------+
| CAN TX / CAN RX                |
+---------------+----------------+
                |
                v
+--------------------------------+
| CAN Transceiver                |
+---------------+----------------+
                |
                v
+--------------------------------+
| CANH / CANL                    |
+--------------------------------+
```

---

# 20. Check CAN Interface

After booting Linux:

```bash
ip link show
```

Look for:

```text
can0
```

Example:

```text
can0: <NOARP,UP,LOWER_UP> ...
```

If `can0` is not present, check Device Tree and the CAN driver.

---

# 21. Check Kernel CAN Support

Run:

```bash
dmesg | grep -i can
```

Also:

```bash
dmesg | grep -i dcan
```

Check network interfaces:

```bash
ip -details link show
```

---

# 22. Configure CAN Bitrate

Example:

```bash
sudo ip link set can0 down
```

Configure:

```bash
sudo ip link set can0 type can bitrate 500000
```

Bring interface up:

```bash
sudo ip link set can0 up
```

Verify:

```bash
ip -details link show can0
```

This example uses:

```text
CAN bitrate = 500 kbit/s
```

Both communicating nodes must use compatible CAN timing/bitrate
configuration.

---

# 23. CAN Interface Status

Check:

```bash
ip -details -statistics link show can0
```

This can show information such as:

```text
bitrate
state
RX packets
TX packets
RX errors
TX errors
bus errors
```

---

# 24. CAN Test Utilities

Common SocketCAN utilities include:

```text
candump
cansend
cangen
canplayer
```

They are typically provided by the Linux `can-utils` package.

Install on Debian/Ubuntu-based systems:

```bash
sudo apt update
sudo apt install can-utils
```

---

# 25. CAN Receive Test

On Node 2:

```bash
candump can0
```

Example:

```text
can0  123   [4]  11 22 33 44
```

This means a CAN frame with:

```text
CAN ID = 0x123
DLC    = 4
DATA   = 11 22 33 44
```

---

# 26. CAN Transmit Test

On Node 1:

```bash
cansend can0 123#11223344
```

Node 2 should display:

```text
can0  123   [4]  11 22 33 44
```

This validates the basic CAN communication path.

---

# 27. CAN Frame Format

A basic CAN data frame contains:

```text
CAN ID
   |
   v
DLC
   |
   v
DATA
```

Example:

```text
ID       = 0x123
DLC      = 4
DATA     = 11 22 33 44
```

Command:

```bash
cansend can0 123#11223344
```

---

# 28. CAN Loopback Mode

Linux CAN can also provide controller-level loopback testing.

Conceptually:

```text
Application
     |
     v
SocketCAN
     |
     v
CAN Controller
     |
     +----------+
                |
                v
             Loopback
                |
                v
SocketCAN
```

However, software loopback does **not** replace a physical two-node
CAN test.

For hardware validation, use:

```text
Node 1 ↔ CAN Bus ↔ Node 2
```

---

# 29. CAN Hardware Test

Recommended test:

```text
Node 1
  |
  | CANH
  +=======================+
                          |
                          |
                       Node 2
                          |
  +=======================+
  |
  | CANL
  |
```

Configure both nodes:

```text
Bitrate = 500000
```

Node 2:

```bash
candump can0
```

Node 1:

```bash
cansend can0 123#DEADBEEF
```

Expected:

```text
can0  123   [4]  DE AD BE EF
```

---

# 30. CAN Logic Analyzer / Oscilloscope Test

For physical-layer debugging, probe:

```text
CANH
CANL
GND
```

Observe the differential bus activity.

Conceptually:

```text
CANH  ____/‾‾‾‾\____
CANL  ‾‾‾‾\____/‾‾‾
```

The exact waveform depends on the transceiver, bus loading, bitrate,
termination, and measurement setup.

---

# 31. CAN Differential Signaling

CAN uses differential signaling.

The receiver determines the bus state from the voltage difference
between:

```text
CANH
CANL
```

Conceptually:

```text
CAN Differential Signal

CANH  ────────┐    ┌────────
              │    │
CANL  ──┐     │    │
        └─────┘    └────────
```

This provides better noise immunity than a single-ended signaling
scheme.

---

# 32. CAN Bus-Off

A CAN controller can enter:

```text
BUS-OFF
```

when excessive transmission errors occur.

Check:

```bash
ip -details -statistics link show can0
```

Possible causes:

```text
[ ] Missing termination
[ ] Incorrect bitrate
[ ] Incorrect wiring
[ ] CANH/CANL swapped
[ ] Only one active node
[ ] Faulty transceiver
[ ] Electrical noise
[ ] Incorrect pinmux
```

---

# 33. CANH / CANL Reversed

Incorrect:

```text
Node 1 CANH -------- CANL Node 2
Node 1 CANL -------- CANH Node 2
```

Correct:

```text
Node 1 CANH -------- CANH Node 2
Node 1 CANL -------- CANL Node 2
```

Always verify the transceiver module labels and schematic because board
silkscreens can differ.

---

# 34. Termination Test

Power down the CAN network.

Measure resistance between:

```text
CANH ↔ CANL
```

For a correctly terminated two-end bus with two 120 Ω terminators, the
measured resistance is approximately:

```text
60 Ω
```

This is a useful physical wiring check.

---

# 35. CAN Debugging Flow

```text
                    CAN Failure
                        |
                        v
                 Is can0 present?
                   /          \
                 NO            YES
                 |              |
                 v              v
          Check Device Tree   Check bitrate
                                |
                                v
                         Check CANH/CANL
                                |
                                v
                         Check termination
                                |
                                v
                         Check transceiver
                                |
                                v
                          Check CAN GND
                                |
                                v
                         Check bus state
                                |
                                v
                           candump
                                |
                                v
                           cansend
```

---

# 36. CAN Wiring Troubleshooting

### Problem: `can0` does not exist

Check:

```text
[ ] CAN controller enabled
[ ] Device Tree
[ ] Pinmux
[ ] Kernel CAN driver
[ ] Kernel logs
```

### Problem: `candump` receives nothing

Check:

```text
[ ] Second CAN node exists
[ ] CANH connected correctly
[ ] CANL connected correctly
[ ] Same bitrate
[ ] Transceiver enabled
[ ] Termination
[ ] CAN controller state
```

### Problem: Bus-off

Check:

```text
[ ] Bitrate
[ ] CANH/CANL wiring
[ ] Termination
[ ] Transceiver
[ ] Bus topology
[ ] Physical cable
```

---

# 37. CAN Bus Bitrate

Common CAN bitrates include:

```text
125000
250000
500000
1000000
```

Example:

```bash
sudo ip link set can0 type can bitrate 500000
```

The supported bitrate depends on the CAN controller, transceiver,
network topology, cable characteristics, and required CAN timing.

---

# 38. CAN Test Procedure

## Step 1 — Power Off

Disconnect power before changing wiring.

## Step 2 — Connect Transceiver

```text
CAN_TX → TXD
CAN_RX → RXD
GND    → GND
VCC    → VCC
```

## Step 3 — Connect CAN Bus

```text
CANH → CANH
CANL → CANL
```

## Step 4 — Verify Termination

Install 120 Ω at each physical end of the bus.

## Step 5 — Boot Linux

## Step 6 — Check Interface

```bash
ip link show
```

## Step 7 — Configure CAN

```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up
```

## Step 8 — Start Receiver

```bash
candump can0
```

## Step 9 — Send Frame

On the other node:

```bash
cansend can0 123#11223344
```

## Step 10 — Verify

Expected:

```text
can0  123   [4]  11 22 33 44
```

---

# 39. CAN Test Checklist

```text
[ ] CAN controller enabled
[ ] CAN TX pin identified
[ ] CAN RX pin identified
[ ] Correct pinmux
[ ] CAN transceiver connected
[ ] Transceiver powered
[ ] Transceiver enabled
[ ] CANH connected
[ ] CANL connected
[ ] Common reference checked
[ ] 120 Ω termination at bus ends
[ ] can0 available
[ ] CAN bitrate configured
[ ] can0 brought UP
[ ] candump running
[ ] cansend successful
[ ] CAN frame received
[ ] Bus statistics checked
```

---

# 40. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── can_pin_map.md
│   │
│   └── wiring/
│       └── can_wiring.md
│
├── hardware/
│   └── schematics/
│       └── can/
│           └── can_test_circuit.md
│
├── device-tree/
│   └── can/
│       ├── bbb-can.dts
│       ├── bbb-can.dtsi
│       └── README.md
│
├── drivers/
│   └── can/
│       └── README.md
│
└── tests/
    └── can/
        ├── can_loopback_test.sh
        ├── can_tx_test.sh
        ├── can_rx_test.sh
        └── README.md
```

---

# 41. Complete CAN Bring-Up

```text
                     CAN Application
                            |
                            v
                         can-utils
                            |
                            v
                         SocketCAN
                            |
                            v
                       Linux CAN Core
                            |
                            v
                     DCAN Controller
                            |
                            v
                      CAN TX / RX
                            |
                            v
                     CAN Transceiver
                            |
                     +------+------+
                     |             |
                     v             v
                   CANH          CANL
                     |             |
                     +------+------+
                            |
                            v
                         CAN BUS
                            |
                            v
                     Second CAN Node
```

---

# 42. Final Objective

The objective of this wiring test is to validate the complete CAN
communication path:

```text
User Application
      ↓
SocketCAN
      ↓
Linux CAN Framework
      ↓
DCAN Driver
      ↓
AM335x DCAN Controller
      ↓
CAN TX / RX
      ↓
CAN Transceiver
      ↓
CANH / CANL
      ↓
CAN Bus
      ↓
Second CAN Node
```

The recommended hardware validation sequence is:

```text
1. Verify DCAN Device Tree
2. Verify CAN pinmux
3. Connect CAN transceiver
4. Verify CANH/CANL wiring
5. Verify 120 Ω termination
6. Configure SocketCAN
7. Start candump
8. Send frames with cansend
9. Verify received frames
10. Check CAN statistics and bus state
```

> **Important:** The BeagleBone Black DCAN controller requires an
> external CAN transceiver for connection to the physical CAN bus.
> Never connect CANH/CANL directly to the SoC's CAN TX/RX pins.

````

**File location:**

```text
beaglebone-black/hardware/wiring/can_wiring.md
````

