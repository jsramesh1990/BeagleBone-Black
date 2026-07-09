#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

#define DEVICE_PATH "/dev/simple_block"
#define BLOCK_SIZE 512
#define BUFFER_SIZE 1024

int main() {
    int fd;
    char write_buffer[BUFFER_SIZE];
    char read_buffer[BUFFER_SIZE];
    ssize_t bytes_written, bytes_read;
    
    // Open device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }
    
    printf("Block device opened successfully\n");
    
    // Get device size
    long size = 0;
    if (ioctl(fd, BLKGETSIZE, &size) >= 0) {
        printf("Device size: %ld sectors (%ld KB)\n", size, (size * BLOCK_SIZE) / 1024);
    }
    
    // Write to block device
    const char *message = "Testing block device driver from userspace!";
    strncpy(write_buffer, message, BUFFER_SIZE - 1);
    
    bytes_written = write(fd, write_buffer, strlen(message));
    if (bytes_written < 0) {
        perror("Failed to write to block device");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Wrote %ld bytes to block device\n", bytes_written);
    
    // Seek to beginning and read
    lseek(fd, 0, SEEK_SET);
    
    bytes_read = read(fd, read_buffer, BUFFER_SIZE - 1);
    if (bytes_read < 0) {
        perror("Failed to read from block device");
        close(fd);
        return EXIT_FAILURE;
    }
    
    read_buffer[bytes_read] = '\0';
    printf("Read %ld bytes from block device:\n%s\n", bytes_read, read_buffer);
    
    // Close device
    close(fd);
    printf("Block device closed\n");
    
    return EXIT_SUCCESS;
}
