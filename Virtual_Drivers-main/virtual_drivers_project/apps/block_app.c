#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <sys/time.h>
#include <pthread.h>
#include <ctype.h>
#include <termios.h>
#include <errno.h>
#include <math.h>

#define DEVICE_PATH "/dev/simple_block"
#define SECTOR_SIZE 512
#define MAX_SECTORS 65536
#define MAX_BUFFER_SIZE (SECTOR_SIZE * 64)  // 32KB
#define MAX_THREADS 10

// Color codes for terminal output
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"

typedef struct {
    int fd;
    unsigned long start_sector;
    unsigned long num_sectors;
    int thread_id;
    int operation;  // 0=read, 1=write, 2=both
    char pattern;
} thread_args;

// Function prototypes
void clear_screen();
void print_banner();
void print_status(int fd);
void show_menu();
void get_device_info(int fd);
void read_sectors(int fd);
void write_sectors(int fd);
void verify_sectors(int fd);
void fill_pattern(int fd);
void benchmark(int fd);
void stress_test(int fd);
void concurrent_test(int fd);
void disk_scan(int fd);
void sector_editor(int fd);
void backup_restore(int fd);
void* block_thread_func(void* arg);
double get_time_ms();

// Utility functions
void clear_screen() {
    printf("\033[2J\033[1;1H");
}

