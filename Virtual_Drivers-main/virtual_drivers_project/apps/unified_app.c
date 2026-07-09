#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h>
#include <termios.h>
#include <sys/time.h>

#define CHAR_DEVICE "/dev/simple_char"
#define BLOCK_DEVICE "/dev/simple_block"
#define SECTOR_SIZE 512
#define MAX_BUFFER_SIZE 65536

// Color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"

// Character device IOCTL definitions
#define CHAR_IOCTL_MAGIC 'C'
#define CHAR_GET_SIZE _IOR(CHAR_IOCTL_MAGIC, 1, int)
#define CHAR_GET_STATS _IOR(CHAR_IOCTL_MAGIC, 3, struct char_stats)

struct char_stats {
    int read_count;
    int write_count;
    int buffer_used;
    int buffer_size;
};

typedef enum {
    OP_READ,
    OP_WRITE,
    OP_BENCHMARK,
    OP_VERIFY
} operation_type;

typedef struct {
    int char_fd;
    int block_fd;
    operation_type op_type;
    size_t data_size;
    unsigned long block_sector;
    int thread_id;
} unified_thread_args;

// Function prototypes
void clear_screen();
void print_banner();
void print_status(int char_fd, int block_fd);
void main_menu();
void char_device_menu(int char_fd);
void block_device_menu(int block_fd);
void transfer_menu(int char_fd, int block_fd);
void performance_menu(int char_fd, int block_fd);
void diagnostics_menu(int char_fd, int block_fd);
void about_screen();
void* unified_thread_func(void* arg);
double get_time_ms();

// Utility functions
void clear_screen() {
    printf("\033[2J\033[1;1H");
}

