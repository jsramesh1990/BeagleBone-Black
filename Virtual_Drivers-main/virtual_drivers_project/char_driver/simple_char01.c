#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE_PATH "/dev/simple_char"
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
    
    printf("Device opened successfully\n");
    
    // Write to device
    const char *message = "Hello from userspace to character driver!";
    bytes_written = write(fd, message, strlen(message));
    if (bytes_written < 0) {
        perror("Failed to write to device");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Wrote %ld bytes to device\n", bytes_written);
    
    // Reset file pointer for reading
    lseek(fd, 0, SEEK_SET);
    
    // Read from device
    bytes_read = read(fd, read_buffer, BUFFER_SIZE - 1);
    if (bytes_read < 0) {
        perror("Failed to read from device");
        close(fd);
        return EXIT_FAILURE;
    }
    
    read_buffer[bytes_read] = '\0';
    printf("Read %ld bytes from device: %s\n", bytes_read, read_buffer);
    
    // Close device
    close(fd);
    printf("Device closed\n");
    
    return EXIT_SUCCESS;
}
