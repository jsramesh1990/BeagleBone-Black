# BeagleBone Black Debugfs Driver

## Overview

This project demonstrates how to create a **debugfs interface**
from a Linux kernel module.

debugfs is a virtual filesystem specifically designed for exposing
kernel debugging and diagnostic information.

The driver creates:

```text
/sys/kernel/debug/bbb_debugfs/
├── value
├── counter
├── enable
├── message
└── status
Directory Structure
20_debugfs/
├── debugfs_driver.c
├── debugfs_driver.h
├── makefile
└── README.md
Debugfs Architecture
                    User Space
                        |
                        |
                  cat / echo
                        |
                        v
                     VFS
                        |
                        v
                    debugfs
                        |
          +-------------+-------------+
          |             |             |
          v             v             v
        value         enable       status
          |             |             |
          +-------------+-------------+
                        |
                        v
                  Kernel Driver
                        |
                        v
                  Driver State
What is debugfs?

debugfs is a Linux virtual filesystem designed primarily for
kernel developers and driver developers.

It provides a convenient way to expose:

Debug information
Driver statistics
Hardware state
Internal variables
Counters
Diagnostic information
Testing controls

It is normally mounted at:

/sys/kernel/debug
Mount debugfs

Check whether debugfs is mounted:

mount | grep debugfs

If it is not mounted:

sudo mount -t debugfs none /sys/kernel/debug

Verify:

ls /sys/kernel/debug
Driver Directory

After loading the module:

sudo insmod debugfs_driver.ko

the driver creates:

/sys/kernel/debug/bbb_debugfs/

Check:

ls -la /sys/kernel/debug/bbb_debugfs

Expected:

value
counter
enable
message
status
value

Read:

cat /sys/kernel/debug/bbb_debugfs/value

Initial value:

0

Write:

echo 100 > /sys/kernel/debug/bbb_debugfs/value

Read:

cat /sys/kernel/debug/bbb_debugfs/value

Output:

100

Every successful write increments the counter.

counter

Read:

cat /sys/kernel/debug/bbb_debugfs/counter

Example:

1

Write operations to value increment the counter.

Example:

echo 100 > /sys/kernel/debug/bbb_debugfs/value
echo 200 > /sys/kernel/debug/bbb_debugfs/value
echo 300 > /sys/kernel/debug/bbb_debugfs/value

Then:

cat /sys/kernel/debug/bbb_debugfs/counter

Output:

3
enable

Read:

cat /sys/kernel/debug/bbb_debugfs/enable

Enable:

echo 1 > /sys/kernel/debug/bbb_debugfs/enable

Read:

cat /sys/kernel/debug/bbb_debugfs/enable

Output:

1

Disable:

echo 0 > /sys/kernel/debug/bbb_debugfs/enable
message

Read:

cat /sys/kernel/debug/bbb_debugfs/message

Default:

BeagleBone Black

Write:

echo "Embedded Linux Driver" \
    > /sys/kernel/debug/bbb_debugfs/message

Read:

cat /sys/kernel/debug/bbb_debugfs/message

Output:

Embedded Linux Driver
status

The status entry provides a complete driver snapshot.

cat /sys/kernel/debug/bbb_debugfs/status

Example:

BeagleBone Black Debugfs Driver
-------------------------------
Value   : 100
Counter : 1
Enable  : 1
Message : Embedded Linux Driver
Complete Test

Load the module:

sudo insmod debugfs_driver.ko

Set value:

echo 100 > /sys/kernel/debug/bbb_debugfs/value

Enable:

echo 1 > /sys/kernel/debug/bbb_debugfs/enable

Set message:

echo "BBB Debug Test" \
    > /sys/kernel/debug/bbb_debugfs/message

Read status:

cat /sys/kernel/debug/bbb_debugfs/status

Expected:

BeagleBone Black Debugfs Driver
-------------------------------
Value   : 100
Counter : 1
Enable  : 1
Message : BBB Debug Test
Driver Flow
insmod
   |
   v
module_init()
   |
   v
debugfs_create_dir()
   |
   v
/sys/kernel/debug/bbb_debugfs
   |
   +-------+-------+-------+-------+
   |       |       |       |       |
   v       v       v       v       v
 value  counter  enable message status
   |       |       |       |       |
   +-------+-------+-------+-------+
                   |
                   v
              Kernel State
Read Flow

For:

cat /sys/kernel/debug/bbb_debugfs/value

the flow is:

User Space
    |
    v
VFS
    |
    v
debugfs
    |
    v
debugfs_value_read()
    |
    v
Driver Data
    |
    v
simple_read_from_buffer()
    |
    v
User Space
Write Flow

For:

echo 100 > /sys/kernel/debug/bbb_debugfs/value

the flow is:

User Space
    |
    v
VFS
    |
    v
debugfs
    |
    v
debugfs_value_write()
    |
    v
copy_from_user()
    |
    v
kstrtoul()
    |
    v
Driver Data
debugfs_create_dir()

The driver creates the debugfs directory using:

debugfs_root = debugfs_create_dir(
    DEBUGFS_DIR_NAME,
    NULL);

This creates:

/sys/kernel/debug/bbb_debugfs
debugfs_create_file()

Individual debugfs files are created using:

debugfs_create_file()

Example:

debugfs_create_file(
    "value",
    0644,
    debugfs_root,
    NULL,
    &debugfs_value_fops);
File Permissions

The driver uses:

value     0644
counter   0444
enable    0644
message   0644
status    0444

Meaning:

value
    Read + Write

counter
    Read only

enable
    Read + Write

message
    Read + Write

status
    Read only
simple_read_from_buffer()

The driver uses:

simple_read_from_buffer()

to simplify copying formatted kernel data to user space.

Example:

return simple_read_from_buffer(
    buffer,
    count,
    ppos,
    output,
    len);
User / Kernel Data Transfer

For writes:

copy_from_user()

is used.

For reads, the driver uses:

simple_read_from_buffer()

which handles the user-space transfer.

Flow:

User Space
    |
    | write
    v
copy_from_user()
    |
    v
Kernel Data

and:

Kernel Data
    |
    | read
    v
simple_read_from_buffer()
    |
    v
User Space
Mutex Protection

The driver has shared state:

struct bbb_debugfs_data {
    struct mutex lock;
    u32 value;
    u32 counter;
    bool enable;
    char message[128];
};

Access is protected using:

mutex_lock()
mutex_unlock()

Example:

mutex_lock(&debugfs_data.lock);

debugfs_data.value = value;
debugfs_data.counter++;

mutex_unlock(&debugfs_data.lock);

This prevents concurrent access to shared data.

Kernel Debug Messages

The driver uses:

pr_info()

for normal driver messages:

BBB debugfs: driver loaded

and:

pr_debug()

for debugging messages:

BBB debugfs: value=100

pr_debug() may require appropriate kernel dynamic-debug
configuration to be visible.

Build

Build the module:

make

Expected:

debugfs_driver.ko

Check:

ls -l debugfs_driver.ko
Load
sudo insmod debugfs_driver.ko

Check:

lsmod | grep debugfs_driver

Check kernel logs:

dmesg | tail -30
Test
make test

or manually:

ls -la /sys/kernel/debug/bbb_debugfs

cat /sys/kernel/debug/bbb_debugfs/status
Remove
sudo rmmod debugfs_driver

Check:

ls /sys/kernel/debug/bbb_debugfs

The directory should no longer exist.

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
debugfs vs procfs vs sysfs
Interface	Main Purpose
/proc	Process and kernel information
/sys	Device and kernel object attributes
/sys/kernel/debug	Kernel/driver debugging

Typical usage:

/proc
   |
   +-- CPU information
   +-- Memory information
   +-- Process information

/sys
   |
   +-- Device attributes
   +-- Hardware configuration
   +-- Device state

/sys/kernel/debug
   |
   +-- Driver debug data
   +-- Hardware diagnostics
   +-- Counters
   +-- Internal state
When to Use debugfs

Use debugfs for:

Driver debugging
Hardware diagnostics
Internal state
Debug counters
Test controls
Temporary development interfaces

Avoid using debugfs as the primary stable userspace API for a
production application.

For a permanent userspace interface, consider:

sysfs
ioctl
character device
netlink
configfs

depending on the requirement.

Embedded Linux Example

For a BeagleBone Black driver, debugfs could expose:

/sys/kernel/debug/my_driver/
├── registers
├── irq_count
├── rx_count
├── tx_count
├── error_count
├── dma_status
└── status

This is useful during board bring-up and driver debugging.

For example:

cat /sys/kernel/debug/my_driver/irq_count
cat /sys/kernel/debug/my_driver/dma_status
cat /sys/kernel/debug/my_driver/registers
Driver Debugging Flow
Hardware
   |
   v
Linux Driver
   |
   +---- IRQ Count
   |
   +---- DMA Status
   |
   +---- Register Values
   |
   +---- Error Count
   |
   v
debugfs
   |
   v
/sys/kernel/debug/
   |
   v
User Space
Real Embedded Use Cases

debugfs is especially useful for:

GPIO driver debugging
SPI driver diagnostics
I2C bus debugging
UART driver statistics
Ethernet RX/TX counters
DMA status
Interrupt counters
Camera pipeline debugging
Audio driver diagnostics
Power-management debugging
Interview Explanation

A strong interview explanation:

"debugfs is a virtual filesystem intended primarily for kernel and
driver debugging. I can create a debugfs directory using
debugfs_create_dir() and expose internal driver variables using
debugfs_create_file(). The driver implements file operations for
reading and writing debug information. Unlike sysfs, debugfs should
generally not be treated as a stable production userspace API."

Interview Flow
Kernel Module
     |
     v
debugfs_create_dir()
     |
     v
Debugfs Directory
     |
     +----------+----------+----------+
     |          |          |          |
     v          v          v          v
   value     counter     enable    status
     |          |          |          |
     +----------+----------+----------+
                |
                v
          Driver State
Key Interview Points
debugfs is a virtual filesystem.
It is mainly intended for kernel and driver debugging.
It is normally mounted at /sys/kernel/debug.
debugfs_create_dir() creates a debugfs directory.
debugfs_create_file() creates debugfs files.
struct file_operations defines file callbacks.
simple_read_from_buffer() simplifies read operations.
copy_from_user() safely receives user data.
Mutex protects shared driver state.
debugfs_remove_recursive() removes the debugfs hierarchy.
debugfs is not normally a stable userspace ABI.
sysfs is preferred for permanent device attributes.
debugfs is useful during board bring-up.
Debug counters and internal state can be exposed through debugfs.
It is commonly used when developing Linux kernel drivers.
Final Flow
                    User Space
                        |
                  cat / echo
                        |
                        v
              /sys/kernel/debug
                        |
                        v
                    debugfs
                        |
                        v
               File Operations
                        |
              +---------+---------+
              |         |         |
              v         v         v
            read      write     status
              |         |         |
              +---------+---------+
                        |
                        v
                  Kernel Driver
                        |
                        v
                Hardware / State

### Final structure

```text
beaglebone-black/
└── drivers/
    └── 20_debugfs/
        ├── debugfs_driver.c
        ├── debugfs_driver.h
        ├── makefile
        └── README.md

Core idea: sysfs → stable device attributes, procfs → kernel/process information, debugfs → driver debugging and diagnostics.