void print_banner() {
    clear_screen();
    printf(COLOR_CYAN "╔══════════════════════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_BOLD COLOR_YELLOW "            ADVANCED BLOCK DEVICE MANAGER           " COLOR_RESET COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_WHITE "                 Virtual Block Driver v2.0               " COLOR_RESET COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "╚══════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    printf("\n");
}

void print_status(int fd) {
    unsigned long sectors;
    unsigned long long bytes;
    
    if (ioctl(fd, BLKGETSIZE, &sectors) >= 0) {
        if (ioctl(fd, BLKGETSIZE64, &bytes) < 0) {
            bytes = (unsigned long long)sectors * SECTOR_SIZE;
        }
        
        printf(COLOR_GREEN "Device: " COLOR_WHITE "%s" COLOR_RESET "\n", DEVICE_PATH);
        printf(COLOR_GREEN "Sectors: " COLOR_WHITE "%lu" COLOR_RESET "\n", sectors);
        printf(COLOR_GREEN "Size: " COLOR_WHITE "%.2f MB" COLOR_RESET "\n", bytes / (1024.0 * 1024.0));
        printf(COLOR_GREEN "Sector size: " COLOR_WHITE "%d bytes" COLOR_RESET "\n", SECTOR_SIZE);
    }
    printf("\n");
}

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

void* block_thread_func(void* arg) {
    thread_args* args = (thread_args*)arg;
    char buffer[SECTOR_SIZE];
    char thread_id_str[16];
    
    sprintf(thread_id_str, "[Thread %d]", args->thread_id);
    
    for (unsigned long i = 0; i < args->num_sectors; i++) {
        unsigned long sector = args->start_sector + i;
        off_t offset = sector * SECTOR_SIZE;
        
        if (args->operation == 0 || args->operation == 2) {  // Read
            lseek(args->fd, offset, SEEK_SET);
            ssize_t read_bytes = read(args->fd, buffer, SECTOR_SIZE);
            printf("%s Read sector %lu (%ld bytes)\n", 
                   thread_id_str, sector, read_bytes);
        }
        
        if (args->operation == 1 || args->operation == 2) {  // Write
            // Prepare data
            memset(buffer, args->pattern, SECTOR_SIZE);
            sprintf(buffer, "Thread %d Sector %lu", args->thread_id, sector);
            
            lseek(args->fd, offset, SEEK_SET);
            ssize_t written = write(args->fd, buffer, SECTOR_SIZE);
            printf("%s Wrote sector %lu (%ld bytes)\n", 
                   thread_id_str, sector, written);
        }
        
        usleep(1000);  // Small delay
    }
    
    return NULL;
}

void get_device_info(int fd) {
    unsigned long sectors;
    unsigned long long bytes;
    
    printf(COLOR_BLUE "\n[DEVICE INFORMATION]\n" COLOR_RESET);
    printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    if (ioctl(fd, BLKGETSIZE, &sectors) < 0) {
        printf(COLOR_RED "Failed to get device size\n" COLOR_RESET);
        return;
    }
    
    if (ioctl(fd, BLKGETSIZE64, &bytes) < 0) {
        bytes = (unsigned long long)sectors * SECTOR_SIZE;
    }
    
    printf(COLOR_GREEN "General Information:\n" COLOR_RESET);
    printf("  Device path:     %s\n", DEVICE_PATH);
    printf("  Sector size:     %d bytes\n", SECTOR_SIZE);
    printf("  Total sectors:   %lu\n", sectors);
    printf("  Total size:      %llu bytes\n", bytes);
    printf("                   %.2f KB\n", bytes / 1024.0);
    printf("                   %.2f MB\n", bytes / (1024.0 * 1024.0));
    
    printf("\n" COLOR_GREEN "Address Space:\n" COLOR_RESET);
    printf("  Start address:   0x00000000\n");
    printf("  End address:     0x%08llx\n", bytes - 1);
    printf("  Sector range:    0 - %lu\n", sectors - 1);
    
    printf("\n" COLOR_GREEN "Geometry:\n" COLOR_RESET);
    printf("  Cylinders:       %lu\n", sectors / (63 * 255));  // Typical geometry
    printf("  Heads:           255\n");
    printf("  Sectors/track:   63\n");
    
    // Check if device is readable/writable
    printf("\n" COLOR_GREEN "Access Test:\n" COLOR_RESET);
    
    char test_buffer[SECTOR_SIZE];
    off_t test_offset = 0;
    
    lseek(fd, test_offset, SEEK_SET);
    ssize_t read_test = read(fd, test_buffer, SECTOR_SIZE);
    if (read_test == SECTOR_SIZE) {
        printf("  Read test:       " COLOR_GREEN "✓ PASS" COLOR_RESET "\n");
    } else {
        printf("  Read test:       " COLOR_RED "✗ FAIL" COLOR_RESET "\n");
    }
    
    lseek(fd, test_offset, SEEK_SET);
    memset(test_buffer, 0xAA, SECTOR_SIZE);
    ssize_t write_test = write(fd, test_buffer, SECTOR_SIZE);
    if (write_test == SECTOR_SIZE) {
        printf("  Write test:      " COLOR_GREEN "✓ PASS" COLOR_RESET "\n");
    } else {
        printf("  Write test:      " COLOR_RED "✗ FAIL" COLOR_RESET "\n");
    }
    
    printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
}

void read_sectors(int fd) {
    unsigned long start_sector, num_sectors;
    int display_mode;
    
    printf(COLOR_BLUE "\n[READ SECTORS]\n" COLOR_RESET);
    printf("Start sector: ");
    scanf("%lu", &start_sector);
    while (getchar() != '\n');
    
    printf("Number of sectors (1-%d): ", MAX_BUFFER_SIZE / SECTOR_SIZE);
    scanf("%lu", &num_sectors);
    while (getchar() != '\n');
    
    if (num_sectors == 0 || num_sectors > MAX_BUFFER_SIZE / SECTOR_SIZE) {
        printf(COLOR_RED "Invalid number of sectors\n" COLOR_RESET);
        return;
    }
    
    printf("Display mode:\n");
    printf("1. Hex dump\n");
    printf("2. ASCII text\n");
    printf("3. Raw binary\n");
    printf("4. Compare sectors\n");
    printf("Choice: ");
    scanf("%d", &display_mode);
    while (getchar() != '\n');
    
    size_t buffer_size = num_sectors * SECTOR_SIZE;
    char* buffer = malloc(buffer_size);
    if (!buffer) {
        printf(COLOR_RED "Memory allocation failed\n" COLOR_RESET);
        return;
    }
    
    off_t offset = start_sector * SECTOR_SIZE;
    lseek(fd, offset, SEEK_SET);
    
    double start_time = get_time_ms();
    ssize_t bytes_read = read(fd, buffer, buffer_size);
    double end_time = get_time_ms();
    
    if (bytes_read != buffer_size) {
        printf(COLOR_RED "Read error: expected %ld bytes, got %ld\n" COLOR_RESET,
               buffer_size, bytes_read);
        free(buffer);
        return;
    }
    
    printf(COLOR_GREEN "\nRead %lu sectors (%ld bytes) in %.2f ms (%.2f MB/s)\n" COLOR_RESET,
           num_sectors, bytes_read, end_time - start_time,
           (bytes_read / (1024.0 * 1024.0)) / ((end_time - start_time) / 1000.0));
    
    if (display_mode == 1) {
        // Hex dump
        printf(COLOR_CYAN "\nHex Dump (first 1024 bytes):\n" COLOR_RESET);
        printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        
        int display_bytes = bytes_read > 1024 ? 1024 : bytes_read;
        for (int i = 0; i < display_bytes; i++) {
            if (i % 16 == 0) {
                if (i > 0) {
                    // Show ASCII representation
                    printf(" | ");
                    for (int j = i-16; j < i; j++) {
                        unsigned char c = buffer[j];
                        printf("%c", isprint(c) ? c : '.');
                    }
                }
                printf("\n" COLOR_YELLOW "%08lx: " COLOR_RESET, offset + i);
            }
            printf("%02x ", (unsigned char)buffer[i]);
        }
        
        // Last line ASCII
        int last_start = (display_bytes / 16) * 16;
        if (last_start < display_bytes) {
            for (int i = last_start; i < display_bytes; i++) {
                if (i % 16 == 0) {
                    printf("\n" COLOR_YELLOW "%08lx: " COLOR_RESET, offset + i);
                }
                printf("%02x ", (unsigned char)buffer[i]);
            }
            printf(" | ");
            for (int j = last_start; j < display_bytes; j++) {
                unsigned char c = buffer[j];
                printf("%c", isprint(c) ? c : '.');
            }
        }
        
        printf("\n" COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        
        if (bytes_read > 1024) {
            printf(COLOR_YELLOW "... %ld more bytes ...\n" COLOR_RESET, bytes_read - 1024);
        }
        
    } else if (display_mode == 2) {
        // ASCII text
        printf(COLOR_CYAN "\nASCII Text (first 1024 bytes):\n" COLOR_RESET);
        printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        
        int is_text = 1;
        for (int i = 0; i < bytes_read && i < 1024; i++) {
            if (!isprint(buffer[i]) && !isspace(buffer[i]) && buffer[i] != '\0') {
                is_text = 0;
                break;
            }
        }
        
        if (is_text) {
            int display_bytes = bytes_read > 1024 ? 1024 : bytes_read;
            for (int i = 0; i < display_bytes; i++) {
                putchar(buffer[i]);
                if ((i + 1) % 80 == 0) printf("\n");
            }
        } else {
            printf(COLOR_YELLOW "Data contains non-printable characters\n" COLOR_RESET);
            printf("Use hex dump mode instead.\n");
        }
        
        printf("\n" COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        
    } else if (display_mode == 3) {
        // Raw binary (save to file)
        char filename[256];
        printf("Enter filename to save raw data: ");
        scanf("%255s", filename);
        while (getchar() != '\n');
        
        FILE* file = fopen(filename, "wb");
        if (!file) {
            printf(COLOR_RED "Failed to open file: %s\n" COLOR_RESET, strerror(errno));
        } else {
            fwrite(buffer, 1, bytes_read, file);
            fclose(file);
            printf(COLOR_GREEN "Saved %ld bytes to %s\n" COLOR_RESET, bytes_read, filename);
        }
        
    } else if (display_mode == 4) {
        // Compare with another range
        unsigned long compare_start;
        printf("Enter sector to compare with: ");
        scanf("%lu", &compare_start);
        while (getchar() != '\n');
        
        char* compare_buffer = malloc(buffer_size);
        if (!compare_buffer) {
            printf(COLOR_RED "Memory allocation failed\n" COLOR_RESET);
            free(buffer);
            return;
        }
        
        off_t compare_offset = compare_start * SECTOR_SIZE;
        lseek(fd, compare_offset, SEEK_SET);
        ssize_t compare_read = read(fd, compare_buffer, buffer_size);
        
        if (compare_read != buffer_size) {
            printf(COLOR_RED "Failed to read comparison data\n" COLOR_RESET);
        } else {
            printf("\n" COLOR_CYAN "Comparison Results:\n" COLOR_RESET);
            
            int differences = 0;
            for (size_t i = 0; i < buffer_size; i++) {
                if (buffer[i] != compare_buffer[i]) {
                    if (differences < 10) {  // Show first 10 differences
                        printf("Difference at byte %ld (sector %lu, offset %ld): "
                               "0x%02x vs 0x%02x\n",
                               i, start_sector + i / SECTOR_SIZE, i % SECTOR_SIZE,
                               (unsigned char)buffer[i], (unsigned char)compare_buffer[i]);
                    }
                    differences++;
                }
            }
            
            if (differences == 0) {
                printf(COLOR_GREEN "Sectors are identical\n" COLOR_RESET);
            } else {
                printf(COLOR_YELLOW "\nTotal differences: %d (%.4f%%)\n" COLOR_RESET,
                       differences, 100.0 * differences / buffer_size);
            }
        }
        
        free(compare_buffer);
    }
    
    free(buffer);
}

void write_sectors(int fd) {
    unsigned long start_sector, num_sectors;
    int write_mode;
    char filename[256];
    
    printf(COLOR_BLUE "\n[WRITE SECTORS]\n" COLOR_RESET);
    printf("Start sector: ");
    scanf("%lu", &start_sector);
    while (getchar() != '\n');
    
    printf("Number of sectors (1-%d): ", MAX_BUFFER_SIZE / SECTOR_SIZE);
    scanf("%lu", &num_sectors);
    while (getchar() != '\n');
    
    if (num_sectors == 0 || num_sectors > MAX_BUFFER_SIZE / SECTOR_SIZE) {
        printf(COLOR_RED "Invalid number of sectors\n" COLOR_RESET);
        return;
    }
    
    printf("Write mode:\n");
    printf("1. Fill with pattern\n");
    printf("2. Load from file\n");
    printf("3. Random data\n");
    printf("4. Incremental pattern\n");
    printf("Choice: ");
    scanf("%d", &write_mode);
    while (getchar() != '\n');
    
    size_t buffer_size = num_sectors * SECTOR_SIZE;
    char* buffer = malloc(buffer_size);
    if (!buffer) {
        printf(COLOR_RED "Memory allocation failed\n" COLOR_RESET);
        return;
    }
    
    // Prepare data based on mode
    switch (write_mode) {
        case 1: {  // Pattern fill
            char pattern;
            printf("Enter pattern byte (0-255, or 'r' for random): ");
            scanf(" %c", &pattern);
            while (getchar() != '\n');
            
            if (pattern == 'r' || pattern == 'R') {
                srand(time(NULL));
                for (size_t i = 0; i < buffer_size; i++) {
                    buffer[i] = rand() % 256;
                }
                printf("Filling with random data\n");
            } else {
                memset(buffer, pattern, buffer_size);
                printf("Filling with byte 0x%02x\n", (unsigned char)pattern);
            }
            break;
        }
            
        case 2: {  // Load from file
            printf("Enter filename: ");
            scanf("%255s", filename);
            while (getchar() != '\n');
            
            FILE* file = fopen(filename, "rb");
            if (!file) {
                printf(COLOR_RED "Failed to open file: %s\n" COLOR_RESET, strerror(errno));
                free(buffer);
                return;
            }
            
            size_t file_read = fread(buffer, 1, buffer_size, file);
            fclose(file);
            
            if (file_read < buffer_size) {
                // Pad with zeros if file is smaller
                memset(buffer + file_read, 0, buffer_size - file_read);
                printf("Loaded %ld bytes from file, padded with zeros\n", file_read);
            } else {
                printf("Loaded %ld bytes from file\n", file_read);
            }
            break;
        }
            
        case 3:  // Random data
            srand(time(NULL));
            for (size_t i = 0; i < buffer_size; i++) {
                buffer[i] = rand() % 256;
            }
            printf("Generated random data\n");
            break;
            
        case 4:  // Incremental pattern
            for (size_t i = 0; i < buffer_size; i++) {
                buffer[i] = i % 256;
            }
            printf("Generated incremental pattern (0x00 to 0xff repeating)\n");
            break;
            
        default:
            printf(COLOR_RED "Invalid write mode\n" COLOR_RESET);
            free(buffer);
            return;
    }
    
    // Confirmation
    unsigned long long bytes_to_write = buffer_size;
    printf("\nAbout to write %lu sectors (%llu bytes) starting at sector %lu\n",
           num_sectors, bytes_to_write, start_sector);
    printf("This will overwrite existing data. Continue? (y/N): ");
    
    char confirm;
    scanf(" %c", &confirm);
    while (getchar() != '\n');
    
    if (confirm != 'y' && confirm != 'Y') {
        printf(COLOR_YELLOW "Write cancelled\n" COLOR_RESET);
        free(buffer);
        return;
    }
    
    // Perform write
    off_t offset = start_sector * SECTOR_SIZE;
    lseek(fd, offset, SEEK_SET);
    
    double start_time = get_time_ms();
    ssize_t bytes_written = write(fd, buffer, buffer_size);
    double end_time = get_time_ms();
    
    if (bytes_written != buffer_size) {
        printf(COLOR_RED "Write error: expected %ld bytes, wrote %ld\n" COLOR_RESET,
               buffer_size, bytes_written);
    } else {
        printf(COLOR_GREEN "\nSuccessfully wrote %lu sectors (%ld bytes) in %.2f ms (%.2f MB/s)\n" COLOR_RESET,
               num_sectors, bytes_written, end_time - start_time,
               (bytes_written / (1024.0 * 1024.0)) / ((end_time - start_time) / 1000.0));
    }
    
    free(buffer);
}

void verify_sectors(int fd) {
    unsigned long start_sector, num_sectors;
    int pattern_type;
    
    printf(COLOR_BLUE "\n[VERIFY SECTORS]\n" COLOR_RESET);
    printf("Start sector: ");
    scanf("%lu", &start_sector);
    while (getchar() != '\n');
    
    printf("Number of sectors (1-%d): ", MAX_BUFFER_SIZE / SECTOR_SIZE);
    scanf("%lu", &num_sectors);
    while (getchar() != '\n');
    
    printf("Expected pattern:\n");
    printf("1. All zeros\n");
    printf("2. All ones (0xFF)\n");
    printf("3. Checkerboard (0xAA)\n");
    printf("4. Incremental (0x00, 0x01, ...)\n");
    printf("5. Specific byte\n");
    printf("Choice: ");
    scanf("%d", &pattern_type);
    while (getchar() != '\n');
    
    size_t buffer_size = num_sectors * SECTOR_SIZE;
    char* buffer = malloc(buffer_size);
    char* expected = malloc(buffer_size);
    
    if (!buffer || !expected) {
        printf(COLOR_RED "Memory allocation failed\n" COLOR_RESET);
        free(buffer);
        free(expected);
        return;
    }
    
    // Prepare expected pattern
    switch (pattern_type) {
        case 1:
            memset(expected, 0, buffer_size);
            printf("Verifying all zeros\n");
            break;
        case 2:
            memset(expected, 0xFF, buffer_size);
            printf("Verifying all ones (0xFF)\n");
            break;
        case 3:
            for (size_t i = 0; i < buffer_size; i++) {
                expected[i] = (i % 2 == 0) ? 0xAA : 0x55;
            }
            printf("Verifying checkerboard pattern (0xAA/0x55)\n");
            break;
        case 4:
            for (size_t i = 0; i < buffer_size; i++) {
                expected[i] = i % 256;
            }
            printf("Verifying incremental pattern\n");
            break;
        case 5: {
            char pattern_byte;
            printf("Enter expected byte value (0-255): ");
            scanf(" %c", &pattern_byte);
            while (getchar() != '\n');
            memset(expected, pattern_byte, buffer_size);
            printf("Verifying byte 0x%02x\n", (unsigned char)pattern_byte);
            break;
        }
        default:
            printf(COLOR_RED "Invalid pattern type\n" COLOR_RESET);
            free(buffer);
            free(expected);
            return;
    }
    
    // Read data from device
    off_t offset = start_sector * SECTOR_SIZE;
    lseek(fd, offset, SEEK_SET);
    ssize_t bytes_read = read(fd, buffer, buffer_size);
    
    if (bytes_read != buffer_size) {
        printf(COLOR_RED "Read error: expected %ld bytes, got %ld\n" COLOR_RESET,
               buffer_size, bytes_read);
        free(buffer);
        free(expected);
        return;
    }
    
    // Compare
    int errors = 0;
    int first_error = -1;
    unsigned char first_expected = 0, first_actual = 0;
    
    for (size_t i = 0; i < buffer_size; i++) {
        if (buffer[i] != expected[i]) {
            if (errors == 0) {
                first_error = i;
                first_expected = expected[i];
                first_actual = buffer[i];
            }
            errors++;
        }
    }
    
    printf("\n" COLOR_CYAN "VERIFICATION RESULTS:\n" COLOR_RESET);
    printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    if (errors == 0) {
        printf(COLOR_GREEN "✓ All sectors verified successfully\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "✗ Verification failed\n" COLOR_RESET);
        printf("  Total sectors checked: %lu\n", num_sectors);
        printf("  Total bytes checked:   %ld\n", buffer_size);
        printf("  Errors found:          %d\n", errors);
        printf("  Error rate:            %.6f%%\n", 100.0 * errors / buffer_size);
        printf("\n");
        printf("  First error at byte %d (sector %lu, offset %d)\n",
               first_error, 
               start_sector + first_error / SECTOR_SIZE,
               first_error % SECTOR_SIZE);
        printf("  Expected: 0x%02x, Actual: 0x%02x\n",
               first_expected, first_actual);
        
        // Show context around first error
        printf("\n" COLOR_YELLOW "Context around first error:\n" COLOR_RESET);
        int context_start = first_error - 16;
        int context_end = first_error + 16;
        
        if (context_start < 0) context_start = 0;
        if (context_end > buffer_size) context_end = buffer_size;
        
        for (int i = context_start; i < context_end; i++) {
            if (i % 16 == 0) {
                printf("\n%08lx: ", offset + i);
            }
            
            if (i == first_error) {
                printf(COLOR_RED "%02x " COLOR_RESET, (unsigned char)buffer[i]);
            } else {
                printf("%02x ", (unsigned char)buffer[i]);
            }
            
            if (i == context_start + 7) printf(" ");
        }
        printf("\n");
    }
    
    printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    free(buffer);
    free(expected);
}

void fill_pattern(int fd) {
    unsigned long start_sector, num_sectors;
    int pattern_type;
    char confirm;
    
    printf(COLOR_BLUE "\n[FILL WITH PATTERN]\n" COLOR_RESET);
    printf("Start sector: ");
    scanf("%lu", &start_sector);
    while (getchar() != '\n');
    
    printf("Number of sectors: ");
    scanf("%lu", &num_sectors);
    while (getchar() != '\n');
    
    if (num_sectors == 0) {
        printf(COLOR_RED "Invalid number of sectors\n" COLOR_RESET);
        return;
    }
    
    printf("Pattern types:\n");
    printf("1. Zero fill (secure erase)\n");
    printf("2. Random data (crypto erase)\n");
    printf("3. Checkerboard (0xAA/0x55)\n");
    printf("4. Walking ones\n");
    printf("5. Walking zeros\n");
    printf("6. Specific byte\n");
    printf("Choice: ");
    scanf("%d", &pattern_type);
    while (getchar() != '\n');
    
    size_t buffer_size = SECTOR_SIZE;  // Fill one sector at a time
    char* buffer = malloc(buffer_size);
    if (!buffer) {
        printf(COLOR_RED "Memory allocation failed\n" COLOR_RESET);
        return;
    }
    
    // Confirmation
    unsigned long long total_bytes = num_sectors * SECTOR_SIZE;
    printf("\nAbout to fill %lu sectors (%llu bytes = %.2f MB)\n",
           num_sectors, total_bytes, total_bytes / (1024.0 * 1024.0));
    printf("This will overwrite existing data. Continue? (y/N): ");
    scanf(" %c", &confirm);
    while (getchar() != '\n');
    
    if (confirm != 'y' && confirm != 'Y') {
        printf(COLOR_YELLOW "Operation cancelled\n" COLOR_RESET);
        free(buffer);
        return;
    }
    
    printf("\n" COLOR_CYAN "Filling sectors %lu to %lu...\n" COLOR_RESET,
           start_sector, start_sector + num_sectors - 1);
    
    double start_time = get_time_ms();
    unsigned long sectors_written = 0;
    
    for (unsigned long i = 0; i < num_sectors; i++) {
        unsigned long sector = start_sector + i;
        off_t offset = sector * SECTOR_SIZE;
        
        // Prepare pattern for this sector
        switch (pattern_type) {
            case 1:  // Zero fill
                memset(buffer, 0, buffer_size);
                break;
            case 2:  // Random
                srand(time(NULL) + i);
                for (size_t j = 0; j < buffer_size; j++) {
                    buffer[j] = rand() % 256;
                }
                break;
            case 3:  // Checkerboard
                for (size_t j = 0; j < buffer_size; j++) {
                    buffer[j] = (j % 2 == 0) ? 0xAA : 0x55;
                }
                break;
            case 4:  // Walking ones
                for (size_t j = 0; j < buffer_size; j++) {
                    buffer[j] = 1 << (j % 8);
                }
                break;
            case 5:  // Walking zeros
                for (size_t j = 0; j < buffer_size; j++) {
                    buffer[j] = ~(1 << (j % 8));
                }
                break;
            case 6:  // Specific byte
                printf("Enter byte value (0-255): ");
                char byte_val;
                scanf(" %c", &byte_val);
                while (getchar() != '\n');
                memset(buffer, byte_val, buffer_size);
                break;
        }
        
        // Write sector
        lseek(fd, offset, SEEK_SET);
        ssize_t written = write(fd, buffer, buffer_size);
        
        if (written != buffer_size) {
            printf(COLOR_RED "\nError writing sector %lu\n" COLOR_RESET, sector);
            break;
        }
        
        sectors_written++;
        
        // Progress display
        if ((i + 1) % 100 == 0 || i == num_sectors - 1) {
            double progress = 100.0 * (i + 1) / num_sectors;
            double elapsed = get_time_ms() - start_time;
            double speed = (sectors_written * SECTOR_SIZE / 1024.0 / 1024.0) / (elapsed / 1000.0);
            
            printf("\rProgress: %6.2f%% | Sectors: %lu/%lu | Speed: %.2f MB/s",
                   progress, i + 1, num_sectors, speed);
            fflush(stdout);
        }
    }
    
    double end_time = get_time_ms();
    double total_time = end_time - start_time;
    double avg_speed = (sectors_written * SECTOR_SIZE / 1024.0 / 1024.0) / (total_time / 1000.0);
    
    printf("\n\n" COLOR_GREEN "Fill completed successfully!\n" COLOR_RESET);
    printf("Sectors written: %lu/%lu\n", sectors_written, num_sectors);
    printf("Total time:      %.2f seconds\n", total_time / 1000.0);
    printf("Average speed:   %.2f MB/s\n", avg_speed);
    
    free(buffer);
}

void benchmark(int fd) {
    int test_size_kb;
    int iterations;
    int access_pattern;
    
    printf(COLOR_BLUE "\n[BENCHMARK TOOL]\n" COLOR_RESET);
    printf("Test size (KB, 1-8192): ");
    scanf("%d", &test_size_kb);
    while (getchar() != '\n');
    
    if (test_size_kb < 1 || test_size_kb > 8192) {
        printf(COLOR_RED "Invalid test size\n" COLOR_RESET);
        return;
    }
    
    printf("Number of iterations: ");
    scanf("%d", &iterations);
    while (getchar() != '\n');
    
    if (iterations < 1) {
        printf(COLOR_RED "Invalid iterations\n" COLOR_RESET);
        return;
    }
    
    printf("Access pattern:\n");
    printf("1. Sequential read/write\n");
    printf("2. Random read/write\n");
    printf("3. Mixed operations\n");
    printf("Choice: ");
    scanf("%d", &access_pattern);
    while (getchar() != '\n');
    
    size_t test_size = test_size_kb * 1024;
    char* test_data = malloc(test_size);
    if (!test_data) {
        printf(COLOR_RED "Memory allocation failed\n" COLOR_RESET);
        return;
    }
    
    // Fill test data
    for (size_t i = 0; i < test_size; i++) {
        test_data[i] = i % 256;
    }
    
    printf("\n" COLOR_CYAN "Running benchmark...\n" COLOR_RESET);
    printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    // Get device size
    unsigned long total_sectors;
    ioctl(fd, BLKGETSIZE, &total_sectors);
    unsigned long max_sector = total_sectors - 1;
    
    // Test parameters
    double seq_write_time = 0, seq_read_time = 0;
    double rand_write_time = 0, rand_read_time = 0;
    
    srand(time(NULL));
    
    // Sequential access test
    if (access_pattern == 1 || access_pattern == 3) {
        printf("\nSequential access test...\n");
        
        // Sequential write
        double start = get_time_ms();
        for (int i = 0; i < iterations; i++) {
            off_t offset = (i * test_size) % (max_sector * SECTOR_SIZE);
            lseek(fd, offset, SEEK_SET);
            write(fd, test_data, test_size);
        }
        seq_write_time = get_time_ms() - start;
        
        // Sequential read
        start = get_time_ms();
        for (int i = 0; i < iterations; i++) {
            off_t offset = (i * test_size) % (max_sector * SECTOR_SIZE);
            lseek(fd, offset, SEEK_SET);
            read(fd, test_data, test_size);
        }
        seq_read_time = get_time_ms() - start;
    }
    
    // Random access test
    if (access_pattern == 2 || access_pattern == 3) {
        printf("Random access test...\n");
        
        // Random write
        double start = get_time_ms();
        for (int i = 0; i < iterations; i++) {
            off_t offset = (rand() % max_sector) * SECTOR_SIZE;
            lseek(fd, offset, SEEK_SET);
            write(fd, test_data, test_size);
        }
        rand_write_time = get_time_ms() - start;
        
        // Random read
        start = get_time_ms();
        for (int i = 0; i < iterations; i++) {
            off_t offset = (rand() % max_sector) * SECTOR_SIZE;
            lseek(fd, offset, SEEK_SET);
            read(fd, test_data, test_size);
        }
        rand_read_time = get_time_ms() - start;
    }
    
    free(test_data);
    
    // Calculate results
    double total_data_mb = (iterations * test_size) / (1024.0 * 1024.0);
    
    printf("\n" COLOR_GREEN "BENCHMARK RESULTS\n" COLOR_RESET);
    printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    printf("Test configuration:\n");
    printf("  Test size:        %d KB\n", test_size_kb);
    printf("  Iterations:       %d\n", iterations);
    printf("  Total data:       %.2f MB\n", total_data_mb);
    
    if (seq_write_time > 0) {
        printf("\n" COLOR_YELLOW "Sequential Access:\n" COLOR_RESET);
        printf("  Write speed:      %.2f MB/s\n", total_data_mb / (seq_write_time / 1000.0));
        printf("  Read speed:       %.2f MB/s\n", total_data_mb / (seq_read_time / 1000.0));
        printf("  Write latency:    %.3f ms/op\n", seq_write_time / iterations);
        printf("  Read latency:     %.3f ms/op\n", seq_read_time / iterations);
    }
    
    if (rand_write_time > 0) {
        printf("\n" COLOR_YELLOW "Random Access:\n" COLOR_RESET);
        printf("  Write speed:      %.2f MB/s\n", total_data_mb / (rand_write_time / 1000.0));
        printf("  Read speed:       %.2f MB/s\n", total_data_mb / (rand_read_time / 1000.0));
        printf("  Write latency:    %.3f ms/op\n", rand_write_time / iterations);
        printf("  Read latency:     %.3f ms/op\n", rand_read_time / iterations);
    }
    
    printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
}

void concurrent_test(int fd) {
    int num_threads;
    unsigned long sectors_per_thread;
    int operation;
    
    printf(COLOR_BLUE "\n[CONCURRENT ACCESS TEST]\n" COLOR_RESET);
    printf("Number of threads (1-%d): ", MAX_THREADS);
    scanf("%d", &num_threads);
    while (getchar() != '\n');
    
    if (num_threads < 1 || num_threads > MAX_THREADS) {
        printf(COLOR_RED "Invalid number of threads\n" COLOR_RESET);
        return;
    }
    
    printf("Sectors per thread: ");
    scanf("%lu", &sectors_per_thread);
    while (getchar() != '\n');
    
    printf("Operation:\n");
    printf("1. Read only\n");
    printf("2. Write only\n");
    printf("3. Read and write\n");
    printf("Choice: ");
    scanf("%d", &operation);
    while (getchar() != '\n');
    
    // Get device size to ensure we don't exceed it
    unsigned long total_sectors;
    ioctl(fd, BLKGETSIZE, &total_sectors);
    
    if (sectors_per_thread * num_threads > total_sectors) {
        printf(COLOR_RED "Requested sectors exceed device capacity\n" COLOR_RESET);
        printf("Device has %lu sectors, requested %lu\n", 
               total_sectors, sectors_per_thread * num_threads);
        return;
    }
    
    printf("\n" COLOR_CYAN "Starting %d threads, %lu sectors each...\n" COLOR_RESET,
           num_threads, sectors_per_thread);
    
    pthread_t threads[MAX_THREADS];
    thread_args args[MAX_THREADS];
    
    double start_time = get_time_ms();
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        args[i].fd = fd;
        args[i].thread_id = i + 1;
        args[i].start_sector = (i * sectors_per_thread) % total_sectors;
        args[i].num_sectors = sectors_per_thread;
        args[i].operation = operation - 1;  // Convert to 0-based
        args[i].pattern = 'A' + (i % 26);
        
        if (pthread_create(&threads[i], NULL, block_thread_func, &args[i]) != 0) {
            printf(COLOR_RED "Failed to create thread %d\n" COLOR_RESET, i);
            return;
        }
    }
    
    // Wait for all threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    double end_time = get_time_ms();
    double total_time = end_time - start_time;
    
    unsigned long total_sectors_processed = num_threads * sectors_per_thread;
    unsigned long long total_bytes = total_sectors_processed * SECTOR_SIZE;
    
    printf("\n" COLOR_GREEN "CONCURRENT TEST COMPLETE\n" COLOR_RESET);
    printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf("Threads:          %d\n", num_threads);
    printf("Sectors/thread:   %lu\n", sectors_per_thread);
    printf("Total sectors:    %lu\n", total_sectors_processed);
    printf("Total data:       %.2f MB\n", total_bytes / (1024.0 * 1024.0));
    printf("Total time:       %.2f seconds\n", total_time / 1000.0);
    printf("Throughput:       %.2f MB/s\n", 
           (total_bytes / (1024.0 * 1024.0)) / (total_time / 1000.0));
    printf("IOPS:             %.1f operations/sec\n", 
           (num_threads * sectors_per_thread * (operation == 3 ? 2 : 1)) / (total_time / 1000.0));
    printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
}

void disk_scan(int fd) {
    unsigned long start_sector, num_sectors;
    int scan_type;
    
    printf(COLOR_BLUE "\n[DISK SCAN]\n" COLOR_RESET);
    printf("Start sector: ");
    scanf("%lu", &start_sector);
    while (getchar() != '\n');
    
    printf("Number of sectors to scan: ");
    scanf("%lu", &num_sectors);
    while (getchar() != '\n');
    
    printf("Scan type:\n");
    printf("1. Bad sector detection\n");
    printf("2. Data integrity check\n");
    printf("3. Pattern consistency\n");
    printf("Choice: ");
    scanf("%d", &scan_type);
    while (getchar() != '\n');
    
    char buffer[SECTOR_SIZE];
    unsigned long bad_sectors = 0;
    unsigned long total_read_errors = 0;
    
    printf("\n" COLOR_CYAN "Scanning sectors %lu to %lu...\n" COLOR_RESET,
           start_sector, start_sector + num_sectors - 1);
    
    double start_time = get_time_ms();
    
    for (unsigned long i = 0; i < num_sectors; i++) {
        unsigned long sector = start_sector + i;
        off_t offset = sector * SECTOR_SIZE;
        
        lseek(fd, offset, SEEK_SET);
        ssize_t bytes_read = read(fd, buffer, SECTOR_SIZE);
        
        if (bytes_read != SECTOR_SIZE) {
            bad_sectors++;
            total_read_errors += SECTOR_SIZE - bytes_read;
            
            if (bad_sectors <= 10) {  // Show first 10 bad sectors
                printf(COLOR_RED "Bad sector at %lu: read %ld/%d bytes\n" COLOR_RESET,
                       sector, bytes_read, SECTOR_SIZE);
            }
        }
        
        // Progress display
        if ((i + 1) % 100 == 0 || i == num_sectors - 1) {
            double progress = 100.0 * (i + 1) / num_sectors;
            printf("\rProgress: %6.2f%% | Sectors scanned: %lu | Bad: %lu",
                   progress, i + 1, bad_sectors);
            fflush(stdout);
        }
    }
    
    double end_time = get_time_ms();
    double total_time = end_time - start_time;
    
    printf("\n\n" COLOR_CYAN "SCAN RESULTS:\n" COLOR_RESET);
    printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf("Sectors scanned:    %lu\n", num_sectors);
    printf("Bad sectors found:  %lu\n", bad_sectors);
    printf("Total read errors:  %lu bytes\n", total_read_errors);
    printf("Error rate:         %.6f%%\n", 100.0 * bad_sectors / num_sectors);
    printf("Scan time:          %.2f seconds\n", total_time / 1000.0);
    printf("Scan speed:         %.1f sectors/sec\n", num_sectors / (total_time / 1000.0));
    
    if (bad_sectors == 0) {
        printf(COLOR_GREEN "\n✓ No bad sectors detected\n" COLOR_RESET);
    } else {
        printf(COLOR_YELLOW "\n⚠ %lu bad sectors detected\n" COLOR_RESET, bad_sectors);
    }
    
    printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
}

void show_menu() {
    printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf(COLOR_BOLD "MAIN MENU:\n" COLOR_RESET);
    printf(" 1. " COLOR_CYAN "Device information" COLOR_RESET "     7. " COLOR_CYAN "Concurrent test" COLOR_RESET "\n");
    printf(" 2. " COLOR_CYAN "Read sectors" COLOR_RESET "          8. " COLOR_CYAN "Disk scan" COLOR_RESET "\n");
    printf(" 3. " COLOR_CYAN "Write sectors" COLOR_RESET "         9. " COLOR_CYAN "Sector editor" COLOR_RESET "\n");
    printf(" 4. " COLOR_CYAN "Verify sectors" COLOR_RESET "       10. " COLOR_CYAN "Backup/restore" COLOR_RESET "\n");
    printf(" 5. " COLOR_CYAN "Fill with pattern" COLOR_RESET "    11. " COLOR_CYAN "Stress test" COLOR_RESET "\n");
    printf(" 6. " COLOR_CYAN "Benchmark" COLOR_RESET "             0. " COLOR_RED "Exit" COLOR_RESET "\n");
    printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf("Enter choice: ");
}

// Stubs for unimplemented functions
void stress_test(int fd) {
    printf(COLOR_BLUE "\n[STRESS TEST]\n" COLOR_RESET);
    printf("This feature is not implemented yet.\n");
    printf("Use concurrent test with many threads instead.\n");
}

void sector_editor(int fd) {
    printf(COLOR_BLUE "\n[SECTOR EDITOR]\n" COLOR_RESET);
    printf("This feature is not implemented yet.\n");
    printf("Use write sectors with hex editor pattern instead.\n");
}

void backup_restore(int fd) {
    printf(COLOR_BLUE "\n[BACKUP/RESTORE]\n" COLOR_RESET);
    printf("This feature is not implemented yet.\n");
    printf("Use read/write sectors with file option instead.\n");
}

int main(int argc, char *argv[]) {
    int fd;
    int choice;
    
    // Check for command line arguments
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Advanced Block Device Application\n");
            printf("Usage: %s [option]\n", argv[0]);
            printf("Options:\n");
            printf("  --help, -h     Show this help\n");
            printf("  --info         Show device information\n");
            printf("  --bench        Run benchmark\n");
            printf("  --scan         Run disk scan\n");
            printf("  --interactive  Start interactive mode (default)\n");
            return 0;
        }
    }
    
    // Open device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, COLOR_RED "Failed to open device: %s\n" COLOR_RESET, strerror(errno));
        fprintf(stderr, "Make sure the driver is loaded:\n");
        fprintf(stderr, "  sudo insmod simple_block.ko\n");
        fprintf(stderr, "  sudo mknod /dev/simple_block b 241 0\n");
        fprintf(stderr, "  sudo chmod 666 /dev/simple_block\n");
        return EXIT_FAILURE;
    }
    
    // Handle command line mode
    if (argc > 1) {
        if (strcmp(argv[1], "--info") == 0) {
            get_device_info(fd);
        } else if (strcmp(argv[1], "--bench") == 0) {
            benchmark(fd);
        } else if (strcmp(argv[1], "--scan") == 0) {
            disk_scan(fd);
        }
        close(fd);
        return EXIT_SUCCESS;
    }
    
    // Interactive mode
    while (1) {
        print_banner();
        print_status(fd);
        show_menu();
        
        char input[10];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        choice = atoi(input);
        
        switch (choice) {
            case 1:
                get_device_info(fd);
                break;
            case 2:
                read_sectors(fd);
                break;
            case 3:
                write_sectors(fd);
                break;
            case 4:
                verify_sectors(fd);
                break;
            case 5:
                fill_pattern(fd);
                break;
            case 6:
                benchmark(fd);
                break;
            case 7:
                concurrent_test(fd);
                break;
            case 8:
                disk_scan(fd);
                break;
            case 9:
                sector_editor(fd);
                break;
            case 10:
                backup_restore(fd);
                break;
            case 11:
                stress_test(fd);
                break;
            case 0:
                close(fd);
                printf(COLOR_GREEN "\nGoodbye!\n" COLOR_RESET);
                return EXIT_SUCCESS;
            default:
                printf(COLOR_RED "\nInvalid choice. Please try again.\n" COLOR_RESET);
                break;
        }
        
        printf(COLOR_YELLOW "\nPress Enter to continue..." COLOR_RESET);
        while (getchar() != '\n');
    }
    
    close(fd);
    return EXIT_SUCCESS;
}
