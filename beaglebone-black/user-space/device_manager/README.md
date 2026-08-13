Build
cd beaglebone-black/user-space/device_manager
make
Run
./device_manager

Or:

make list
make status
make scan
make help
Install
sudo make install

This installs:

/usr/local/bin/device_manager
Clean
make clean

Final directory:

device_manager/
├── device_manager.c
├── device_manager.h
├── Makefile
└── README.md
