Driver flow
                  User Space
                      │
              open/read/write/ioctl
                      │
                      ▼
              /dev/bbb_char
                      │
                      ▼
             char_driver.c
                      │
       ┌──────────────┼──────────────┐
       ▼              ▼              ▼
     open()         read()         write()
       │              │              │
       └──────────────┼──────────────┘
                      ▼
                Kernel Buffer
                      │
                      ▼
                release()
Build on BeagleBone Black
cd beaglebone-black/drivers/01_char_driver

make

Load:

sudo insmod char_driver.ko

Check:

lsmod | grep char_driver
ls -l /dev/bbb_char
dmesg | tail -30

Test write/read:

echo "Hello BBB Driver" | sudo tee /dev/bbb_char

sudo cat /dev/bbb_char

Remove:

sudo rmmod char_driver

One important point: the Makefile defaults to the currently running kernel's build tree. For your actual BeagleBone Black Yocto/BSP build, use the target kernel build directory or Yocto's module recipe rather than compiling against an unrelated host kernel.
