# `can_test_circuit.md`

````markdown
# BeagleBone Black CAN Test Circuit

## 1. Overview

This document describes a basic hardware test circuit for validating
CAN communication on the BeagleBone Black.

The BeagleBone Black AM335x processor provides CAN controller hardware,
but a **CAN transceiver is required** to convert the controller's
logic-level TX/RX signals into the differential CANH/CANL bus signals.

The complete path is:

```text
BeagleBone Black
      |
      v
AM335x CAN Controller
      |
      v
Linux CAN Driver
      |
      v
CAN TX / CAN RX
      |
      v
CAN Transceiver
      |
      v
CANH / CANL
      |
      v
CAN Bus
      |
      v
Second CAN Node
````

---

# 2. Important Hardware Requirement

Do **not** connect the BeagleBone Black CAN TX/RX pins directly to
CANH/CANL.

A CAN transceiver is required.

Example transceivers:

```text
MCP2551
MCP2562
SN65HVD230
SN65HVD232
```

The transceiver must be compatible with the BeagleBone Black's
logic-voltage requirements.

---

# 3. CAN Test Architecture

```text
             BeagleBone Black
             +-------------+
             |             |
             |  CAN TX     |
             |     |       |
             |     v       |
             | CAN        |
             | Controller |
             |     |       |
             | CAN RX     |
             |     |       |
             +-----+-------+
                   |
             +-----+------+
             | CAN        |
             | Transceiver|
             +-----+------+
                   |
             +-----+------+
             |            |
            CANH         CANL
             |            |
             +-----+------+
                   |
             CAN Bus Cable
                   |
             +-----+------+
             |            |
          CANH          CANL
             |            |
       +-----+------------+-----+
       | CAN Transceiver        |
       +-----------+------------+
                   |
             CAN Controller
                   |
             Second CAN Node
```

---

# 4. Required Components

```text
1 × BeagleBone Black
1 × CAN transceiver
1 × Second CAN node
120 Ω termination resistors
CAN twisted-pair cable
Jumper wires
Breadboard or suitable CAN interface board
```

For a two-node test:

```text
Node 1 = BeagleBone Black
Node 2 = USB-CAN adapter / second development board
```

---

# 5. BeagleBone CAN Connections

The exact CAN header pins should be verified using:

```text
hardware/pinout/can_pin_map.md
```

The logical signals are:

```text
CAN_TX
CAN_RX
```

Connect them to the corresponding transceiver pins:

```text
BeagleBone CAN_TX
       |
       v
Transceiver TXD

BeagleBone CAN_RX
       |
       v
Transceiver RXD
```

---

# 6. CAN Transceiver Connections

Generic transceiver:

```text
                    CAN Transceiver
                 +-------------------+
BBB CAN_TX ----->| TXD               |
                 |                   |
BBB CAN_RX <-----| RXD               |
                 |                   |
3.3V ----------->| VCC               |
GND ------------>| GND               |
                 |                   |
                 | CANH ------------+---- CANH
                 |                   |
                 | CANL ------------+---- CANL
                 +-------------------+
```

The exact pin names depend on the selected transceiver.

---

# 7. Complete Two-Node Circuit

```text
             NODE 1                         NODE 2

       BeagleBone Black                 CAN Device
       +-------------+                 +-------------+
       |             |                 |             |
       | CAN_TX -----|----> TXD        |             |
       |             |    Transceiver  |             |
       | CAN_RX <----|---- RXD         |             |
       |             |                 |             |
       +-------------+                 +-------------+
              |                               |
              v                               v
        +-----------+                   +-----------+
        | CAN       |                   | CAN       |
        |Transceiver|                   |Transceiver|
        +-----+-----+                   +-----+-----+
              |                               |
             CANH=============================CANH
              |                               |
             CANL=============================CANL
              |                               |
              +-------------------------------+
```

---

# 8. CANH and CANL

CAN communication uses differential signaling.

```text
CANH
  |
  |============================|
  |
CANL
  |
  |============================|
