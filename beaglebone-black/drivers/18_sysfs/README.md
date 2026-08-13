# BeagleBone Black Sysfs Driver

## Overview

This project demonstrates how a Linux kernel platform driver can
create and expose device configuration and status through **sysfs**.

The driver creates three attributes:

```text
value
enable
status

The attributes allow user space to read and modify driver state
without creating a custom character device.

Directory Structure
18_sysfs/
├── sysfs_driver.c
├── sysfs_driver.h
├── makefile
└── README.md
Sysfs Architecture
                  User Space
                      |
                      v
              /sys filesystem
                      |
              +-------+-------+
              |       |       |
              v       v       v
            value   enable  status
              |       |       |
              +-------+-------+
                      |
                      v
                 Sysfs Core
                      |
                      v
              Linux Kernel Driver
                      |
                      v
                Device State
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
Allocate Private Data
     |
     v
Initialize Driver State
     |
     v
Create Sysfs Group
     |
     v
/sys/.../value
/sys/.../enable
/sys/.../status
Sysfs Location

After loading the driver, the attributes are created under the
platform device's sysfs directory.

Find them using:

find /sys/bus/platform/devices \
    -type f \
    \( -name value -o -name enable -o -name status \)

The exact device path depends on how the platform device is created
and matched.

Sysfs Attributes
value

Read:

cat /sys/.../value

Write:

echo 100 > /sys/.../value

The driver receives the write through:

value_store()
enable

Read:

cat /sys/.../enable

Enable:

echo 1 > /sys/.../enable

Disable:

echo 0 > /sys/.../enable

The driver receives the operation through:

enable_store()
status

Read:

cat /sys/.../status

Example:

value=100 enable=1

The driver generates this through:

status_show()
Sysfs Read Flow
User
 |
 | cat /sys/.../value
 |
 v
VFS
 |
 v
Sysfs
 |
 v
value_show()
 |
 v
Driver Private Data
 |
 v
User
Sysfs Write Flow
User
 |
 | echo 100 > /sys/.../value
 |
 v
VFS
 |
 v
Sysfs
 |
 v
value_store()
 |
 v
kstrtoint()
 |
 v
Driver State
DEVICE_ATTR

The driver uses:

DEVICE_ATTR_RW(value);

This creates:

value_show()
value_store()

Similarly:

DEVICE_ATTR_RW(enable);

creates:

enable_show()
enable_store()

And:

DEVICE_ATTR_RO(status);

creates a read-only attribute:

status_show()
Attribute Permissions

The standard macros provide appropriate sysfs permissions.

Conceptually:

value
  |
  +-- read
  +-- write

enable
  |
  +-- read
  +-- write

status
  |
  +-- read only
Attribute Group

The attributes are grouped using:

struct attribute_group

The group contains:

value
enable
status

and is created using:

sysfs_create_group()

It is removed using:

sysfs_remove_group()
Driver Private Data

The driver maintains:

struct bbb_sysfs_priv

with:

device pointer
mutex
value
enable

Example:

bbb_sysfs_priv
       |
       +-- dev
       |
       +-- lock
       |
       +-- value
       |
       +-- enable
Mutex Protection

Sysfs callbacks can be accessed from user space while other kernel
operations may also access the same data.

The driver therefore protects shared state using:

mutex_lock()
mutex_unlock()

Example:

mutex_lock(&priv->lock);

priv->value = value;

mutex_unlock(&priv->lock);

This prevents concurrent access problems.

Build

Build the module:

make

Expected output:

sysfs_driver.ko

Check:

ls -l sysfs_driver.ko
Load
sudo insmod sysfs_driver.ko

Check:

lsmod | grep sysfs_driver

Check logs:

dmesg | tail -30
Find Sysfs Attributes
find /sys/bus/platform/devices \
    -type f \
    \( -name value -o -name enable -o -name status \)

You can also search globally:

find /sys -type f -name "value" 2>/dev/null
Test value

Locate the attribute first:

find /sys -type f -name "value" 2>/dev/null

Then:

cat /sys/.../value

Write:

echo 123 > /sys/.../value

Read again:

cat /sys/.../value

Expected:

123
Test enable

Read:

cat /sys/.../enable

Enable:

echo 1 > /sys/.../enable

Verify:

cat /sys/.../enable

Expected:

1

Disable:

echo 0 > /sys/.../enable
Test status
cat /sys/.../status

Example:

value=123 enable=0
Kernel Logs

Check:

dmesg | grep -i sysfs

When writing a value:

echo 500 > /sys/.../value

the driver prints:

sysfs value updated: 500
Remove Driver
sudo rmmod sysfs_driver

The driver removes its sysfs group during:

bbb_sysfs_remove()
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
Device Tree

This driver contains the following compatible string:

bbb,sysfs-demo

A platform device can be described in Device Tree as:

sysfs_demo {
    compatible = "bbb,sysfs-demo";
    status = "okay";
};

The kernel matches this with:

static const struct of_device_id bbb_sysfs_of_match[]
Device Tree Flow
Device Tree
     |
     | compatible =
     | "bbb,sysfs-demo"
     |
     v
Platform Device
     |
     v
Platform Driver
     |
     v
probe()
     |
     v
Create Sysfs Group
     |
     v
Sysfs Attributes
Sysfs vs Procfs
Sysfs

Used primarily for exposing device and kernel object attributes.

/sys

Examples:

/sys/class
/sys/devices
/sys/bus
/sys/block
Procfs

Primarily provides process and kernel runtime information.

/proc

Examples:

/proc/cpuinfo
/proc/meminfo
/proc/interrupts
/proc/modules
Sysfs vs Character Device
Sysfs

Good for:

Configuration
Simple status
Device parameters
Enable/disable controls

Example:

echo 1 > /sys/.../enable
Character Device

Better for:

Streaming data
Complex commands
Large data transfers
read()
write()
ioctl()
poll()

Example:

/dev/mydevice
Real Embedded Use Cases

Sysfs is commonly useful for exposing:

Device enable
Device status
Mode selection
Debug configuration
Power state
Threshold
Timeout
Frequency
Simple hardware controls

Example:

/sys/class/mydevice/mydevice0/
├── enable
├── status
├── mode
├── threshold
└── value
Important Kernel APIs

Sysfs-related APIs used in this driver:

device_create_file()
device_remove_file()

sysfs_create_group()
sysfs_remove_group()

dev_get_drvdata()
platform_set_drvdata()

The demo uses the attribute-group approach.

Driver Flow for Interview
Platform Driver
      |
      v
probe()
      |
      v
Allocate Private Data
      |
      v
Initialize Mutex
      |
      v
Create Attribute Group
      |
      v
Sysfs Entries
      |
      +--------+--------+
      |        |        |
      v        v        v
    value    enable   status
      |        |        |
      +--------+--------+
               |
               v
         Driver State
Interview Explanation

A strong explanation is:

"Sysfs provides a kernel-to-user-space interface for exposing
device attributes. In this driver I create a platform driver,
allocate private data, create a sysfs attribute group, and
implement show/store callbacks. User space can read or modify
driver parameters using standard file operations such as cat and
echo. Shared data is protected using a mutex."

Important Concepts
Sysfs
struct device
device_attribute
DEVICE_ATTR
attribute_group
sysfs_create_group
sysfs_remove_group
show() callback
store() callback
Platform driver
Device Tree
dev_get_drvdata()
platform_set_drvdata()
Mutex
User-space/kernel-space interface
Final Flow
                User Space
                    |
             cat / echo
                    |
                    v
                  Sysfs
                    |
                    v
             show()/store()
                    |
                    v
             Driver Private Data
                    |
                    v
              Hardware/State

Sysfs is best suited for simple configuration and status
attributes, not for high-throughput data transfer.


### Final structure

```text
beaglebone-black/
└── drivers/
    └── 18_sysfs/
        ├── sysfs_driver.c
        ├── sysfs_driver.h
        ├── makefile
        └── README.md

Core flow:

Device Tree
    ↓
Platform Driver
    ↓
probe()
    ↓
sysfs_create_group()
    ↓
/sys/.../value
/sys/.../enable
/sys/.../status
    ↓
show()/store()
    ↓
Driver State / Hardware
