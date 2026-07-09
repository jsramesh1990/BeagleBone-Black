obj-m += simple_char.o

KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

test: test_char
	@echo "=== Testing Character Device ==="
	sudo ./test_char

test_char: test_char.c
	gcc -o test_char test_char.c

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f test_char

.PHONY: all clean test
