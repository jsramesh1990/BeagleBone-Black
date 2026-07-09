#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <ctype.h>

#define DEVICE_PATH "/dev/simple_char"
#define BUFFER_SIZE 4096
#define MAX_INPUT 256

// IOCTL definitions (must match driver)
#define CHAR_IOCTL_MAGIC 'C'
#define CHAR_GET_SIZE _IOR(CHAR_IOCTL_MAGIC, 1, int)
#define CHAR_RESET_BUFFER _IO(CHAR_IOCTL_MAGIC, 2)
#define CHAR_GET_STATS _IOR(CHAR_IOCTL_MAGIC, 3, struct char_stats)
#define CHAR_SET_BUFFER_SIZE _IOW(CHAR_IOCTL_MAGIC, 4, int)

struct char_stats {
    int read_count;
    int write_count;
    int buffer_used;
    int buffer_size;
};

void clear_screen() {
    printf("\033[2J\033[1;1H");
}

void display_header() {
    printf("========================================\n");
    printf("    CHARACTER DEVICE INTERFACE\n");
    printf("========================================\n");
}

void display_menu() {
    printf("\nMAIN MENU:\n");
    printf("1. Write to device\n");
    printf("2. Read from device\n");
    printf("3. Read entire buffer\n");
    printf("4. Append to device\n");
    printf("5. Seek position\n");
    printf("6. Get device statistics\n");
    printf("7. Reset device buffer\n");
    printf("8. Change buffer size\n");
    printf("9. View current position\n");
    printf("0. Exit\n");
    printf("\nEnter your choice: ");
}

void press_enter_to_continue() {
    printf("\nPress Enter to continue...");
    while (getchar() != '\n');
    getchar();
}

int get_device_size(int fd) {
    int size;
    if (ioctl(fd, CHAR_GET_SIZE, &size) < 0) {
        perror("Failed to get device size");
        return -1;
    }
    return size;
}

void get_device_stats(int fd) {
    struct char_stats stats;
    
    if (ioctl(fd, CHAR_GET_STATS, &stats) < 0) {
        perror("Failed to get device statistics");
        return;
    }
    
    printf("\nDEVICE STATISTICS:\n");
    printf("==================\n");
    printf("Buffer size:      %d bytes\n", stats.buffer_size);
    printf("Buffer used:      %d bytes\n", stats.buffer_used);
    printf("Read operations:  %d\n", stats.read_count);
    printf("Write operations: %d\n", stats.write_count);
    printf("Free space:       %d bytes\n", stats.buffer_size - stats.buffer_used);
}

void write_to_device(int fd) {
    char input[MAX_INPUT];
    ssize_t bytes_written;
    
    printf("\nEnter text to write to device (max %d chars):\n", MAX_INPUT-1);
    printf("> ");
    
    fgets(input, MAX_INPUT, stdin);
    input[strcspn(input, "\n")] = 0; // Remove newline
    
    if (strlen(input) == 0) {
        printf("No input provided.\n");
        return;
    }
    
    bytes_written = write(fd, input, strlen(input));
    if (bytes_written < 0) {
        perror("Failed to write to device");
    } else {
        printf("Successfully wrote %ld bytes to device\n", bytes_written);
    }
}

