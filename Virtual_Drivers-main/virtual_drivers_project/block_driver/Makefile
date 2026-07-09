obj-m += simple_block.o

KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

test: test_block
	@echo "=== Testing Block Device ==="
	sudo ./test_block

test_block: test_block.c
	gcc -o test_block test_block.c

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f test_block

.PHONY: all clean test
