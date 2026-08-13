Build and test
cd beaglebone-black/user-space/rtc_test
make
make read
make set
make alarm
make periodic

Or directly:

sudo ./rtc_test /dev/rtc0 read

The rtc_test application uses the standard Linux RTC interface through /dev/rtc0 and ioctl() operations such as RTC_RD_TIME, RTC_ALM_SET, RTC_AIE_ON, and RTC_PIE_ON.
