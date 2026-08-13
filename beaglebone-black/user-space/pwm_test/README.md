Build
cd beaglebone-black/user-space/pwm_test
make
Run tests
make set
make enable
make disable
make sweep

Or directly:

sudo ./pwm_test 0 0 set
sudo ./pwm_test 0 0 enable
sudo ./pwm_test 0 0 disable
sudo ./pwm_test 0 0 sweep
Files
pwm_test/
├── pwm_test.c
├── pwm_test.h
├── Makefile
└── README.md

The Makefile builds the user-space PWM utility and provides shortcuts for configure, enable, disable, and duty-cycle sweep testing.
