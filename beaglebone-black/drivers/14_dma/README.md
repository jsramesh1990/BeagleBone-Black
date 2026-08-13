# BeagleBone Black DMA Driver

## Overview

This project demonstrates the Linux DMA Engine framework using a
memory-to-memory DMA transfer.

The driver:

- Requests a DMA channel
- Allocates DMA-capable buffers
- Prepares a DMA memcpy operation
- Submits the DMA transaction
- Starts the DMA engine
- Waits for completion
- Verifies the copied data
- Releases DMA resources

---

## Directory Structure

```text
14_dma/
├── dma_driver.c
├── dma_driver.h
├── Makefile
└── README.md
DMA Architecture
                 CPU
                  |
                  |
          Prepare DMA Transfer
                  |
                  v
          Linux DMA Engine API
                  |
                  v
             DMA Channel
                  |
          +-------+-------+
          |               |
          v               v
     Source Buffer   Destination Buffer
          |               ^
          |               |
          +---- DMA ------+
                  |
                  v
             Completion
                  |
                  v
              CPU Verify
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
dma_request_chan()
     |
     v
DMA Channel
     |
     v
dma_alloc_coherent()
     |
     +----------------+
     |                |
     v                v
Source Buffer    Destination Buffer
     |                |
     +-------+--------+
             |
             v
dmaengine_prep_dma_memcpy()
             |
             v
dmaengine_submit()
             |
             v
dma_async_issue_pending()
             |
             v
DMA Transfer
             |
             v
Completion Callback
             |
             v
Data Verification
Device Tree

The driver expects a DMA channel named:

memcpy

Example structure:

dma_test {
    compatible = "bbb,dma-demo";

    dmas = <&edma0 0>;
    dma-names = "memcpy";

    status = "okay";
};

The exact dmas specification is SoC/BSP/device-tree dependent.
For the BeagleBone Black AM335x, use the DMA controller and channel
definitions supplied by your kernel's existing Device Tree rather than
copying the example blindly.

DMA Buffer

The driver allocates:

Source      = 4096 bytes
Destination = 4096 bytes

The source buffer is filled with:

0xA5

The destination buffer initially contains:

0x00

The DMA engine copies:

Source
  |
  | 4096 bytes
  v
Destination
DMA APIs Used
Request DMA Channel
dma_request_chan()

Requests a DMA channel associated with the Device Tree.

Allocate DMA Buffer
dma_alloc_coherent()

Allocates memory that can safely be accessed by both CPU and DMA.

Prepare Transfer
dmaengine_prep_dma_memcpy()

Prepares a memory-to-memory DMA transaction.

Submit Transfer
dmaengine_submit()

Places the prepared transaction into the DMA engine queue.

Start Transfer
dma_async_issue_pending()

Starts pending DMA transactions.

Completion Callback

The driver registers:

desc->callback = bbb_dma_complete_func;

When DMA completes:

DMA Complete
     |
     v
Callback
     |
     v
complete()
     |
     v
Waiting Thread Wakes
Terminate DMA

During failure or driver removal:

dmaengine_terminate_sync()

stops the active DMA operation.

Build
make

Expected module:

dma_driver.ko

Verify:

ls -l dma_driver.ko
Load Driver
sudo insmod dma_driver.ko

Check:

lsmod | grep dma_driver

Check logs:

dmesg | grep -i dma

Expected messages are similar to:

Probing BBB DMA driver
DMA channel acquired
DMA transfer started: 4096 bytes
DMA transfer completed
DMA data verification successful
DMA driver initialized successfully
Remove Driver
sudo rmmod dma_driver

Check:

lsmod | grep dma_driver
DMA Status

Check DMA class:

ls /sys/class/dma/

Depending on the kernel configuration, DMA channels may appear here.

Check DMA debug information:

cat /sys/kernel/debug/dmaengine/summary

If debugfs is not mounted:

mount -t debugfs none /sys/kernel/debug

Then:

cat /sys/kernel/debug/dmaengine/summary
Debugging

Kernel messages:

dmesg | grep -i dma

Recent messages:

dmesg | tail -30

Check DMA channels:

ls -l /sys/class/dma/

Check DMA engine summary:

cat /sys/kernel/debug/dmaengine/summary
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

Test:

make test
DMA Transfer Verification

The driver verifies the transfer using:

memcmp()

Expected:

Source Buffer
    |
    | 0xA5 0xA5 0xA5 ...
    |
    v
DMA Engine
    |
    v
Destination Buffer
    |
    | 0xA5 0xA5 0xA5 ...
    |
    v
memcmp()
    |
    v
PASS

If the buffers differ:

DMA data verification failed
CPU vs DMA

Without DMA:

CPU
 |
 +--> Read Source
 |
 +--> Copy Data
 |
 +--> Write Destination

With DMA:

CPU
 |
 +--> Configure DMA
 |
 +--> Start DMA
 |
 |       DMA Engine
 |          |
 |          +--> Read Source
 |          |
 |          +--> Write Destination
 |
 +--> Continue other work
 |
 +--> DMA Completion

DMA reduces CPU involvement for large or continuous data transfers.

DMA in Embedded Linux

Typical embedded DMA applications include:

Camera
   |
   v
CSI / ISP
   |
   v
DMA
   |
   v
DDR

Audio
   |
   v
I2S / McASP
   |
   v
DMA
   |
   v
DDR

Ethernet
   |
   v
Ethernet MAC
   |
   v
DMA
   |
   v
DDR

SPI
   |
   v
SPI Controller
   |
   v
DMA
   |
   v
DDR
Important Concepts
DMA Address

DMA devices use DMA addresses rather than normal CPU virtual
addresses.

CPU Virtual Address
        |
        v
DMA Mapping
        |
        v
DMA Address
        |
        v
DMA Controller
Cache Coherency

For non-coherent DMA systems, cache synchronization is important.

Common APIs include:

dma_sync_single_for_device()
dma_sync_single_for_cpu()

This demo uses dma_alloc_coherent(), which simplifies CPU/DMA
memory visibility.

DMA Completion

DMA is asynchronous.

Therefore:

CPU
 |
 +--> Submit DMA
 |
 +--> Continue / Wait
 |
 v
DMA Engine
 |
 v
Interrupt
 |
 v
DMA Callback
 |
 v
Transfer Complete

The driver uses a Linux completion object:

struct completion completion;

and:

wait_for_completion_timeout()

to wait for the DMA operation.

Error Handling

The driver handles:

DMA channel allocation failure
DMA buffer allocation failure
DMA descriptor preparation failure
DMA submission failure
DMA timeout
DMA data verification failure
Driver removal while DMA is active
Production Driver Flow

A production peripheral driver generally looks like:

Application
     |
     v
Kernel Driver
     |
     v
Peripheral Controller
     |
     v
DMA Engine
     |
     v
DDR Memory

For example:

Camera
   |
   v
V4L2 Driver
   |
   v
Camera Controller
   |
   v
DMA
   |
   v
DDR

or:

UART
 |
 v
UART Driver
 |
 v
DMA
 |
 v
DDR
Interview Summary

DMA stands for Direct Memory Access.

It allows a peripheral or DMA controller to transfer data between
memory and a peripheral, or between memory regions, without requiring
the CPU to copy every byte.

The Linux DMA Engine API provides a common abstraction:

dma_request_chan()
        ↓
dmaengine_prep_*
        ↓
dmaengine_submit()
        ↓
dma_async_issue_pending()
        ↓
DMA Completion Callback

The main advantages are:

Reduced CPU utilization
Higher throughput
Efficient peripheral data transfers
Better real-time performance
Useful for camera, audio, SPI, UART, Ethernet and storage
subsystems

### Final structure

```text
beaglebone-black/
└── drivers/
    └── 14_dma/
        ├── dma_driver.c
        ├── dma_driver.h
        ├── Makefile
        └── README.md

Interview flow:

Device Tree
    ↓
dma_request_chan()
    ↓
DMA Channel
    ↓
Allocate DMA Buffers
    ↓
Prepare DMA Descriptor
    ↓
dmaengine_submit()
    ↓
dma_async_issue_pending()
    ↓
DMA Hardware
    ↓
Interrupt / Callback
    ↓
Completion
    ↓
Verify Data
