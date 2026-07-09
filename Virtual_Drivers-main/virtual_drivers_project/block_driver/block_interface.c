#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <termios.h>
#include <ctype.h>
#include <time.h>

#define DEVICE_PATH "/dev/simple_block"
#define SECTOR_SIZE 512
#define MAX_SECTORS 65536
#define BUFFER_SIZE (SECTOR_SIZE * 16)  // 8KB buffer

void clear_screen() {
    printf("\033[2J\033[1;1H");
}

void display_header() {
    printf("========================================\n");
    printf("     BLOCK DEVICE INTERFACE\n");
    printf("========================================\n");
}

void display_menu() {
    printf("\nBLOCK DEVICE MENU:\n");
    printf("1. Write data to device\n");
    printf("2. Read data from device\n");
    printf("3. Read specific sector\n");
    printf("4. Fill device with pattern\n");
    printf("5. Clear device (zero fill)\n");
    printf("6. Get device information\n");
    printf("7. Benchmark read/write\n");
    printf("8. Compare sectors\n");
    printf("9. Hex dump sector\n");
    printf("0. Exit\n");
    printf("\nEnter your choice: ");
}

void press_enter_to_continue() {
    printf("\nPress Enter to continue...");
    while (getchar() != '\n');
    getchar();
}

unsigned long get_device_size_sectors(int fd) {
    unsigned long sectors;
    if (ioctl(fd, BLKGETSIZE, &sectors) < 0) {
        perror("Failed to get device size");
        return 0;
    }
    return sectors;
}

unsigned long long get_device_size_bytes(int fd) {
    unsigned long long bytes;
    if (ioctl(fd, BLKGETSIZE64, &bytes) < 0) {
        // Fallback to sectors * sector size
        unsigned long sectors = get_device_size_sectors(fd);
        bytes = (unsigned long long)sectors * SECTOR_SIZE;
    }
    return bytes;
}

void get_device_info(int fd) {
    unsigned long sectors = get_device_size_sectors(fd);
    unsigned long long bytes = get_device_size_bytes(fd);
    
    printf("\nDEVICE INFORMATION:\n");
    printf("===================\n");
    printf("Device:          %s\n", DEVICE_PATH);
    printf("Sector size:     %d bytes\n", SECTOR_SIZE);
    printf("Total sectors:   %lu\n", sectors);
    printf("Total size:      %llu bytes\n", bytes);
    printf("                 %.2f KB\n", bytes / 1024.0);
    printf("                 %.2f MB\n", bytes / (1024.0 * 1024.0));
    printf("\nAddress Range:   0x00000000 - 0x%08llx\n", bytes - 1);
    printf("Sector Range:    0 - %lu\n", sectors - 1);
}

