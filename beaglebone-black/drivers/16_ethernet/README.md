# BeagleBone Black Ethernet Driver

## Overview

This project demonstrates the Linux Ethernet networking driver
framework using `struct net_device`.

The driver demonstrates:

- Linux network device registration
- `net_device`
- `net_device_ops`
- Interface open/close
- Packet transmission callback
- Network statistics
- MAC address configuration
- Ethtool driver information
- Platform driver integration

> **Important:** This is a framework/demo driver. It does not
> directly control the BeagleBone Black AM335x CPSW Ethernet
> hardware.

---

## Directory Structure

```text
16_ethernet/
├── ethernet_demo.c
├── ethernet_demo.h
├── makefile
└── README.md
Ethernet Driver Architecture
                  User Space
                      |
          +-----------+-----------+
          |                       |
          v                       v
       ip / ping             Applications
          |                       |
          +-----------+-----------+
                      |
                      v
                Linux Network
                   Stack
                      |
                      v
                net_device
                      |
                      v
             Ethernet Driver
                      |
                      v
              DMA TX/RX Rings
                      |
                      v
              Ethernet MAC
                      |
                      v
                  PHY
                      |
                      v
              Ethernet Cable
Driver Flow
Device Tree
     |
     v
Platform Device
     |
     v
probe()
     |
     v
alloc_etherdev()
     |
     v
Configure net_device
     |
     v
register_netdev()
     |
     v
ethX interface
     |
     v
Network Stack
     |
     v
ndo_start_xmit()
     |
     v
TX DMA
     |
     v
Ethernet MAC
Probe Flow

The platform driver's probe() function:

probe()
  |
  +--> Allocate net_device
  |
  +--> Configure Ethernet device
  |
  +--> Initialize private data
  |
  +--> Set MAC address
  |
  +--> register_netdev()
  |
  +--> Network interface available
Network Device

Linux represents a network interface using:

struct net_device

It contains information about:

Interface name
MAC address
MTU
Network operations
Device state
Statistics
Hardware capabilities

Example:

eth0
 |
 +-- MAC address
 +-- MTU
 +-- net_device_ops
 +-- Statistics
Network Device Operations

The driver implements:

struct net_device_ops

Important callbacks:

ndo_open
ndo_stop
ndo_start_xmit
ndo_get_stats64
ndo_set_mac_address
Interface Open

When the interface is brought up:

ip link set eth0 up

Linux calls:

ndo_open()

The demo executes:

netif_start_queue()

Flow:

ip link set eth0 up
        |
        v
ndo_open()
        |
        v
netif_start_queue()
        |
        v
Interface Ready
Interface Stop

When:

ip link set eth0 down

Linux calls:

ndo_stop()

The driver executes:

netif_stop_queue()
Packet Transmission

When the Linux network stack has a packet to transmit:

Application
     |
     v
Socket
     |
     v
TCP/IP
     |
     v
Network Stack
     |
     v
ndo_start_xmit()

The driver receives:

struct sk_buff *skb

The skb contains the network packet.

SKB

Linux uses:

struct sk_buff

to represent network packets.

Basic flow:

Application
     |
     v
Socket
     |
     v
SKB
     |
     v
Ethernet Driver
     |
     v
Hardware

The driver gets the packet length using:

skb->len
Real Hardware TX Flow

A production Ethernet driver normally does:

ndo_start_xmit()
       |
       v
Get skb
       |
       v
DMA map
       |
       v
TX descriptor
       |
       v
Ethernet MAC
       |
       v
PHY
       |
       v
Ethernet Cable

The demo driver does not perform the actual DMA operation.

It records the packet and releases the skb.

RX Flow

A real Ethernet driver receives packets through RX DMA descriptors.

Ethernet Cable
       |
       v
PHY
       |
       v
Ethernet MAC
       |
       v
RX DMA
       |
       v
RX Descriptor
       |
       v
NAPI Poll
       |
       v
Allocate / build SKB
       |
       v
Network Stack
       |
       v
Application

Typical production drivers use NAPI for efficient receive
processing.

TX/RX DMA

Ethernet controllers normally use DMA.

                    CPU
                     |
                     v
              Ethernet Driver
                /          \
               /            \
              v              v
          TX Ring         RX Ring
              |              |
              v              v
          DMA Engine     DMA Engine
              |              |
              v              v
         Ethernet MAC <----> Ethernet MAC

DMA reduces CPU overhead for packet movement between DDR and the
Ethernet controller.

MAC Address

The demo uses a locally administered MAC address:

02:00:00:BB:00:01

Check it using:

ip link show

Example:

eth0: <BROADCAST,MULTICAST>
    link/ether 02:00:00:bb:00:01

In a production driver, the MAC address should normally come from:

EEPROM
OTP
Device Tree
Bootloader
Hardware configuration
SoC/board-specific storage
Change MAC Address

The driver implements:

ndo_set_mac_address

A MAC can be changed from user space:

ip link set eth0 down
ip link set eth0 address 02:11:22:33:44:55
ip link set eth0 up

Verify:

ip link show eth0
Network Statistics

Check:

ip -s link

Typical counters include:

RX packets
RX bytes
RX errors
TX packets
TX bytes
TX errors

The driver implements:

ndo_get_stats64()
Ethtool

The driver provides basic ethtool information.

Check:

ethtool -i eth0

Example information:

driver: bbb_ethernet_demo
version: 1.0
bus-info: platform
Build

Build the module:

make

Expected:

ethernet_demo.ko

Check:

ls -l ethernet_demo.ko
Load Driver
sudo insmod ethernet_demo.ko

Check:

lsmod | grep ethernet_demo

Check logs:

dmesg | tail -30
Check Network Interface
ip link show

The demo registers a network interface, normally with a name such
as:

eth0

or another available interface name.

Bring Interface Up
sudo ip link set eth0 up

Check:

ip link show eth0

Expected state:

UP
Bring Interface Down
sudo ip link set eth0 down
Network Statistics
ip -s link show eth0

Example:

RX:
    packets
    bytes
    errors

TX:
    packets
    bytes
    errors
Test Packet Transmission

The demo does not have real hardware TX.

However, the network stack can still call:

ndo_start_xmit()

when packets are queued for the interface.

For example:

ping -I eth0 192.168.1.1

The driver records the transmitted packets.

Check:

ip -s link show eth0
Debug Logs

Check driver messages:

dmesg | grep -i ethernet

Check all network messages:

dmesg | grep -i net

Follow kernel messages:

dmesg -w
Remove Driver

First bring the interface down:

sudo ip link set eth0 down

Then:

sudo rmmod ethernet_demo

Check:

lsmod | grep ethernet_demo
Makefile Commands

Build:

make

Clean:

make clean

Load:

make load

Unload:

make unload

Status:

make status

Logs:

make logs

Interface information:

make info

Test:

make test
Ethernet Hardware Architecture

For the actual BeagleBone Black:

             AM335x SoC
                 |
                 v
           CPSW Ethernet
             Controller
                 |
          +------+------+
          |             |
         MAC           DMA
          |             |
          +------+------+
                 |
                 v
                PHY
                 |
                 v
              RJ45
                 |
                 v
             Ethernet
              Network

The AM335x contains the Ethernet CPSW subsystem.

A production Linux Ethernet driver must interact with the actual
CPSW controller rather than simply registering a dummy net_device.

Production Ethernet Driver

A real embedded Ethernet driver generally contains:

                    Device Tree
                         |
                         v
                      probe()
                         |
             +-----------+-----------+
             |           |           |
             v           v           v
           MAC          PHY         DMA
          Setup        Setup        Setup
             |           |           |
             +-----------+-----------+
                         |
                         v
                  Register netdev
                         |
                         v
                       eth0
                         |
                +--------+--------+
                |                 |
                v                 v
               TX                RX
                |                 |
                v                 v
             DMA Ring          DMA Ring
                |                 |
                v                 v
             CPSW MAC          CPSW MAC
PHY Handling

A production driver normally communicates with the Ethernet PHY
through MDIO.

Ethernet Driver
       |
       v
      MDIO
       |
       v
      PHY
       |
       +--> Link Speed
       |
       +--> Duplex
       |
       +--> Auto Negotiation
       |
       +--> Link Status

Typical speeds:

10 Mbps
100 Mbps
1000 Mbps
NAPI

Linux Ethernet drivers normally use NAPI for RX processing.

Flow:

Packet Received
      |
      v
Ethernet MAC
      |
      v
RX Interrupt
      |
      v
Disable / Reduce Interrupts
      |
      v
NAPI Poll
      |
      v
Process RX Packets
      |
      v
Network Stack

NAPI combines interrupt notification with polling to improve
performance under heavy traffic.

Important Ethernet Driver Concepts
struct net_device
net_device_ops
sk_buff
TX/RX descriptor rings
DMA
NAPI
PHY
MDIO
MAC address
Link speed and duplex
Interrupt handling
Packet statistics
ethtool
Device Tree
Network stack integration
Interview Flow

A strong Ethernet driver explanation is:

Device Tree
    ↓
Ethernet MAC/CPSW Probe
    ↓
Initialize Hardware
    ↓
Initialize DMA TX/RX Rings
    ↓
Initialize PHY through MDIO
    ↓
Register net_device
    ↓
eth0 appears
    ↓
ndo_open()
    ↓
Enable RX/TX
    ↓
TX → skb → DMA → MAC → PHY
    ↓
RX → PHY → MAC → DMA → NAPI → skb
    ↓
Linux Network Stack
Important Note

This project demonstrates the Linux networking framework, not
the complete AM335x CPSW hardware implementation.

For a production BeagleBone Black Ethernet driver, the existing
Linux CPSW driver should normally be extended/configured rather than
creating an independent replacement driver.

The production driver must handle:

CPSW registers
DMA descriptors
TX/RX rings
Interrupts
NAPI
PHY/MDIO
Link state
DMA mapping
Packet buffers
Error recovery
Power management
Device Tree
Runtime configuration

### Final structure

```text
beaglebone-black/
└── drivers/
    └── 16_ethernet/
        ├── ethernet_demo.c
        ├── ethernet_demo.h
        ├── makefile
        └── README.md
Core Ethernet flow
Application
    ↓
Socket
    ↓
TCP/IP Stack
    ↓
net_device
    ↓
ndo_start_xmit()
    ↓
SKB
    ↓
DMA TX Ring
    ↓
CPSW Ethernet MAC
    ↓
PHY
    ↓
RJ45 / Network

For the real BeagleBone Black, the important hardware block is the AM335x CPSW Ethernet subsystem. The demo above intentionally stays at the net_device framework level; a real CPSW driver needs DMA descriptors, NAPI, interrupts, MDIO/PHY handling, and hardware register programming.
