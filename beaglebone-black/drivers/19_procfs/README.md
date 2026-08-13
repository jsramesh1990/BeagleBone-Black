# BeagleBone Black Procfs Driver

## Overview

This project demonstrates how to create a **procfs interface**
from a Linux kernel module.

The driver creates:

```text
/proc/bbb_proc/
├── value
├── enable
├── status
└── message

The interface demonstrates communication between user space and
kernel space using the /proc virtual filesystem.

Note: procfs is traditionally used for kernel/process
information. For device configuration and device attributes,
sysfs is generally preferred.

Directory Structure
19_procfs/
├── proc_driver.c
├── proc_driver.h
├── makefile
└── README.md
Procfs Architecture
                  User Space
                      |
                      |
               cat / echo
                      |
                      v
                  VFS Layer
                      |
                      v
                   Procfs
                      |
          +-----------+-----------+
          |           |           |
          v           v           v
        value       enable      status
          |           |           |
          +-----------+-----------+
                      |
                      v
                Kernel Driver
                      |
                      v
                Driver State
Driver Flow
Module Load
    |
    v
bbb_proc_init()
    |
    v
proc_mkdir()
    |
    v
/proc/bbb_proc
    |
    +--------+---------+---------+
    |        |         |         |
    v        v         v         v
 value    enable    status    message
    |        |         |         |
    +--------+---------+---------+
             |
             v
       Kernel Driver
Procfs Directory

After loading:

sudo insmod proc_driver.ko

the following directory is created:

/proc/bbb_proc/

Check it:

ls -la /proc/bbb_proc

Expected:

value
enable
status
message
value
Read
cat /proc/bbb_proc/value

Initial value:

0
Write
echo 100 > /proc/bbb_proc/value

Read again:

cat /proc/bbb_proc/value

Output:

100

The write operation reaches:

proc_value_write()

The read operation reaches:

proc_value_read()
enable
Read
cat /proc/bbb_proc/enable

Output:

0
Enable
echo 1 > /proc/bbb_proc/enable

Check:

cat /proc/bbb_proc/enable

Output:

1
Disable
echo 0 > /proc/bbb_proc/enable
status

The status entry is read-only.

cat /proc/bbb_proc/status

Example:

BeagleBone Black Procfs Driver
-----------------------------
Value   : 100
Enable  : 1
Message : BeagleBone Black

The status information is generated using the Linux seq_file
interface.

message
Read
cat /proc/bbb_proc/message

Output:

BeagleBone Black
Write
echo "Driver Test" > /proc/bbb_proc/message

Read:

cat /proc/bbb_proc/message

Output:

Driver Test
User Space to Kernel Flow

For:

echo 100 > /proc/bbb_proc/value

the flow is:

echo
 |
 v
VFS
 |
 v
procfs
 |
 v
proc_value_write()
 |
 v
copy_from_user()
 |
 v
kstrtoint()
 |
 v
proc_data.value
Kernel to User Space Flow

For:

cat /proc/bbb_proc/value

the flow is:

cat
 |
 v
VFS
 |
 v
procfs
 |
 v
proc_value_read()
 |
 v
proc_data.value
 |
 v
copy_to_user()
 |
 v
User Space
copy_to_user()

Kernel memory cannot normally be accessed directly from user space.

The driver uses:

copy_to_user()

to safely copy data:

Kernel Buffer
     |
     | copy_to_user()
     v
User Buffer

Example:

if (copy_to_user(buffer, output, len))
    return -EFAULT;
copy_from_user()

For data coming from user space:

copy_from_user()

is used.

Flow:

User Buffer
     |
     | copy_from_user()
     v
Kernel Buffer

Example:

if (copy_from_user(input, buffer, count))
    return -EFAULT;
proc_ops

Modern Linux kernels use:

struct proc_ops

instead of using struct file_operations directly for procfs.

Example:

static const struct proc_ops proc_value_ops = {
    .proc_read  = proc_value_read,
    .proc_write = proc_value_write,
};
Read Callback

The read callback receives:

static ssize_t proc_value_read(
    struct file *file,
    char __user *buffer,
    size_t count,
    loff_t *ppos);

Important parameters:

file
    |
    +-- Open proc file

buffer
    |
    +-- User-space destination

count
    |
    +-- Number of requested bytes

ppos
    |
    +-- Current file position
Write Callback

The write callback receives:

static ssize_t proc_value_write(
    struct file *file,
    const char __user *buffer,
    size_t count,
    loff_t *ppos);

The driver:

User Input
    |
    v
copy_from_user()
    |
    v
Kernel Buffer
    |
    v
kstrtoint()
    |
    v
Driver Variable
seq_file

For formatted and potentially larger output, Linux provides the
seq_file interface.

This driver uses:

single_open()
seq_read()
seq_lseek()
single_release()

The status callback:

proc_status_show()

generates:

Value
Enable
Message
Mutex Protection

The driver has shared data:

struct bbb_proc_data {
    struct mutex lock;
    int value;
    bool enable;
    char message[128];
};

Access is protected using:

mutex_lock()
mutex_unlock()

Example:

mutex_lock(&proc_data.lock);

proc_data.value = value;

mutex_unlock(&proc_data.lock);

This protects the data against concurrent access.

Build

Build:

make

Expected:

proc_driver.ko

Check:

ls -l proc_driver.ko
Load
sudo insmod proc_driver.ko

Check:

lsmod | grep proc_driver

Check logs:

dmesg | tail -30
Test

Check directory:

ls -la /proc/bbb_proc

Read value:

cat /proc/bbb_proc/value

Write value:

echo 50 > /proc/bbb_proc/value

Read:

cat /proc/bbb_proc/value

Enable:

echo 1 > /proc/bbb_proc/enable

Read status:

cat /proc/bbb_proc/status

Change message:

echo "BBB Kernel Driver" > /proc/bbb_proc/message

Read:

cat /proc/bbb_proc/message
Complete Test
sudo insmod proc_driver.ko

cat /proc/bbb_proc/value

echo 100 > /proc/bbb_proc/value

echo 1 > /proc/bbb_proc/enable

echo "Embedded Linux" > /proc/bbb_proc/message

cat /proc/bbb_proc/status

Expected:

BeagleBone Black Procfs Driver
-----------------------------
Value   : 100
Enable  : 1
Message : Embedded Linux
Kernel Logs

Check:

dmesg | grep -i procfs

Example:

BBB procfs: initializing driver
BBB procfs: /proc/bbb_proc created
BBB procfs: value updated to 100
BBB procfs: enable = 1
BBB procfs: message updated
Remove Driver
sudo rmmod proc_driver

Verify:

ls /proc/bbb_proc

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
Procfs vs Sysfs
Procfs
/proc

Traditionally used for:

Process information
Kernel information
Runtime information
Debug information

Examples:

/proc/cpuinfo
/proc/meminfo
/proc/interrupts
/proc/modules
Sysfs
/sys

Primarily used for:

Device attributes
Device configuration
Device state
Hardware information

Example:

/sys/class/gpio/
/sys/class/leds/
/sys/class/net/

For a new device driver, sysfs is generally preferred for device
attributes.

Procfs vs Character Device
             User Space
                 |
        +--------+--------+
        |                 |
        v                 v
      Procfs         Character Device
        |                 |
        v                 v
     /proc              /dev
        |                 |
        v                 v
     read/write       read/write/ioctl

Procfs is convenient for simple information and debugging.

Character devices are better when implementing:

Streaming
IOCTL commands
Polling
Blocking I/O
Large data transfers
Embedded Linux Use Cases

Procfs can be useful for:

Debug information
Driver statistics
Runtime state
Testing
Temporary diagnostic interfaces

Example:

/proc/my_driver/status
/proc/my_driver/stats
/proc/my_driver/debug

However, permanent hardware configuration should normally use
sysfs, ioctl, or another appropriate driver interface.

Important Kernel APIs

This driver demonstrates:

proc_mkdir()
proc_create()
proc_remove()

copy_to_user()
copy_from_user()

kstrtoint()
kstrtobool()

mutex_init()
mutex_lock()
mutex_unlock()

single_open()
seq_read()
seq_lseek()
single_release()
Interview Explanation

A strong interview explanation:

"Procfs is a virtual filesystem exposed through /proc. I can
create proc entries from a kernel module using proc_mkdir() and
proc_create(). The driver implements read and write callbacks,
uses copy_to_user() and copy_from_user() for safe user-kernel
data transfer, and uses seq_file for formatted output. For
device-specific configuration, however, I would generally prefer
sysfs over procfs."

Interview Flow
insmod
  |
  v
module_init()
  |
  v
proc_mkdir()
  |
  v
proc_create()
  |
  v
/proc/bbb_proc/
  |
  +------ value
  |
  +------ enable
  |
  +------ status
  |
  +------ message
  |
  v
User Space
  |
  +-- cat
  |
  +-- echo
  |
  v
proc_ops
  |
  v
Driver Callback
Key Interview Points
Procfs is a virtual filesystem.
It is mounted at /proc.
proc_mkdir() creates a proc directory.
proc_create() creates a proc entry.
Modern kernels use struct proc_ops.
.proc_read handles user-space reads.
.proc_write handles user-space writes.
copy_to_user() transfers data to user space.
copy_from_user() transfers data from user space.
seq_file simplifies formatted output.
mutex protects shared driver state.
ppos tracks the file position.
kstrtoint() converts user input to an integer.
kstrtobool() converts user input to a Boolean.
Sysfs is generally preferred for device attributes.

### Final structure

```text
beaglebone-black/
└── drivers/
    └── 19_procfs/
        ├── proc_driver.c
        ├── proc_driver.h
        ├── makefile
        └── README.md

Core flow:

User Space
    ↓
/proc/bbb_proc/
    ↓
proc_ops
    ↓
read()/write()
    ↓
copy_to_user()/copy_from_user()
    ↓
Kernel Driver
    ↓
Driver State