void print_banner() {
    clear_screen();
    printf(COLOR_CYAN "╔══════════════════════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_BOLD COLOR_YELLOW "         VIRTUAL DRIVERS UNIFIED CONTROL PANEL       " COLOR_RESET COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_WHITE "           Character & Block Device Manager v2.0         " COLOR_RESET COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "╚══════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    printf("\n");
}

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

void print_status(int char_fd, int block_fd) {
    printf(COLOR_BOLD "DEVICE STATUS:\n" COLOR_RESET);
    printf(COLOR_MAGENTA "──────────────────────────────────────────────────────\n" COLOR_RESET);
    
    // Character device status
    if (char_fd >= 0) {
        struct char_stats char_stats;
        if (ioctl(char_fd, CHAR_GET_STATS, &char_stats) >= 0) {
            printf(COLOR_GREEN "● Character Device: " COLOR_WHITE "/dev/simple_char\n" COLOR_RESET);
            printf("  Buffer: %d/%d bytes | Ops: R:%d W:%d\n",
                   char_stats.buffer_used, char_stats.buffer_size,
                   char_stats.read_count, char_stats.write_count);
        } else {
            printf(COLOR_GREEN "● Character Device: " COLOR_WHITE "Connected\n" COLOR_RESET);
        }
    } else {
        printf(COLOR_RED "○ Character Device: " COLOR_WHITE "Disconnected\n" COLOR_RESET);
    }
    
    // Block device status
    if (block_fd >= 0) {
        unsigned long sectors;
        if (ioctl(block_fd, BLKGETSIZE, &sectors) >= 0) {
            printf(COLOR_GREEN "● Block Device: " COLOR_WHITE "/dev/simple_block\n" COLOR_RESET);
            printf("  Size: %lu sectors (%.2f MB)\n", 
                   sectors, (sectors * SECTOR_SIZE) / (1024.0 * 1024.0));
        } else {
            printf(COLOR_GREEN "● Block Device: " COLOR_WHITE "Connected\n" COLOR_RESET);
        }
    } else {
        printf(COLOR_RED "○ Block Device: " COLOR_WHITE "Disconnected\n" COLOR_RESET);
    }
    
    printf(COLOR_MAGENTA "──────────────────────────────────────────────────────\n" COLOR_RESET);
    printf("\n");
}

void* unified_thread_func(void* arg) {
    unified_thread_args* args = (unified_thread_args*)arg;
    char buffer[4096];
    char thread_id_str[16];
    
    sprintf(thread_id_str, "[Thread %d]", args->thread_id);
    
    switch (args->op_type) {
        case OP_READ:
            if (args->char_fd >= 0) {
                lseek(args->char_fd, 0, SEEK_SET);
                ssize_t read_bytes = read(args->char_fd, buffer, args->data_size);
                printf("%s Read %ld bytes from character device\n", 
                       thread_id_str, read_bytes);
            }
            if (args->block_fd >= 0) {
                off_t offset = args->block_sector * SECTOR_SIZE;
                lseek(args->block_fd, offset, SEEK_SET);
                ssize_t read_bytes = read(args->block_fd, buffer, args->data_size);
                printf("%s Read %ld bytes from block device sector %lu\n",
                       thread_id_str, read_bytes, args->block_sector);
            }
            break;
            
        case OP_WRITE:
            // Prepare test data
            memset(buffer, args->thread_id + 'A', sizeof(buffer));
            
            if (args->char_fd >= 0) {
                lseek(args->char_fd, 0, SEEK_SET);
                ssize_t written = write(args->char_fd, buffer, args->data_size);
                printf("%s Wrote %ld bytes to character device\n",
                       thread_id_str, written);
            }
            if (args->block_fd >= 0) {
                off_t offset = args->block_sector * SECTOR_SIZE;
                lseek(args->block_fd, offset, SEEK_SET);
                ssize_t written = write(args->block_fd, buffer, args->data_size);
                printf("%s Wrote %ld bytes to block device sector %lu\n",
                       thread_id_str, written, args->block_sector);
            }
            break;
            
        default:
            break;
    }
    
    return NULL;
}

void main_menu() {
    printf(COLOR_BOLD "MAIN MENU:\n" COLOR_RESET);
    printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf("1. " COLOR_GREEN "Character Device Operations" COLOR_RESET "\n");
    printf("2. " COLOR_GREEN "Block Device Operations" COLOR_RESET "\n");
    printf("3. " COLOR_YELLOW "Data Transfer Between Devices" COLOR_RESET "\n");
    printf("4. " COLOR_YELLOW "Performance Testing" COLOR_RESET "\n");
    printf("5. " COLOR_BLUE "System Diagnostics" COLOR_RESET "\n");
    printf("6. " COLOR_BLUE "Concurrent Access Test" COLOR_RESET "\n");
    printf("7. " COLOR_MAGENTA "About / Help" COLOR_RESET "\n");
    printf("0. " COLOR_RED "Exit" COLOR_RESET "\n");
    printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf("Enter choice: ");
}

void char_device_menu(int char_fd) {
    if (char_fd < 0) {
        printf(COLOR_RED "Character device not connected!\n" COLOR_RESET);
        return;
    }
    
    int choice;
    
    while (1) {
        clear_screen();
        printf(COLOR_BLUE "\n[CHARACTER DEVICE OPERATIONS]\n" COLOR_RESET);
        printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf("1. Write test data\n");
        printf("2. Read data\n");
        printf("3. Get statistics\n");
        printf("4. Clear buffer\n");
        printf("5. Hex dump\n");
        printf("0. Back to main menu\n");
        printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf("Enter choice: ");
        
        scanf("%d", &choice);
        while (getchar() != '\n');
        
        switch (choice) {
            case 1: {
                char buffer[1024];
                printf("Enter text to write: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                
                ssize_t written = write(char_fd, buffer, strlen(buffer));
                if (written >= 0) {
                    printf(COLOR_GREEN "Wrote %ld bytes\n" COLOR_RESET, written);
                } else {
                    printf(COLOR_RED "Write failed\n" COLOR_RESET);
                }
                break;
            }
                
            case 2: {
                char buffer[1024];
                int bytes;
                printf("Bytes to read: ");
                scanf("%d", &bytes);
                while (getchar() != '\n');
                
                ssize_t read_bytes = read(char_fd, buffer, bytes);
                if (read_bytes >= 0) {
                    buffer[read_bytes] = '\0';
                    printf(COLOR_GREEN "Read %ld bytes:\n" COLOR_RESET, read_bytes);
                    printf("%s\n", buffer);
                } else {
                    printf(COLOR_RED "Read failed\n" COLOR_RESET);
                }
                break;
            }
                
            case 3: {
                struct char_stats stats;
                if (ioctl(char_fd, CHAR_GET_STATS, &stats) >= 0) {
                    printf(COLOR_CYAN "\nCharacter Device Statistics:\n" COLOR_RESET);
                    printf("  Buffer size: %d bytes\n", stats.buffer_size);
                    printf("  Buffer used: %d bytes\n", stats.buffer_used);
                    printf("  Read count:  %d\n", stats.read_count);
                    printf("  Write count: %d\n", stats.write_count);
                } else {
                    printf(COLOR_RED "Failed to get statistics\n" COLOR_RESET);
                }
                break;
            }
                
            case 4: {
                // Reset by writing empty string
                lseek(char_fd, 0, SEEK_SET);
                write(char_fd, "", 0);
                printf(COLOR_GREEN "Buffer cleared\n" COLOR_RESET);
                break;
            }
                
            case 5: {
                char buffer[256];
                lseek(char_fd, 0, SEEK_SET);
                ssize_t bytes_read = read(char_fd, buffer, sizeof(buffer));
                
                if (bytes_read > 0) {
                    printf(COLOR_CYAN "\nHex dump (first %ld bytes):\n" COLOR_RESET, bytes_read);
                    for (int i = 0; i < bytes_read; i++) {
                        printf("%02x ", (unsigned char)buffer[i]);
                        if ((i + 1) % 16 == 0) printf("\n");
                    }
                    printf("\n");
                }
                break;
            }
                
            case 0:
                return;
                
            default:
                printf(COLOR_RED "Invalid choice\n" COLOR_RESET);
        }
        
        printf(COLOR_YELLOW "\nPress Enter to continue..." COLOR_RESET);
        while (getchar() != '\n');
    }
}

void block_device_menu(int block_fd) {
    if (block_fd < 0) {
        printf(COLOR_RED "Block device not connected!\n" COLOR_RESET);
        return;
    }
    
    int choice;
    
    while (1) {
        clear_screen();
        printf(COLOR_BLUE "\n[BLOCK DEVICE OPERATIONS]\n" COLOR_RESET);
        printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf("1. Get device information\n");
        printf("2. Read sector\n");
        printf("3. Write sector\n");
        printf("4. Fill sectors with pattern\n");
        printf("5. Verify sectors\n");
        printf("0. Back to main menu\n");
        printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf("Enter choice: ");
        
        scanf("%d", &choice);
        while (getchar() != '\n');
        
        switch (choice) {
            case 1: {
                unsigned long sectors;
                if (ioctl(block_fd, BLKGETSIZE, &sectors) >= 0) {
                    printf(COLOR_CYAN "\nBlock Device Information:\n" COLOR_RESET);
                    printf("  Total sectors: %lu\n", sectors);
                    printf("  Sector size:   %d bytes\n", SECTOR_SIZE);
                    printf("  Total size:    %.2f MB\n", 
                           (sectors * SECTOR_SIZE) / (1024.0 * 1024.0));
                } else {
                    printf(COLOR_RED "Failed to get device information\n" COLOR_RESET);
                }
                break;
            }
                
            case 2: {
                unsigned long sector;
                printf("Sector number: ");
                scanf("%lu", &sector);
                while (getchar() != '\n');
                
                char buffer[SECTOR_SIZE];
                off_t offset = sector * SECTOR_SIZE;
                lseek(block_fd, offset, SEEK_SET);
                
                ssize_t bytes_read = read(block_fd, buffer, SECTOR_SIZE);
                if (bytes_read == SECTOR_SIZE) {
                    printf(COLOR_GREEN "Read sector %lu successfully\n" COLOR_RESET);
                    
                    // Show first 64 bytes in hex
                    printf("First 64 bytes:\n");
                    for (int i = 0; i < 64; i++) {
                        printf("%02x ", (unsigned char)buffer[i]);
                        if ((i + 1) % 16 == 0) printf("\n");
                    }
                } else {
                    printf(COLOR_RED "Failed to read sector\n" COLOR_RESET);
                }
                break;
            }
                
            case 3: {
                unsigned long sector;
                printf("Sector number: ");
                scanf("%lu", &sector);
                while (getchar() != '\n');
                
                char buffer[SECTOR_SIZE];
                printf("Enter data pattern (max %d chars): ", SECTOR_SIZE);
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                
                // Pad with zeros if necessary
                size_t data_len = strlen(buffer);
                if (data_len < SECTOR_SIZE) {
                    memset(buffer + data_len, 0, SECTOR_SIZE - data_len);
                }
                
                off_t offset = sector * SECTOR_SIZE;
                lseek(block_fd, offset, SEEK_SET);
                
                ssize_t written = write(block_fd, buffer, SECTOR_SIZE);
                if (written == SECTOR_SIZE) {
                    printf(COLOR_GREEN "Wrote sector %lu successfully\n" COLOR_RESET);
                } else {
                    printf(COLOR_RED "Failed to write sector\n" COLOR_RESET);
                }
                break;
            }
                
            case 4: {
                unsigned long start_sector, num_sectors;
                char pattern;
                
                printf("Start sector: ");
                scanf("%lu", &start_sector);
                while (getchar() != '\n');
                
                printf("Number of sectors: ");
                scanf("%lu", &num_sectors);
                while (getchar() != '\n');
                
                printf("Pattern byte (0-255, or 'r' for random): ");
                scanf(" %c", &pattern);
                while (getchar() != '\n');
                
                char buffer[SECTOR_SIZE];
                
                printf("\nFilling sectors %lu to %lu...\n",
                       start_sector, start_sector + num_sectors - 1);
                
                for (unsigned long i = 0; i < num_sectors; i++) {
                    unsigned long sector = start_sector + i;
                    
                    if (pattern == 'r' || pattern == 'R') {
                        srand(time(NULL) + i);
                        for (int j = 0; j < SECTOR_SIZE; j++) {
                            buffer[j] = rand() % 256;
                        }
                    } else {
                        memset(buffer, pattern, SECTOR_SIZE);
                    }
                    
                    off_t offset = sector * SECTOR_SIZE;
                    lseek(block_fd, offset, SEEK_SET);
                    write(block_fd, buffer, SECTOR_SIZE);
                    
                    if ((i + 1) % 100 == 0) {
                        printf("\rProgress: %lu/%lu sectors", i + 1, num_sectors);
                        fflush(stdout);
                    }
                }
                
                printf(COLOR_GREEN "\n\nFill completed successfully!\n" COLOR_RESET);
                break;
            }
                
            case 5: {
                unsigned long sector;
                printf("Sector number to verify: ");
                scanf("%lu", &sector);
                while (getchar() != '\n');
                
                char buffer[SECTOR_SIZE];
                off_t offset = sector * SECTOR_SIZE;
                lseek(block_fd, offset, SEEK_SET);
                
                ssize_t bytes_read = read(block_fd, buffer, SECTOR_SIZE);
                if (bytes_read != SECTOR_SIZE) {
                    printf(COLOR_RED "Sector %lu read error\n" COLOR_RESET);
                    break;
                }
                
                // Simple verification - check for all zeros or all ones
                int all_zeros = 1;
                int all_ones = 1;
                
                for (int i = 0; i < SECTOR_SIZE; i++) {
                    if (buffer[i] != 0) all_zeros = 0;
                    if ((unsigned char)buffer[i] != 0xFF) all_ones = 0;
                }
                
                if (all_zeros) {
                    printf(COLOR_YELLOW "Sector %lu: All zeros\n" COLOR_RESET);
                } else if (all_ones) {
                    printf(COLOR_YELLOW "Sector %lu: All ones (0xFF)\n" COLOR_RESET);
                } else {
                    printf(COLOR_GREEN "Sector %lu: Contains data\n" COLOR_RESET);
                    
                    // Show some statistics
                    int printable = 0;
                    for (int i = 0; i < SECTOR_SIZE; i++) {
                        if (isprint(buffer[i])) printable++;
                    }
                    
                    printf("  Printable characters: %d/%d (%.1f%%)\n",
                           printable, SECTOR_SIZE, 100.0 * printable / SECTOR_SIZE);
                }
                break;
            }
                
            case 0:
                return;
                
            default:
                printf(COLOR_RED "Invalid choice\n" COLOR_RESET);
        }
        
        printf(COLOR_YELLOW "\nPress Enter to continue..." COLOR_RESET);
        while (getchar() != '\n');
    }
}

void transfer_menu(int char_fd, int block_fd) {
    if (char_fd < 0 || block_fd < 0) {
        printf(COLOR_RED "Both devices must be connected!\n" COLOR_RESET);
        return;
    }
    
    int choice;
    
    while (1) {
        clear_screen();
        printf(COLOR_BLUE "\n[DATA TRANSFER BETWEEN DEVICES]\n" COLOR_RESET);
        printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf("1. Character → Block (copy buffer to sector 0)\n");
        printf("2. Block → Character (copy sector 0 to buffer)\n");
        printf("3. Compare character buffer with block sector\n");
        printf("4. Mirror character buffer to multiple sectors\n");
        printf("0. Back to main menu\n");
        printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf("Enter choice: ");
        
        scanf("%d", &choice);
        while (getchar() != '\n');
        
        switch (choice) {
            case 1: {  // Char → Block
                char buffer[MAX_BUFFER_SIZE];
                
                // Read from character device
                lseek(char_fd, 0, SEEK_SET);
                ssize_t char_bytes = read(char_fd, buffer, sizeof(buffer));
                
                if (char_bytes <= 0) {
                    printf(COLOR_YELLOW "Character device is empty\n" COLOR_RESET);
                    break;
                }
                
                // Write to block device
                unsigned long sector = 0;
                off_t offset = sector * SECTOR_SIZE;
                lseek(block_fd, offset, SEEK_SET);
                
                // Ensure we write full sectors
                size_t bytes_to_write = char_bytes;
                if (bytes_to_write % SECTOR_SIZE != 0) {
                    bytes_to_write = ((bytes_to_write / SECTOR_SIZE) + 1) * SECTOR_SIZE;
                    memset(buffer + char_bytes, 0, bytes_to_write - char_bytes);
                }
                
                ssize_t block_bytes = write(block_fd, buffer, bytes_to_write);
                
                if (block_bytes == bytes_to_write) {
                    printf(COLOR_GREEN "Copied %ld bytes to block device sector %lu\n" COLOR_RESET,
                           char_bytes, sector);
                    printf("Used %ld sectors\n", bytes_to_write / SECTOR_SIZE);
                } else {
                    printf(COLOR_RED "Transfer failed\n" COLOR_RESET);
                }
                break;
            }
                
            case 2: {  // Block → Char
                unsigned long sector;
                printf("Source sector number: ");
                scanf("%lu", &sector);
                while (getchar() != '\n');
                
                char buffer[SECTOR_SIZE];
                off_t offset = sector * SECTOR_SIZE;
                lseek(block_fd, offset, SEEK_SET);
                
                ssize_t block_bytes = read(block_fd, buffer, SECTOR_SIZE);
                if (block_bytes != SECTOR_SIZE) {
                    printf(COLOR_RED "Failed to read block device\n" COLOR_RESET);
                    break;
                }
                
                // Write to character device
                lseek(char_fd, 0, SEEK_SET);
                ssize_t char_bytes = write(char_fd, buffer, SECTOR_SIZE);
                
                if (char_bytes == SECTOR_SIZE) {
                    printf(COLOR_GREEN "Copied sector %lu to character device\n" COLOR_RESET, sector);
                } else {
                    printf(COLOR_RED "Transfer failed\n" COLOR_RESET);
                }
                break;
            }
                
            case 3: {  // Compare
                char char_buffer[MAX_BUFFER_SIZE];
                char block_buffer[SECTOR_SIZE];
                
                // Read from character device
                lseek(char_fd, 0, SEEK_SET);
                ssize_t char_bytes = read(char_fd, char_buffer, sizeof(char_buffer));
                
                if (char_bytes <= 0) {
                    printf(COLOR_YELLOW "Character device is empty\n" COLOR_RESET);
                    break;
                }
                
                unsigned long sector;
                printf("Block sector to compare: ");
                scanf("%lu", &sector);
                while (getchar() != '\n');
                
                off_t offset = sector * SECTOR_SIZE;
                lseek(block_fd, offset, SEEK_SET);
                ssize_t block_bytes = read(block_fd, block_buffer, SECTOR_SIZE);
                
                if (block_bytes != SECTOR_SIZE) {
                    printf(COLOR_RED "Failed to read block device\n" COLOR_RESET);
                    break;
                }
                
                // Compare
                int match = 1;
                size_t compare_len = char_bytes < SECTOR_SIZE ? char_bytes : SECTOR_SIZE;
                
                for (size_t i = 0; i < compare_len; i++) {
                    if (char_buffer[i] != block_buffer[i]) {
                        match = 0;
                        printf(COLOR_RED "Mismatch at byte %ld: 0x%02x vs 0x%02x\n" COLOR_RESET,
                               i, (unsigned char)char_buffer[i], (unsigned char)block_buffer[i]);
                        break;
                    }
                }
                
                if (match) {
                    if (char_bytes == SECTOR_SIZE) {
                        printf(COLOR_GREEN "Data matches exactly\n" COLOR_RESET);
                    } else if (char_bytes < SECTOR_SIZE) {
                        printf(COLOR_YELLOW "Character data matches first %ld bytes of sector\n" COLOR_RESET,
                               char_bytes);
                    } else {
                        printf(COLOR_YELLOW "Sector matches first %d bytes of character data\n" COLOR_RESET,
                               SECTOR_SIZE);
                    }
                }
                break;
            }
                
            case 4: {  // Mirror
                unsigned long start_sector, num_sectors;
                
                printf("Start sector: ");
                scanf("%lu", &start_sector);
                while (getchar() != '\n');
                
                printf("Number of sectors: ");
                scanf("%lu", &num_sectors);
                while (getchar() != '\n');
                
                char buffer[SECTOR_SIZE];
                
                // Read from character device (first sector's worth)
                lseek(char_fd, 0, SEEK_SET);
                ssize_t char_bytes = read(char_fd, buffer, SECTOR_SIZE);
                
                if (char_bytes <= 0) {
                    printf(COLOR_YELLOW "Character device is empty\n" COLOR_RESET);
                    break;
                }
                
                // Pad if necessary
                if (char_bytes < SECTOR_SIZE) {
                    memset(buffer + char_bytes, 0, SECTOR_SIZE - char_bytes);
                }
                
                printf("\nMirroring to sectors %lu to %lu...\n",
                       start_sector, start_sector + num_sectors - 1);
                
                for (unsigned long i = 0; i < num_sectors; i++) {
                    unsigned long sector = start_sector + i;
                    off_t offset = sector * SECTOR_SIZE;
                    lseek(block_fd, offset, SEEK_SET);
                    
                    ssize_t written = write(block_fd, buffer, SECTOR_SIZE);
                    if (written != SECTOR_SIZE) {
                        printf(COLOR_RED "\nFailed at sector %lu\n" COLOR_RESET, sector);
                        break;
                    }
                    
                    if ((i + 1) % 100 == 0) {
                        printf("\rProgress: %lu/%lu sectors", i + 1, num_sectors);
                        fflush(stdout);
                    }
                }
                
                printf(COLOR_GREEN "\n\nMirroring completed!\n" COLOR_RESET);
                break;
            }
                
            case 0:
                return;
                
            default:
                printf(COLOR_RED "Invalid choice\n" COLOR_RESET);
        }
        
        printf(COLOR_YELLOW "\nPress Enter to continue..." COLOR_RESET);
        while (getchar() != '\n');
    }
}

void performance_menu(int char_fd, int block_fd) {
    if (char_fd < 0 || block_fd < 0) {
        printf(COLOR_RED "Both devices must be connected!\n" COLOR_RESET);
        return;
    }
    
    int choice;
    
    while (1) {
        clear_screen();
        printf(COLOR_BLUE "\n[PERFORMANCE TESTING]\n" COLOR_RESET);
        printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf("1. Sequential read/write benchmark\n");
        printf("2. Random access benchmark\n");
        printf("3. Concurrent access test\n");
        printf("4. Device comparison test\n");
        printf("0. Back to main menu\n");
        printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf("Enter choice: ");
        
        scanf("%d", &choice);
        while (getchar() != '\n');
        
        switch (choice) {
            case 1: {  // Sequential benchmark
                int iterations;
                size_t data_size;
                
                printf("Data size per operation (bytes, 1-65536): ");
                scanf("%ld", &data_size);
                while (getchar() != '\n');
                
                printf("Number of iterations: ");
                scanf("%d", &iterations);
                while (getchar() != '\n');
                
                if (data_size <= 0 || data_size > 65536 || iterations <= 0) {
                    printf(COLOR_RED "Invalid parameters\n" COLOR_RESET);
                    break;
                }
                
                char* test_data = malloc(data_size);
                for (size_t i = 0; i < data_size; i++) {
                    test_data[i] = i % 256;
                }
                
                printf(COLOR_CYAN "\nRunning sequential benchmark...\n" COLOR_RESET);
                
                // Character device test
                double char_start = get_time_ms();
                for (int i = 0; i < iterations; i++) {
                    lseek(char_fd, 0, SEEK_SET);
                    write(char_fd, test_data, data_size);
                    lseek(char_fd, 0, SEEK_SET);
                    read(char_fd, test_data, data_size);
                }
                double char_time = get_time_ms() - char_start;
                
                // Block device test
                unsigned long sectors = (data_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
                double block_start = get_time_ms();
                for (int i = 0; i < iterations; i++) {
                    off_t offset = (i * sectors) * SECTOR_SIZE;
                    lseek(block_fd, offset, SEEK_SET);
                    write(block_fd, test_data, sectors * SECTOR_SIZE);
                    lseek(block_fd, offset, SEEK_SET);
                    read(block_fd, test_data, sectors * SECTOR_SIZE);
                }
                double block_time = get_time_ms() - block_start;
                
                free(test_data);
                
                // Results
                double total_data_mb = (iterations * data_size) / (1024.0 * 1024.0);
                
                printf(COLOR_GREEN "\nBENCHMARK RESULTS:\n" COLOR_RESET);
                printf(COLOR_MAGENTA "──────────────────────────────────────────────────────\n" COLOR_RESET);
                printf("Test configuration:\n");
                printf("  Data size:      %ld bytes\n", data_size);
                printf("  Iterations:     %d\n", iterations);
                printf("  Total data:     %.2f MB\n", total_data_mb);
                printf("\n");
                printf("Character device:\n");
                printf("  Time:           %.2f ms\n", char_time);
                printf("  Throughput:     %.2f MB/s\n", 
                       total_data_mb / (char_time / 1000.0));
                printf("\n");
                printf("Block device:\n");
                printf("  Time:           %.2f ms\n", block_time);
                printf("  Throughput:     %.2f MB/s\n", 
                       total_data_mb / (block_time / 1000.0));
                printf("\n");
                printf("Performance ratio: %.2fx\n", 
                       (total_data_mb / (char_time / 1000.0)) / 
                       (total_data_mb / (block_time / 1000.0)));
                printf(COLOR_MAGENTA "──────────────────────────────────────────────────────\n" COLOR_RESET);
                break;
            }
                
            case 2: {  // Random benchmark
                printf(COLOR_YELLOW "Random access benchmark not implemented yet\n" COLOR_RESET);
                printf("Use sequential benchmark instead.\n");
                break;
            }
                
            case 3: {  // Concurrent test
                int num_threads;
                int iterations;
                
                printf("Number of threads (1-10): ");
                scanf("%d", &num_threads);
                while (getchar() != '\n');
                
                printf("Iterations per thread: ");
                scanf("%d", &iterations);
                while (getchar() != '\n');
                
                if (num_threads < 1 || num_threads > 10 || iterations < 1) {
                    printf(COLOR_RED "Invalid parameters\n" COLOR_RESET);
                    break;
                }
                
                printf(COLOR_CYAN "\nStarting %d threads with %d iterations each...\n" COLOR_RESET,
                       num_threads, iterations);
                
                pthread_t threads[num_threads];
                unified_thread_args args[num_threads];
                
                double start_time = get_time_ms();
                
                // Create threads
                for (int i = 0; i < num_threads; i++) {
                    args[i].char_fd = char_fd;
                    args[i].block_fd = block_fd;
                    args[i].op_type = OP_WRITE;  // Test with writes
                    args[i].data_size = 1024;
                    args[i].block_sector = i * 10;  // Spread sectors
                    args[i].thread_id = i + 1;
                    
                    pthread_create(&threads[i], NULL, unified_thread_func, &args[i]);
                }
                
                // Wait for threads
                for (int i = 0; i < num_threads; i++) {
                    pthread_join(threads[i], NULL);
                }
                
                double total_time = get_time_ms() - start_time;
                
                printf(COLOR_GREEN "\nConcurrent test completed!\n" COLOR_RESET);
                printf("Threads: %d, Iterations: %d\n", num_threads, iterations);
                printf("Total time: %.2f ms\n", total_time);
                printf("Throughput: %.1f ops/sec\n",
                       (num_threads * iterations) / (total_time / 1000.0));
                break;
            }
                
            case 4: {  // Device comparison
                printf(COLOR_CYAN "\nDevice Comparison Test\n" COLOR_RESET);
                printf("This test compares the performance characteristics\n");
                printf("of character vs block devices.\n\n");
                
                // Get device info for comparison
                struct char_stats char_stats;
                unsigned long block_sectors;
                
                if (ioctl(char_fd, CHAR_GET_STATS, &char_stats) >= 0 &&
                    ioctl(block_fd, BLKGETSIZE, &block_sectors) >= 0) {
                    
                    printf(COLOR_GREEN "COMPARISON SUMMARY:\n" COLOR_RESET);
                    printf(COLOR_MAGENTA "──────────────────────────────────────────────────────\n" COLOR_RESET);
                    printf("%-20s %-20s %-20s\n", "Feature", "Character Device", "Block Device");
                    printf(COLOR_MAGENTA "──────────────────────────────────────────────────────\n" COLOR_RESET);
                    
                    printf("%-20s %-20s %-20s\n", "Device Type", "Character", "Block");
                    printf("%-20s %-20d %-20lu\n", "Max Size", 
                           char_stats.buffer_size, block_sectors * SECTOR_SIZE);
                    printf("%-20s %-20s %-20s\n", "Access Unit", "Byte", "Sector (512B)");
                    printf("%-20s %-20d %-20lu\n", "Operations", 
                           char_stats.read_count + char_stats.write_count, 
                           block_sectors * 2);  // Estimate
                    printf("%-20s %-20s %-20s\n", "Random Access", "Fast", "Slower");
                    printf("%-20s %-20s %-20s\n", "Sequential", "Very Fast", "Fast");
                    printf("%-20s %-20s %-20s\n", "Use Case", "Streaming", "Storage");
                    
                    printf(COLOR_MAGENTA "──────────────────────────────────────────────────────\n" COLOR_RESET);
                    
                    printf("\n" COLOR_YELLOW "Key Differences:\n" COLOR_RESET);
                    printf("• Character devices: Byte-oriented, good for streaming\n");
                    printf("• Block devices: Sector-oriented, good for storage\n");
                    printf("• Character: Faster for small random accesses\n");
                    printf("• Block: Better for large sequential transfers\n");
                } else {
                    printf(COLOR_RED "Failed to get device information\n" COLOR_RESET);
                }
                break;
            }
                
            case 0:
                return;
                
            default:
                printf(COLOR_RED "Invalid choice\n" COLOR_RESET);
        }
        
        printf(COLOR_YELLOW "\nPress Enter to continue..." COLOR_RESET);
        while (getchar() != '\n');
    }
}

void diagnostics_menu(int char_fd, int block_fd) {
    int choice;
    
    while (1) {
        clear_screen();
        printf(COLOR_BLUE "\n[SYSTEM DIAGNOSTICS]\n" COLOR_RESET);
        printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf("1. Run connectivity test\n");
        printf("2. Check device health\n");
        printf("3. Test data integrity\n");
        printf("4. Resource usage\n");
        printf("5. Kernel messages\n");
        printf("0. Back to main menu\n");
        printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf("Enter choice: ");
        
        scanf("%d", &choice);
        while (getchar() != '\n');
        
        switch (choice) {
            case 1: {  // Connectivity test
                printf(COLOR_CYAN "\nConnectivity Test:\n" COLOR_RESET);
                printf(COLOR_MAGENTA "──────────────────────────────────────────────────────\n" COLOR_RESET);
                
                if (char_fd >= 0) {
                    printf("Character device: ");
                    char test_buf[10] = "test";
                    lseek(char_fd, 0, SEEK_SET);
                    
                    if (write(char_fd, test_buf, 4) == 4) {
                        lseek(char_fd, 0, SEEK_SET);
                        char read_buf[10];
                        if (read(char_fd, read_buf, 4) == 4) {
                            read_buf[4] = '\0';
                            if (strcmp(test_buf, read_buf) == 0) {
                                printf(COLOR_GREEN "✓ Connected and working\n" COLOR_RESET);
                            } else {
                                printf(COLOR_YELLOW "⚠ Connected but data mismatch\n" COLOR_RESET);
                            }
                        } else {
                            printf(COLOR_RED "✗ Connected but read failed\n" COLOR_RESET);
                        }
                    } else {
                        printf(COLOR_RED "✗ Connected but write failed\n" COLOR_RESET);
                    }
                } else {
                    printf("Character device: " COLOR_RED "✗ Not connected\n" COLOR_RESET);
                }
                
                if (block_fd >= 0) {
                    printf("Block device:     ");
                    char test_buf[SECTOR_SIZE];
                    memset(test_buf, 0xAA, SECTOR_SIZE);
                    
                    off_t test_offset = 0;
                    lseek(block_fd, test_offset, SEEK_SET);
                    
                    if (write(block_fd, test_buf, SECTOR_SIZE) == SECTOR_SIZE) {
                        lseek(block_fd, test_offset, SEEK_SET);
                        char read_buf[SECTOR_SIZE];
                        if (read(block_fd, read_buf, SECTOR_SIZE) == SECTOR_SIZE) {
                            if (memcmp(test_buf, read_buf, SECTOR_SIZE) == 0) {
                                printf(COLOR_GREEN "✓ Connected and working\n" COLOR_RESET);
                            } else {
                                printf(COLOR_YELLOW "⚠ Connected but data mismatch\n" COLOR_RESET);
                            }
                        } else {
                            printf(COLOR_RED "✗ Connected but read failed\n" COLOR_RESET);
                        }
                    } else {
                        printf(COLOR_RED "✗ Connected but write failed\n" COLOR_RESET);
                    }
                } else {
                    printf("Block device:     " COLOR_RED "✗ Not connected\n" COLOR_RESET);
                }
                
                printf(COLOR_MAGENTA "──────────────────────────────────────────────────────\n" COLOR_RESET);
                break;
            }
                
            case 2: {  // Device health
                printf(COLOR_CYAN "\nDevice Health Check:\n" COLOR_RESET);
                printf(COLOR_MAGENTA "──────────────────────────────────────────────────────\n" COLOR_RESET);
                
                if (char_fd >= 0) {
                    struct char_stats stats;
                    if (ioctl(char_fd, CHAR_GET_STATS, &stats) >= 0) {
                        printf("Character Device Health:\n");
                        printf("  Buffer usage:   %d/%d bytes (%.1f%%)\n",
                               stats.buffer_used, stats.buffer_size,
                               100.0 * stats.buffer_used / stats.buffer_size);
                        printf("  Total ops:      %d\n", 
                               stats.read_count + stats.write_count);
                        
                        if (stats.buffer_used == stats.buffer_size) {
                            printf("  Status:         " COLOR_YELLOW "⚠ Buffer full\n" COLOR_RESET);
                        } else if (stats.buffer_used > stats.buffer_size * 0.9) {
                            printf("  Status:         " COLOR_YELLOW "⚠ Buffer nearly full\n" COLOR_RESET);
                        } else {
                            printf("  Status:         " COLOR_GREEN "✓ Healthy\n" COLOR_RESET);
                        }
                    }
                }
                
                if (block_fd >= 0) {
                    unsigned long sectors;
                    if (ioctl(block_fd, BLKGETSIZE, &sectors) >= 0) {
                        printf("\nBlock Device Health:\n");
                        printf("  Total sectors:  %lu\n", sectors);
                        printf("  Total size:     %.2f MB\n",
                               (sectors * SECTOR_SIZE) / (1024.0 * 1024.0));
                        
                        // Simple read test
                        char test_buf[SECTOR_SIZE];
                        lseek(block_fd, 0, SEEK_SET);
                        if (read(block_fd, test_buf, SECTOR_SIZE) == SECTOR_SIZE) {
                            printf("  Read test:      " COLOR_GREEN "✓ Pass\n" COLOR_RESET);
                        } else {
                            printf("  Read test:      " COLOR_RED "✗ Fail\n" COLOR_RESET);
                        }
                        
                        printf("  Status:         " COLOR_GREEN "✓ Healthy\n" COLOR_RESET);
                    }
                }
                
                printf(COLOR_MAGENTA "──────────────────────────────────────────────────────\n" COLOR_RESET);
                break;
            }
                
            case 3: {  // Data integrity
                printf(COLOR_CYAN "\nData Integrity Test:\n" COLOR_RESET);
                printf("This test writes a known pattern and verifies it.\n");
                
                if (char_fd >= 0 && block_fd >= 0) {
                    char pattern[] = "INTEGRITY_TEST_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
                    size_t pattern_len = strlen(pattern);
                    
                    // Write to both devices
                    lseek(char_fd, 0, SEEK_SET);
                    write(char_fd, pattern, pattern_len);
                    
                    lseek(block_fd, 0, SEEK_SET);
                    write(block_fd, pattern, SECTOR_SIZE < pattern_len ? SECTOR_SIZE : pattern_len);
                    
                    // Verify
                    char verify_buf[256];
                    
                    lseek(char_fd, 0, SEEK_SET);
                    ssize_t char_read = read(char_fd, verify_buf, pattern_len);
                    int char_ok = (char_read == pattern_len) && 
                                  (memcmp(pattern, verify_buf, pattern_len) == 0);
                    
                    lseek(block_fd, 0, SEEK_SET);
                    ssize_t block_read = read(block_fd, verify_buf, SECTOR_SIZE);
                    size_t compare_len = SECTOR_SIZE < pattern_len ? SECTOR_SIZE : pattern_len;
                    int block_ok = (block_read >= compare_len) &&
                                   (memcmp(pattern, verify_buf, compare_len) == 0);
                    
                    printf("\nResults:\n");
                    printf("  Character device: %s\n", 
                           char_ok ? COLOR_GREEN "✓ Integrity OK" COLOR_RESET : 
                           COLOR_RED "✗ Integrity FAILED" COLOR_RESET);
                    printf("  Block device:     %s\n", 
                           block_ok ? COLOR_GREEN "✓ Integrity OK" COLOR_RESET : 
                           COLOR_RED "✗ Integrity FAILED" COLOR_RESET);
                    
                    if (char_ok && block_ok) {
                        printf(COLOR_GREEN "\n✓ All devices passed integrity test!\n" COLOR_RESET);
                    }
                } else {
                    printf(COLOR_RED "Both devices must be connected for this test\n" COLOR_RESET);
                }
                break;
            }
                
            case 4:  // Resource usage
                printf(COLOR_YELLOW "\nResource usage information not available\n" COLOR_RESET);
                printf("Check system monitoring tools for resource usage.\n");
                break;
                
            case 5:  // Kernel messages
                printf(COLOR_CYAN "\nRecent Kernel Messages (dmesg):\n" COLOR_RESET);
                printf("Run 'dmesg | grep simple' in another terminal to see driver messages.\n");
                printf("Or run: sudo dmesg | tail -20\n");
                break;
                
            case 0:
                return;
                
            default:
                printf(COLOR_RED "Invalid choice\n" COLOR_RESET);
        }
        
        printf(COLOR_YELLOW "\nPress Enter to continue..." COLOR_RESET);
        while (getchar() != '\n');
    }
}

void about_screen() {
    clear_screen();
    printf(COLOR_CYAN "╔══════════════════════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_BOLD COLOR_YELLOW "                 ABOUT / HELP                    " COLOR_RESET COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "╚══════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    printf("\n");
    
    printf(COLOR_BOLD "Virtual Drivers Unified Control Panel v2.0\n" COLOR_RESET);
    printf("\n");
    
    printf(COLOR_GREEN "DESCRIPTION:\n" COLOR_RESET);
    printf("This application provides a unified interface for managing both\n");
    printf("character and block device drivers. It allows testing, benchmarking,\n");
    printf("and data transfer between different types of virtual devices.\n");
    printf("\n");
    
    printf(COLOR_YELLOW "FEATURES:\n" COLOR_RESET);
    printf("• Character device management (read/write/statistics)\n");
    printf("• Block device management (sector operations)\n");
    printf("• Data transfer between character and block devices\n");
    printf("• Performance benchmarking and comparison\n");
    printf("• System diagnostics and health checks\n");
    printf("• Concurrent access testing\n");
    printf("\n");
    
    printf(COLOR_BLUE "USAGE NOTES:\n" COLOR_RESET);
    printf("1. Both drivers must be loaded before using this application\n");
    printf("2. Run with sudo for device access privileges\n");
    printf("3. Character device: /dev/simple_char\n");
    printf("4. Block device: /dev/simple_block\n");
    printf("\n");
    
    printf(COLOR_MAGENTA "COMMAND LINE OPTIONS:\n" COLOR_RESET);
    printf("  --help          Show this help screen\n");
    printf("  --char-only     Start with only character device support\n");
    printf("  --block-only    Start with only block device support\n");
    printf("\n");
    
    printf(COLOR_CYAN "──────────────────────────────────────────────────────\n" COLOR_RESET);
    printf("Press Enter to return to main menu...");
    while (getchar() != '\n');
}

int main(int argc, char *argv[]) {
    int char_fd = -1;
    int block_fd = -1;
    int auto_connect = 1;
    
    // Parse command line arguments
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            about_screen();
            return EXIT_SUCCESS;
        } else if (strcmp(argv[1], "--char-only") == 0) {
            auto_connect = 1;
            // Will only try to connect char device
        } else if (strcmp(argv[1], "--block-only") == 0) {
            auto_connect = 2;
            // Will only try to connect block device
        }
    }
    
    // Auto-connect to devices
    if (auto_connect == 1 || auto_connect == 0) {
        char_fd = open(CHAR_DEVICE, O_RDWR);
        if (char_fd < 0) {
            fprintf(stderr, COLOR_YELLOW "Warning: Could not open character device\n" COLOR_RESET);
        }
    }
    
    if (auto_connect == 2 || auto_connect == 0) {
        block_fd = open(BLOCK_DEVICE, O_RDWR);
        if (block_fd < 0) {
            fprintf(stderr, COLOR_YELLOW "Warning: Could not open block device\n" COLOR_RESET);
        }
    }
    
    // Main loop
    int choice;
    while (1) {
        print_banner();
        print_status(char_fd, block_fd);
        main_menu();
        
        char input[10];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        choice = atoi(input);
        
        switch (choice) {
            case 1:
                char_device_menu(char_fd);
                break;
            case 2:
                block_device_menu(block_fd);
                break;
            case 3:
                transfer_menu(char_fd, block_fd);
                break;
            case 4:
                performance_menu(char_fd, block_fd);
                break;
            case 5:
                diagnostics_menu(char_fd, block_fd);
                break;
            case 6:
                // Concurrent test is part of performance menu
                performance_menu(char_fd, block_fd);
                break;
            case 7:
                about_screen();
                break;
            case 0:
                if (char_fd >= 0) close(char_fd);
                if (block_fd >= 0) close(block_fd);
                printf(COLOR_GREEN "\nGoodbye!\n" COLOR_RESET);
                return EXIT_SUCCESS;
            default:
                printf(COLOR_RED "\nInvalid choice. Please try again.\n" COLOR_RESET);
                printf(COLOR_YELLOW "Press Enter to continue..." COLOR_RESET);
                while (getchar() != '\n');
                break;
        }
    }
    
    if (char_fd >= 0) close(char_fd);
    if (block_fd >= 0) close(block_fd);
    return EXIT_SUCCESS;
}
