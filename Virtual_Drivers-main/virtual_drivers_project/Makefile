KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: char block apps

char:
	@echo "Building character driver..."
	$(MAKE) -C char_driver

block:
	@echo "Building block driver..."
	$(MAKE) -C block_driver

apps:
	@echo "Building applications..."
	$(MAKE) -C apps

clean:
	$(MAKE) -C char_driver clean
	$(MAKE) -C block_driver clean
	$(MAKE) -C apps clean

install: all
	@echo "Installing drivers..."
	sudo insmod char_driver/simple_char.ko 2>/dev/null || true
	sudo insmod block_driver/simple_block.ko 2>/dev/null || true
	@echo "Creating device nodes..."
	sudo mknod /dev/simple_char c 240 0 2>/dev/null || true
	sudo mknod /dev/simple_block b 241 0 2>/dev/null || true
	sudo chmod 666 /dev/simple_char /dev/simple_block
	@echo "Installing applications..."
	$(MAKE) -C apps install

uninstall:
	@echo "Removing drivers..."
	sudo rmmod simple_char 2>/dev/null || true
	sudo rmmod simple_block 2>/dev/null || true
	@echo "Removing device nodes..."
	sudo rm -f /dev/simple_char /dev/simple_block
	@echo "Uninstalling applications..."
	$(MAKE) -C apps uninstall

test:
	@echo "=== Running Character Device Tests ==="
	$(MAKE) -C char_driver test
	@echo "\n=== Running Block Device Tests ==="
	$(MAKE) -C block_driver test

demo:
	@echo "=== Driver Demo ==="
	sudo $(MAKE) install
	@echo "\n1. Character Device Interface:"
	@echo "   Run: ./char_driver/char_interface"
	@echo "\n2. Block Device Interface:"
	@echo "   Run: ./block_driver/block_interface"
	@echo "\n3. Advanced Applications:"
	@echo "   Run: ./apps/char_app"
	@echo "   Run: ./apps/block_app"
	@echo "   Run: ./apps/unified_app"

help:
	@echo "Available targets:"
	@echo "  all       - Build everything"
	@echo "  char      - Build character driver only"
	@echo "  block     - Build block driver only"
	@echo "  apps      - Build applications only"
	@echo "  install   - Install drivers and applications"
	@echo "  uninstall - Remove everything"
	@echo "  test      - Run basic tests"
	@echo "  demo      - Show demo instructions"
	@echo "  clean     - Clean build files"
	@echo "  help      - Show this help"

.PHONY: all char block apps clean install uninstall test demo help