void write_to_device(int fd) {
    unsigned long sector;
    char buffer[BUFFER_SIZE];
    int bytes_to_write;
    ssize_t bytes_written;
    char input[256];
    
    printf("\nEnter starting sector number: ");
    scanf("%lu", &sector);
    while (getchar() != '\n');
    
    unsigned long max_sector = get_device_size_sectors(fd) - 1;
    if (sector > max_sector) {
        printf("Error: Sector %lu exceeds maximum sector %lu\n", sector, max_sector);
        return;
    }
    
    printf("Enter data to write (max %d chars):\n", BUFFER_SIZE-1);
    printf("> ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    
    if (strlen(input) == 0) {
        printf("No input provided. Using default pattern.\n");
        // Fill with pattern
        for (int i = 0; i < BUFFER_SIZE; i++) {
            buffer[i] = i % 256;
        }
        bytes_to_write = BUFFER_SIZE;
    } else {
        bytes_to_write = strlen(input);
        if (bytes_to_write > BUFFER_SIZE) {
            bytes_to_write = BUFFER_SIZE;
        }
        memcpy(buffer, input, bytes_to_write);
        // Pad with zeros if less than sector size
        if (bytes_to_write < SECTOR_SIZE) {
            memset(buffer + bytes_to_write, 0, SECTOR_SIZE - bytes_to_write);
            bytes_to_write = SECTOR_SIZE;
        }
    }
    
    // Align to sector boundary
    bytes_to_write = (bytes_to_write + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1);
    
    // Seek to sector
    off_t offset = sector * SECTOR_SIZE;
    lseek(fd, offset, SEEK_SET);
    
    bytes_written = write(fd, buffer, bytes_to_write);
    if (bytes_written < 0) {
        perror("Failed to write to device");
    } else {
        printf("Successfully wrote %ld bytes to sector %lu\n", 
               bytes_written, sector);
        printf("(%ld sectors affected)\n", bytes_written / SECTOR_SIZE);
    }
}

void read_from_device(int fd) {
    unsigned long sector;
    int sectors_to_read;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    printf("\nEnter starting sector number: ");
    scanf("%lu", &sector);
    while (getchar() != '\n');
    
    printf("Enter number of sectors to read (max %d): ", 
           BUFFER_SIZE / SECTOR_SIZE);
    scanf("%d", &sectors_to_read);
    while (getchar() != '\n');
    
    unsigned long max_sector = get_device_size_sectors(fd) - 1;
    if (sector > max_sector) {
        printf("Error: Sector %lu exceeds maximum sector %lu\n", sector, max_sector);
        return;
    }
    
    if (sectors_to_read <= 0 || sectors_to_read > BUFFER_SIZE / SECTOR_SIZE) {
        printf("Invalid number of sectors\n");
        return;
    }
    
    int bytes_to_read = sectors_to_read * SECTOR_SIZE;
    
    // Seek to sector
    off_t offset = sector * SECTOR_SIZE;
    lseek(fd, offset, SEEK_SET);
    
    bytes_read = read(fd, buffer, bytes_to_read);
    if (bytes_read < 0) {
        perror("Failed to read from device");
    } else if (bytes_read == 0) {
        printf("No data read (end of device?)\n");
    } else {
        printf("\nRead %ld bytes from sector %lu:\n", bytes_read, sector);
        printf("================================\n");
        
        // Check if data is printable
        int is_text = 1;
        for (int i = 0; i < bytes_read && i < 1024; i++) {
            if (!isprint(buffer[i]) && !isspace(buffer[i]) && buffer[i] != '\0') {
                is_text = 0;
                break;
            }
        }
        
        if (is_text) {
            buffer[bytes_read] = '\0';
            printf("%s\n", buffer);
        } else {
            // Display hex dump
            printf("Hex dump:\n");
            for (int i = 0; i < bytes_read; i++) {
                printf("%02x ", (unsigned char)buffer[i]);
                if ((i + 1) % 16 == 0) printf("\n");
                if (i >= 511) { // Limit display
                    printf("\n[... %ld more bytes ...]\n", bytes_read - 512);
                    break;
                }
            }
        }
        printf("================================\n");
    }
}

void read_specific_sector(int fd) {
    unsigned long sector;
    char buffer[SECTOR_SIZE];
    ssize_t bytes_read;
    
    printf("\nEnter sector number: ");
    scanf("%lu", &sector);
    while (getchar() != '\n');
    
    unsigned long max_sector = get_device_size_sectors(fd) - 1;
    if (sector > max_sector) {
        printf("Error: Sector %lu exceeds maximum sector %lu\n", sector, max_sector);
        return;
    }
    
    off_t offset = sector * SECTOR_SIZE;
    lseek(fd, offset, SEEK_SET);
    
    bytes_read = read(fd, buffer, SECTOR_SIZE);
    if (bytes_read < 0) {
        perror("Failed to read sector");
    } else {
        printf("\nSECTOR %lu (%08lx - %08lx):\n", 
               sector, offset, offset + SECTOR_SIZE - 1);
        printf("================================\n");
        
        // Display in hex and ASCII
        for (int i = 0; i < SECTOR_SIZE; i++) {
            if (i % 16 == 0) {
                if (i > 0) {
                    // Show ASCII representation
                    printf(" | ");
                    for (int j = i-16; j < i; j++) {
                        unsigned char c = buffer[j];
                        printf("%c", isprint(c) ? c : '.');
                    }
                }
                printf("\n%08lx: ", offset + i);
            }
            printf("%02x ", (unsigned char)buffer[i]);
        }
        
        // Last line ASCII
        printf(" | ");
        for (int j = SECTOR_SIZE - 16; j < SECTOR_SIZE; j++) {
            unsigned char c = buffer[j];
            printf("%c", isprint(c) ? c : '.');
        }
        printf("\n================================\n");
    }
}

void fill_with_pattern(int fd) {
    unsigned long start_sector, num_sectors;
    char pattern;
    char buffer[SECTOR_SIZE];
    
    printf("\nFILL DEVICE WITH PATTERN\n");
    printf("=========================\n");
    
    printf("Enter starting sector: ");
    scanf("%lu", &start_sector);
    while (getchar() != '\n');
    
    printf("Enter number of sectors: ");
    scanf("%lu", &num_sectors);
    while (getchar() != '\n');
    
    printf("Enter pattern character (or 'r' for random): ");
    scanf(" %c", &pattern);
    while (getchar() != '\n');
    
    unsigned long max_sector = get_device_size_sectors(fd) - 1;
    if (start_sector + num_sectors - 1 > max_sector) {
        printf("Error: Requested range exceeds device size\n");
        return;
    }
    
    printf("\nFilling sectors %lu-%lu...\n", 
           start_sector, start_sector + num_sectors - 1);
    
    srand(time(NULL));
    
    for (unsigned long i = 0; i < num_sectors; i++) {
        if (pattern == 'r' || pattern == 'R') {
            // Fill with random data
            for (int j = 0; j < SECTOR_SIZE; j++) {
                buffer[j] = rand() % 256;
            }
        } else {
            // Fill with pattern character
            memset(buffer, pattern, SECTOR_SIZE);
        }
        
        off_t offset = (start_sector + i) * SECTOR_SIZE;
        lseek(fd, offset, SEEK_SET);
        
        ssize_t written = write(fd, buffer, SECTOR_SIZE);
        if (written != SECTOR_SIZE) {
            perror("Failed to write sector");
            break;
        }
        
        // Show progress
        if ((i + 1) % 10 == 0) {
            printf("  Filled %lu/%lu sectors\r", i + 1, num_sectors);
            fflush(stdout);
        }
    }
    
    printf("\nDone! Filled %lu sectors with pattern.\n", num_sectors);
}

void clear_device(int fd) {
    unsigned long start_sector, num_sectors;
    char confirm;
    
    printf("\nCLEAR DEVICE (ZERO FILL)\n");
    printf("=========================\n");
    
    printf("Enter starting sector: ");
    scanf("%lu", &start_sector);
    while (getchar() != '\n');
    
    printf("Enter number of sectors (0 for all): ");
    scanf("%lu", &num_sectors);
    while (getchar() != '\n');
    
    unsigned long max_sector = get_device_size_sectors(fd) - 1;
    if (num_sectors == 0) {
        num_sectors = max_sector - start_sector + 1;
    }
    
    if (start_sector + num_sectors - 1 > max_sector) {
        printf("Error: Requested range exceeds device size\n");
        return;
    }
    
    printf("This will zero %lu sectors (%lu KB). Continue? (y/N): ",
           num_sectors, (num_sectors * SECTOR_SIZE) / 1024);
    scanf(" %c", &confirm);
    while (getchar() != '\n');
    
    if (confirm != 'y' && confirm != 'Y') {
        printf("Operation cancelled\n");
        return;
    }
    
    char zero_buffer[SECTOR_SIZE] = {0};
    
    printf("\nClearing sectors %lu-%lu...\n", 
           start_sector, start_sector + num_sectors - 1);
    
    for (unsigned long i = 0; i < num_sectors; i++) {
        off_t offset = (start_sector + i) * SECTOR_SIZE;
        lseek(fd, offset, SEEK_SET);
        
        ssize_t written = write(fd, zero_buffer, SECTOR_SIZE);
        if (written != SECTOR_SIZE) {
            perror("Failed to clear sector");
            break;
        }
        
        // Show progress
        if ((i + 1) % 50 == 0) {
            printf("  Cleared %lu/%lu sectors\r", i + 1, num_sectors);
            fflush(stdout);
        }
    }
    
    printf("\nDone! Cleared %lu sectors.\n", num_sectors);
}

void benchmark_device(int fd) {
    unsigned long sector;
    int num_operations;
    char buffer[SECTOR_SIZE];
    clock_t start, end;
    double read_time, write_time;
    
    printf("\nBENCHMARK DEVICE PERFORMANCE\n");
    printf("=============================\n");
    
    printf("Enter starting sector for benchmark: ");
    scanf("%lu", &sector);
    while (getchar() != '\n');
    
    printf("Enter number of operations: ");
    scanf("%d", &num_operations);
    while (getchar() != '\n');
    
    if (num_operations <= 0) {
        printf("Invalid number of operations\n");
        return;
    }
    
    // Prepare test data
    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = i % 256;
    }
    
    printf("\nRunning write benchmark...\n");
    start = clock();
    
    for (int i = 0; i < num_operations; i++) {
        off_t offset = (sector + i) * SECTOR_SIZE;
        lseek(fd, offset, SEEK_SET);
        write(fd, buffer, SECTOR_SIZE);
    }
    
    end = clock();
    write_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Running read benchmark...\n");
    start = clock();
    
    for (int i = 0; i < num_operations; i++) {
        off_t offset = (sector + i) * SECTOR_SIZE;
        lseek(fd, offset, SEEK_SET);
        read(fd, buffer, SECTOR_SIZE);
    }
    
    end = clock();
    read_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\nBENCHMARK RESULTS:\n");
    printf("==================\n");
    printf("Operations:       %d\n", num_operations);
    printf("Total data:       %d KB\n", (num_operations * SECTOR_SIZE) / 1024);
    printf("\nWrite time:       %.3f seconds\n", write_time);
    printf("Write speed:      %.2f KB/s\n", 
           (num_operations * SECTOR_SIZE) / (write_time * 1024));
    printf("\nRead time:        %.3f seconds\n", read_time);
    printf("Read speed:       %.2f KB/s\n", 
           (num_operations * SECTOR_SIZE) / (read_time * 1024));
}

int main() {
    int fd;
    int choice;
    
    // Open device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        printf("Make sure the driver is loaded:\n");
        printf("sudo insmod simple_block.ko\n");
        return EXIT_FAILURE;
    }
    
    // Set terminal to raw mode
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    
    while (1) {
        clear_screen();
        display_header();
        
        // Display device info
        unsigned long sectors = get_device_size_sectors(fd);
        unsigned long long bytes = get_device_size_bytes(fd);
        printf("Device: %s | Size: %lu sectors (%.2f MB)\n", 
               DEVICE_PATH, sectors, bytes / (1024.0 * 1024.0));
        
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
                read_specific_sector(fd);
                break;
            case 4:
                fill_with_pattern(fd);
                break;
            case 5:
                clear_device(fd);
                break;
            case 6:
                get_device_info(fd);
                break;
            case 7:
                benchmark_device(fd);
                break;
            case 8:
                printf("Compare sectors feature not implemented yet\n");
                break;
            case 9:
                printf("Hex dump feature not implemented yet\n");
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
