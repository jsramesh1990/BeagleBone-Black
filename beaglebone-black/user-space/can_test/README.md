Build
cd beaglebone-black/user-space/can_test
make
Configure CAN
make setup

Equivalent commands:

sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up
Check CAN status
make status
Run tests
make tx
make rx
make loopback
Clean
make clean

Final directory:

can_test/
├── can_test.c
├── can_test.h
├── Makefile
└── README.md
