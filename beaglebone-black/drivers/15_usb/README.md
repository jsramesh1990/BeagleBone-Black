# BeagleBone Black USB Driver

## Overview

This project demonstrates the basic structure of a Linux USB device
driver.

The driver registers itself with the Linux USB core and matches a USB
device using its:

- Vendor ID (VID)
- Product ID (PID)

When a matching USB device is connected, the USB core calls the
driver's `probe()` function.

When the device is disconnected, the USB core calls `disconnect()`.

---

## Directory Structure

```text
15_usb/
├── usb_driver.c
├── usb_driver.h
├── makefile
└── README.md
USB Driver Architecture
                 USB Device
                      |
                      v
              USB Host Controller
                      |
                      v
                   USB Core
                      |
                      v
              Device ID Matching
                      |
                      v
               usb_driver.c
                      |
             +--------+--------+
             |                 |
             v                 v
          probe()         disconnect()
             |
             v
       USB Interface
             |
             v
        Endpoints
USB Driver Flow
USB Device Connected
        |
        v
USB Host Controller
        |
        v
Linux USB Core
        |
        v
VID/PID Matching
        |
        v
bbb_usb_probe()
        |
        v
Get USB Device
        |
        v
Find Endpoints
        |
        v
Store Driver Data
        |
        v
USB Device Ready
Disconnect Flow
USB Device Removed
        |
        v
USB Core
        |
        v
bbb_usb_disconnect()
        |
        v
Release USB Device
        |
        v
Free Driver Data
USB Device ID

The driver currently uses example IDs:

#define USB_VENDOR_ID_TEST  0x1234
#define USB_PRODUCT_ID_TEST 0x5678

These are only placeholders.

Find the real device IDs using:

lsusb

Example:

Bus 001 Device 003: ID 1d6b:0002 Linux Foundation 2.0 root hub

Here:

VID = 1d6b
PID = 0002

Then update:

#define USB_VENDOR_ID_TEST  0x1d6b
#define USB_PRODUCT_ID_TEST 0x0002

For a real project, use the actual USB device's VID/PID.

USB Matching

The driver contains:

static const struct usb_device_id bbb_usb_id_table[] = {
    {
        USB_DEVICE(
            USB_VENDOR_ID_TEST,
            USB_PRODUCT_ID_TEST
        )
    },
    { }
};

The USB core compares:

USB Device VID/PID
        |
        v
USB Device ID Table
        |
        v
Match?
   /       \
 YES       NO
  |         |
  v         v
probe()   Ignore
Build

Build the driver:

make

Expected output:

usb_driver.ko

Check:

ls -l usb_driver.ko
Load Driver
sudo insmod usb_driver.ko

Check:

lsmod | grep usb_driver

Check kernel logs:

dmesg | tail -30
Connect USB Device

After loading the driver, connect the USB device that matches the
VID/PID.

Check:

lsusb

Then:

dmesg | tail -30

Expected messages are similar to:

BBB USB driver probe
Vendor ID : 0x1234
Product ID: 0x5678
Bus       : 001
Device    : 002
Bulk IN endpoint: 0x81
Bulk IN max packet: 64
Bulk OUT endpoint: 0x02
Bulk OUT max packet: 64
USB device successfully attached
USB Endpoints

USB communication occurs through endpoints.

Common endpoint types:

USB Endpoint
     |
     +---- Control
     |
     +---- Bulk
     |
     +---- Interrupt
     |
     +---- Isochronous

This demo identifies:

Bulk IN
Bulk OUT
Endpoint Direction
Bulk IN
USB Device
     |
     v
USB Host

The device sends data to the host.

Example:

USB Device ---> Linux Driver
Bulk OUT
USB Host
     |
     v
USB Device

The host sends data to the device.

Example:

Linux Driver ---> USB Device
USB Driver Data

The driver maintains:

struct bbb_usb_device

containing:

usb_device
usb_interface
Bulk IN endpoint
Bulk OUT endpoint
Endpoint packet sizes

The private data is attached to the USB interface using:

usb_set_intfdata()

It can later be retrieved using:

usb_get_intfdata()
USB APIs Used
Register Driver
usb_register()

Registers the driver with the Linux USB core.

Unregister Driver
usb_deregister()

Removes the driver from the USB core.

Get USB Device
interface_to_usbdev()

Gets the struct usb_device associated with an interface.

USB Device Reference
usb_get_dev()
usb_put_dev()

Manages the USB device reference count.

Store Private Data
usb_set_intfdata()

Stores driver-specific data with the USB interface.

Retrieve Private Data
usb_get_intfdata()

Retrieves the driver-specific data.

USB Probe

The probe function:

bbb_usb_probe()

is called when the USB core finds a matching device.

Main operations:

probe()
 |
 +--> Get USB device
 |
 +--> Read VID/PID
 |
 +--> Allocate driver data
 |
 +--> Find endpoints
 |
 +--> Store private data
 |
 +--> Initialize device
USB Disconnect

The disconnect function:

bbb_usb_disconnect()

is called when:

USB cable is removed
USB device is unplugged
Driver is unloaded
USB device is removed

The driver releases:

USB device reference
Driver memory
Private data
Debugging

Check USB devices:

lsusb

Detailed USB information:

lsusb -v

Check USB topology:

lsusb -t

Check kernel logs:

dmesg | grep -i usb

Check driver:

lsmod | grep usb_driver
USB Topology

Check:

lsusb -t

Example:

/: Bus 01.Port 1: Dev 1, Class=root_hub
    |__ Port 2: Dev 2, Class=Vendor Specific

This shows how USB devices are connected to the host controller.

Remove Driver
sudo rmmod usb_driver

Check:

lsmod | grep usb_driver

The USB core will call the driver's disconnect callback for any
currently attached matching interfaces.

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

USB information:

make info

Logs:

make logs

Test:

make test
Complete USB Architecture
                       User Application
                              |
                              v
                       USB Device Driver
                              |
                    +---------+---------+
                    |                   |
                    v                   v
                USB Bulk            USB Control
                Transfer             Transfer
                    |                   |
                    +---------+---------+
                              |
                              v
                         Linux USB Core
                              |
                              v
                    USB Host Controller
                              |
                              v
                         USB PHY
                              |
                              v
                         USB Device
USB Transfer Types
Control Transfer

Used for device configuration and management.

Host
 |
 v
USB Device

Typical operations:

Get descriptor
Set configuration
Set interface
Device control
Bulk Transfer

Used for reliable large data transfers.

Examples:

USB storage
USB Ethernet
USB serial
Data acquisition
Interrupt Transfer

Used for small periodic data.

Examples:

USB keyboard
USB mouse
HID devices
Isochronous Transfer

Used for time-sensitive streaming.

Examples:

USB audio
USB video

Isochronous transfers prioritize timing over retransmission.

USB Descriptors

USB devices provide descriptors describing their capabilities.

Basic hierarchy:

Device Descriptor
       |
       v
Configuration Descriptor
       |
       v
Interface Descriptor
       |
       v
Endpoint Descriptor

The driver can inspect these descriptors to understand the USB
device configuration.

USB Device Driver vs Platform Driver

A USB driver is different from a normal platform driver.

Platform Driver
Device Tree
     |
     v
Platform Device
     |
     v
Platform Driver
     |
     v
probe()
USB Driver
USB Device
     |
     v
USB Core
     |
     v
VID/PID Match
     |
     v
USB Driver
     |
     v
probe()

USB devices are generally discovered dynamically through the USB bus.

Production USB Driver Flow

A real USB driver commonly looks like:

USB Device
    |
    v
USB Core
    |
    v
VID/PID or Interface Matching
    |
    v
probe()
    |
    v
Allocate Driver Data
    |
    v
Find Endpoints
    |
    v
Allocate USB Buffers
    |
    v
Submit URBs
    |
    v
USB Host Controller
    |
    v
USB Device

For actual data transfer, Linux USB drivers normally use URBs
(USB Request Blocks).

Typical APIs include:

usb_alloc_urb()
usb_fill_bulk_urb()
usb_submit_urb()
usb_kill_urb()
usb_free_urb()

This demo intentionally stops at device detection and endpoint
enumeration.

Interview Summary

A Linux USB driver is registered with the USB core using:

usb_register()

The driver provides:

usb_driver
    |
    +---- name
    +---- id_table
    +---- probe
    +---- disconnect

The USB core matches the device using the driver's ID table.

The basic flow is:

USB Enumeration
      ↓
VID/PID Matching
      ↓
probe()
      ↓
Endpoint Discovery
      ↓
URB Allocation
      ↓
usb_submit_urb()
      ↓
USB Transfer
      ↓
Completion Callback

For an embedded Linux engineer, the important concepts are:

USB Host Controller
USB Core
USB Device
USB Interface
USB Endpoint
USB Descriptor
USB Driver
URB
Bulk/Interrupt/Isochronous transfers
probe() / disconnect() lifecycle

### Final structure

```text
beaglebone-black/
└── drivers/
    └── 15_usb/
        ├── usb_driver.c
        ├── usb_driver.h
        ├── makefile
        └── README.md

Core USB flow to remember:

USB Device
    ↓
USB Host Controller
    ↓
USB Core
    ↓
VID/PID Match
    ↓
probe()
    ↓
Find Interface/Endpoints
    ↓
Allocate URB
    ↓
usb_submit_urb()
    ↓
USB Transfer
    ↓
Completion Callback
    ↓
disconnect()

Important: 0x1234:0x5678 in the code is only a placeholder. Replace it with the actual VID/PID from lsusb for the USB device you want your BeagleBone Black driver to handle.