```

The receiver determines the bus state from the voltage difference
between CANH and CANL.

Typical concept:

```text
Recessive:
CANH ≈ CANL

Dominant:
CANH > CANL
```

The exact voltages depend on the transceiver and bus conditions.

---

# 9. CAN Termination

A CAN bus normally requires a termination resistor at each physical
end of the bus.

Typical arrangement:

```text
             120 Ω
        +----/\/\/\----+
        |              |
CANH ---+--------------+----------------+
                                         |
                                         |
CANL ---+--------------------------------+
        |                                |
        +----/\/\/\----------------------+
             120 Ω
```

More clearly:

```text
Node 1                                  Node 2

CANH ==================================== CANH
       |                              |
      120Ω                           120Ω
       |                              |
CANL ==================================== CANL
```

Only the two physical ends of the CAN bus should normally be terminated.

---

# 10. Two-Node Test

Recommended test:

```text
BeagleBone Black
       |
       v
CAN Transceiver
       |
       |
     CAN Bus
       |
       v
USB-CAN Adapter
       |
       v
PC
```

This allows the PC to monitor CAN frames.

---

# 11. CAN Bus Wiring

```text
BBB CAN Transceiver
       |
       +------ CANH =================== CANH ------+
       |                                            |
       +------ CANL =================== CANL ------+
                                                    |
                                             USB-CAN Adapter
```

Also connect the grounds where required by the transceiver/interface
design:

```text
BBB GND -------------------- USB-CAN GND
```

---

# 12. CAN Power

Typical structure:

```text
BBB 3.3V
   |
   v
CAN Transceiver VCC

BBB GND
   |
   v
CAN Transceiver GND
```

Verify the selected transceiver's supply-voltage requirements before
connecting it.

---

# 13. CAN Device Tree Flow

The CAN controller must be enabled through Device Tree.

Project files:

```text
beaglebone-black/
└── device-tree/
    └── can/
        ├── bbb-can.dts
        ├── bbb-can.dtsi
        └── README.md
```

Flow:

```text
bbb-can.dts
      |
      v
Pinmux Configuration
      |
      v
CAN Controller Enabled
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

# 14. Linux CAN Architecture

Linux uses **SocketCAN** for CAN networking.

```text
User Application
       |
       v
   SocketCAN
       |
       v
   CAN Network
   Interface
      can0
       |
       v
 Linux CAN Driver
       |
       v
 AM335x CAN Controller
       |
       v
 CAN Transceiver
       |
       v
 CAN Bus
```

---

# 15. Check CAN Interface

After booting:

```bash
ip link
```

Look for:

```text
can0
```

Alternatively:

```bash
ip -details link show can0
```

---

# 16. Configure CAN Bitrate

Example:

```bash
sudo ip link set can0 down
```

Configure:

```bash
sudo ip link set can0 type can bitrate 500000
```

Bring it up:

```bash
sudo ip link set can0 up
```

Check:

```bash
ip -details link show can0
```

This example configures:

```text
CAN bitrate = 500 kbit/s
```

Both CAN nodes must use compatible bus timing.

---

# 17. CAN Interface Status

Check:

```bash
ip -details link show can0
```

Look for information such as:

```text
state
bitrate
sample-point
restart-ms
berr-counter
```

A healthy interface should normally show an operational state after
being brought up and connected to a functioning CAN bus.

---

# 18. Install CAN Utilities

On Debian/Ubuntu-based systems:

```bash
sudo apt update
sudo apt install can-utils
```

Important utilities include:

```text
candump
cansend
cangen
canplayer
cansniffer
```

---

# 19. CAN Receive Test

Start the CAN receiver:

```bash
candump can0
```

Example:

```text
can0  123   [2]  11 22
```

Meaning:

```text
Interface : can0
CAN ID    : 0x123
DLC       : 2
Data      : 11 22
```

---

# 20. CAN Transmit Test

Send a CAN frame:

```bash
cansend can0 123#1122
```

This sends:

```text
CAN ID = 0x123
DLC    = 2
DATA   = 11 22
```