void read_from_device(int fd) {
    char buffer[BUFFER_SIZE];
    int bytes_to_read;
    ssize_t bytes_read;
    
    printf("\nEnter number of bytes to read: ");
    scanf("%d", &bytes_to_read);
    while (getchar() != '\n'); // Clear input buffer
    
    if (bytes_to_read <= 0 || bytes_to_read > BUFFER_SIZE-1) {
        printf("Invalid number of bytes. Must be between 1 and %d\n", BUFFER_SIZE-1);
        return;
    }
    
    bytes_read = read(fd, buffer, bytes_to_read);
    if (bytes_read < 0) {
        perror("Failed to read from device");
    } else if (bytes_read == 0) {
        printf("End of buffer reached or buffer is empty\n");
    } else {
        buffer[bytes_read] = '\0';
        printf("\nRead %ld bytes:\n", bytes_read);
        printf("====================\n");
        printf("%s\n", buffer);
        printf("====================\n");
        
        // Display in hex for binary data
        printf("\nHex dump (first 64 bytes):\n");
        int display_len = bytes_read < 64 ? bytes_read : 64;
        for (int i = 0; i < display_len; i++) {
            printf("%02x ", (unsigned char)buffer[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");
    }
}

void read_entire_buffer(int fd) {
    char buffer[BUFFER_SIZE];
    off_t current_pos;
    ssize_t bytes_read;
    
    // Save current position
    current_pos = lseek(fd, 0, SEEK_CUR);
    
    // Seek to beginning
    lseek(fd, 0, SEEK_SET);
    
    bytes_read = read(fd, buffer, BUFFER_SIZE-1);
    if (bytes_read < 0) {
        perror("Failed to read from device");
    } else if (bytes_read == 0) {
        printf("Buffer is empty\n");
    } else {
        buffer[bytes_read] = '\0';
        printf("\nENTIRE BUFFER (%ld bytes):\n", bytes_read);
        printf("=========================\n");
        
        // Check if buffer contains printable text
        int is_text = 1;
        for (int i = 0; i < bytes_read && i < 1024; i++) {
            if (!isprint(buffer[i]) && !isspace(buffer[i]) && buffer[i] != '\0') {
                is_text = 0;
                break;
            }
        }
        
        if (is_text) {
            printf("%s\n", buffer);
        } else {
            printf("[Binary data - displaying hex dump]\n");
            for (int i = 0; i < bytes_read; i++) {
                printf("%02x ", (unsigned char)buffer[i]);
                if ((i + 1) % 16 == 0) printf("\n");
            }
            printf("\n");
        }
        printf("=========================\n");
    }
    
    // Restore position
    lseek(fd, current_pos, SEEK_SET);
}

void append_to_device(int fd) {
    char input[MAX_INPUT];
    ssize_t bytes_written;
    
    // Seek to end
    lseek(fd, 0, SEEK_END);
    
    printf("\nEnter text to append to device:\n");
    printf("> ");
    
    fgets(input, MAX_INPUT, stdin);
    input[strcspn(input, "\n")] = 0;
    
    if (strlen(input) == 0) {
        printf("No input provided.\n");
        return;
    }
    
    bytes_written = write(fd, input, strlen(input));
    if (bytes_written < 0) {
        perror("Failed to append to device");
    } else {
        printf("Successfully appended %ld bytes\n", bytes_written);
    }
}

void seek_position(int fd) {
    long offset;
    int whence;
    off_t new_pos;
    
    printf("\nSeek Options:\n");
    printf("1. SEEK_SET (from beginning)\n");
    printf("2. SEEK_CUR (from current position)\n");
    printf("3. SEEK_END (from end)\n");
    printf("Enter choice (1-3): ");
    scanf("%d", &whence);
    
    printf("Enter offset: ");
    scanf("%ld", &offset);
    while (getchar() != '\n');
    
    if (whence < 1 || whence > 3) {
        printf("Invalid choice\n");
        return;
    }
    
    new_pos = lseek(fd, offset, whence - 1); // Convert to 0-based
    if (new_pos == (off_t)-1) {
        perror("Seek failed");
    } else {
        printf("New position: %ld\n", new_pos);
    }
}

void reset_device_buffer(int fd) {
    char confirm;
    
    printf("\nWARNING: This will erase all data in the device buffer!\n");
    printf("Are you sure? (y/N): ");
    scanf(" %c", &confirm);
    while (getchar() != '\n');
    
    if (confirm == 'y' || confirm == 'Y') {
        if (ioctl(fd, CHAR_RESET_BUFFER) < 0) {
            perror("Failed to reset buffer");
        } else {
            printf("Device buffer reset successfully\n");
        }
    } else {
        printf("Reset cancelled\n");
    }
}

void change_buffer_size(int fd) {
    int new_size;
    
    printf("\nCurrent buffer size: %d bytes\n", get_device_size(fd));
    printf("Enter new buffer size (1-65536): ");
    scanf("%d", &new_size);
    while (getchar() != '\n');
    
    if (new_size < 1 || new_size > 65536) {
        printf("Invalid size\n");
        return;
    }
    
    if (ioctl(fd, CHAR_SET_BUFFER_SIZE, &new_size) < 0) {
        perror("Failed to change buffer size");
    } else {
        printf("Buffer size changed to %d bytes\n", new_size);
    }
}

void view_current_position(int fd) {
    off_t current_pos = lseek(fd, 0, SEEK_CUR);
    int buffer_size = get_device_size(fd);
    
    printf("\nCurrent position: %ld bytes\n", current_pos);
    printf("Buffer size: %d bytes\n", buffer_size);
    printf("Bytes to end: %ld bytes\n", buffer_size - current_pos);
}

int main() {
    int fd;
    int choice;
    
    // Open device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        printf("Make sure the driver is loaded and device node exists:\n");
        printf("sudo insmod simple_char.ko\n");
        printf("sudo mknod /dev/simple_char c 240 0\n");
        return EXIT_FAILURE;
    }
    
    // Set terminal to raw mode for better input
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    
    while (1) {
        clear_screen();
        display_header();
        
        // Get device info
        int size = get_device_size(fd);
        if (size > 0) {
            off_t pos = lseek(fd, 0, SEEK_CUR);
            printf("Device: %s | Buffer: %d bytes | Position: %ld\n", 
                   DEVICE_PATH, size, pos);
        }
        
        display_menu();
        
        // Get user choice
        char input[10];
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);
        
        switch (choice) {
            case 1:
                write_to_device(fd);
                break;
            case 2:
                read_from_device(fd);
                break;
            case 3:
                read_entire_buffer(fd);
                break;
            case 4:
                append_to_device(fd);
                break;
            case 5:
                seek_position(fd);
                break;
            case 6:
                get_device_stats(fd);
                break;
            case 7:
                reset_device_buffer(fd);
                break;
            case 8:
                change_buffer_size(fd);
                break;
            case 9:
                view_current_position(fd);
                break;
            case 0:
                close(fd);
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                printf("\nGoodbye!\n");
                return EXIT_SUCCESS;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
        
        press_enter_to_continue();
    }
    
    close(fd);
    return EXIT_SUCCESS;
}