Another node running:

```bash
candump can0
```

should receive the frame.

---

# 21. CAN Loopback Test

Linux CAN supports software loopback.

Conceptually:

```text
Application
     |
     v
SocketCAN
     |
     v
can0
     |
     +----------+
                |
                v
             Loopback
                |
                v
             can0 RX
```

Enable:

```bash
sudo ip link set can0 type can bitrate 500000 loopback on
```

Bring interface up:

```bash
sudo ip link set can0 up
```

Then:

```bash
candump can0
```

In another terminal:

```bash
cansend can0 123#DEADBEEF
```

The frame can be observed through the SocketCAN interface.

> Software loopback validates the Linux CAN stack but does **not**
> validate the physical CAN transceiver, CANH/CANL wiring, or bus
> termination.

---

# 22. Physical CAN Test

For a real hardware test:

```text
               BeagleBone Black
                      |
                      v
                CAN Controller
                      |
                      v
                CAN Transceiver
                      |
                      v
                    CANH
                      |
                      +================+
                                       |
                                       v
                                   CAN Node 2
                                       |
                                       ^
                      +================+
                      |
                    CANL
```

Test sequence:

```text
1. Connect CAN transceiver
2. Connect CANH
3. Connect CANL
4. Add termination
5. Configure bitrate
6. Bring can0 up
7. Start candump
8. Send cansend frame
9. Verify received frame
```

---

# 23. CAN Bus Bitrate

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

All nodes must use compatible CAN timing.

---

# 24. CAN Error Monitoring

Check interface statistics:

```bash
ip -s -d link show can0
```

Look for:

```text
RX errors
TX errors
restarts
bus-off
```

CAN errors may indicate:

```text
Incorrect bitrate
Missing termination
Incorrect wiring
CANH/CANL reversed
Faulty transceiver
Electrical noise
No active CAN node
```

---

# 25. CAN Bus-Off

If the CAN controller detects excessive errors, it may enter:

```text
BUS-OFF
```

Check:

```bash
ip -details link show can0
```

Possible recovery configuration:

```bash
sudo ip link set can0 type can bitrate 500000 restart-ms 100
```

Then:

```bash
sudo ip link set can0 up
```

The exact recovery strategy should be selected according to the
application requirements.

---

# 26. CANH/CANL Wiring Problem

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

---

# 27. CAN Termination Check

With the bus powered down, a typical two-terminated CAN bus can measure
approximately:

```text
CANH ↔ CANL ≈ 60 Ω
```

because two 120 Ω termination resistors are effectively in parallel.

```text
120 Ω || 120 Ω ≈ 60 Ω
```

This is a useful physical wiring check, but the actual measurement
depends on other equipment connected to the bus.

---

# 28. Logic Analyzer / Oscilloscope

For physical debugging, observe:

```text
CANH
CANL
```

An oscilloscope is generally more useful than a basic logic analyzer
for examining the differential CAN physical layer.

Check:

```text
1. CANH waveform
2. CANL waveform
3. Differential voltage
4. Noise
5. Ringing
6. Signal integrity
7. Bus termination
```

---

# 29. CAN Test Flow

```text
Hardware Wiring
      |
      v
CAN Transceiver
      |
      v
CANH / CANL
      |
      v
Termination
      |
      v
Device Tree
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
      +----------+
      |          |
      v          v
  candump    cansend
      |          |
      +-----+----+
            |
            v
       CAN Bus Test
```

---

# 30. CAN Debugging

If `can0` does not appear:

```text
CAN0 Missing
     |
     +--> Check Device Tree
     |
     +--> Check CAN driver
     |
     +--> Check pinmux
     |
     +--> Check kernel logs
     |
     +--> Check network interface
```

Check:

```bash
dmesg | grep -i can
```

and:

```bash
ip link
```

---

# 31. If CAN Interface Is Up but No Frames

Check:

```text
[ ] CANH connected
[ ] CANL connected
[ ] GND/reference connected as required
[ ] Transceiver powered
[ ] CANH/CANL not reversed
[ ] 120 Ω termination at bus ends
[ ] Same bitrate on both nodes
[ ] Second CAN node active
[ ] Correct CAN ID
[ ] CAN interface UP
```

---

# 32. CAN Test Commands

### Bring interface down

```bash
sudo ip link set can0 down
```

### Configure bitrate

```bash
sudo ip link set can0 type can bitrate 500000
```

### Bring interface up

```bash
sudo ip link set can0 up
```

### Check interface

```bash
ip -details link show can0
```

### Receive

```bash
candump can0
```

### Transmit

```bash
cansend can0 123#11223344
```

### Generate traffic

```bash
cangen can0
```

### Stop interface

```bash
sudo ip link set can0 down
```

---

# 33. Example CAN Test

Terminal 1:

```bash
candump can0
```

Terminal 2:

```bash
cansend can0 123#DEADBEEF
```

Expected:

```text
can0  123   [4]  DE AD BE EF
```

This confirms that a CAN frame was transmitted and observed by the
SocketCAN receive path.

---

# 34. CAN Application Architecture

A user-space CAN application can use Linux SocketCAN:

```text
+---------------------------+
| User Application          |
|                           |
| socket(AF_CAN, ...)       |
| bind()                    |
| write()/send()            |
| read()/recv()             |
+-------------+-------------+
              |
              v
        SocketCAN
              |
              v
           can0
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

---

# 35. Hardware-to-Software Validation

The complete test should be performed in stages.

### Stage 1 — Linux Interface

```bash
ip link
```

Verify:

```text
can0
```

### Stage 2 — CAN Configuration

```bash
ip -details link show can0
```

Verify:

```text
bitrate
state
```

### Stage 3 — Software Loopback

```bash
candump can0
```

and:

```bash
cansend can0 123#1122
```

### Stage 4 — Physical Bus

Connect:

```text
CANH
CANL
Termination
Second CAN Node
```

### Stage 5 — Physical Frame Test

Send:

```bash
cansend can0 123#11223344
```

Receive:

```bash
candump can0
```

---

# 36. CAN Test Checklist

```text
[ ] CAN controller enabled
[ ] CAN Device Tree configured
[ ] CAN pinmux configured
[ ] CAN driver loaded
[ ] can0 detected
[ ] CAN bitrate configured
[ ] can0 brought UP
[ ] SocketCAN working
[ ] Software loopback tested
[ ] CAN transceiver connected
[ ] CANH connected
[ ] CANL connected
[ ] CAN termination checked
[ ] Second CAN node connected
[ ] Physical CAN frame transmitted
[ ] Physical CAN frame received
[ ] CAN errors checked
[ ] Bus-off behavior tested
```

---

# 37. Project Integration

```text
beaglebone-black/
│
├── hardware/
│   ├── pinout/
│   │   └── can_pin_map.md
│   │
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

# 38. Complete CAN Bring-Up

```text
                CAN Hardware
                     |
                     v
               CAN Pin Map
                     |
                     v
              Test Circuit
                     |
                     v
              CAN Transceiver
                     |
                     v
                 CANH/CANL
                     |
                     v
                Termination
                     |
                     v
                Device Tree
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
             +-------+-------+
             |               |
             v               v
          candump         cansend
             |               |
             +-------+-------+
                     |
                     v
                CAN Bus Test
```

---

# 39. Final Objective

The purpose of this circuit is to validate the complete BeagleBone Black
CAN communication path:

```text
CAN Application
      ↓
SocketCAN
      ↓
Linux CAN Driver
      ↓
AM335x CAN Controller
      ↓
CAN Transceiver
      ↓
CANH / CANL
      ↓
CAN Bus
      ↓
Second CAN Node
```

The **software loopback test** validates the Linux CAN/SocketCAN path,
while the **two-node physical test** validates the complete hardware
path including the transceiver, CANH/CANL wiring, termination, and bus
communication.

````

**File:**

```text
beaglebone-black/hardware/schematics/can/can_test_circuit.md
````

