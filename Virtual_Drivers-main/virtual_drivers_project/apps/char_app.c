#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <pthread.h>
#include <ctype.h>
#include <termios.h>
#include <errno.h>

#define DEVICE_PATH "/dev/simple_char"
#define MAX_BUFFER_SIZE 65536
#define MAX_THREADS 10
#define IOCTL_MAGIC 'C'

// IOCTL definitions (must match driver)
#define CHAR_GET_SIZE _IOR(IOCTL_MAGIC, 1, int)
#define CHAR_RESET_BUFFER _IO(IOCTL_MAGIC, 2)
#define CHAR_GET_STATS _IOR(IOCTL_MAGIC, 3, struct char_stats)
#define CHAR_SET_BUFFER_SIZE _IOW(IOCTL_MAGIC, 4, int)

struct char_stats {
    int read_count;
    int write_count;
    int buffer_used;
    int buffer_size;
};

typedef struct {
    int fd;
    char *data;
    size_t size;
    int thread_id;
    int iterations;
    int operation;  // 0=read, 1=write, 2=both
} thread_args;

// Global variables
static struct termios old_termios;
static int color_enabled = 1;

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

// Function prototypes
void enable_raw_mode();
void disable_raw_mode();
void clear_screen();
void print_banner();
void print_status(int fd);
void show_menu();
void write_data(int fd);
void read_data(int fd);
void hex_view(int fd);
void benchmark(int fd);
void concurrent_test(int fd);
void stress_test(int fd);
void display_stats(int fd);
void pattern_test(int fd);
void search_pattern(int fd);
void compare_with_file(int fd);
void buffer_operations(int fd);
void* thread_function(void* arg);
double get_time_ms();

// Utility functions
void enable_raw_mode() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}

void clear_screen() {
    printf("\033[2J\033[1;1H");
}

void print_banner() {
    clear_screen();
    printf(COLOR_CYAN "╔══════════════════════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_BOLD COLOR_YELLOW "           ADVANCED CHARACTER DEVICE MANAGER          " COLOR_RESET COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_WHITE "                 Virtual Character Driver v2.0            " COLOR_RESET COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "╚══════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    printf("\n");
}

void print_status(int fd) {
    struct char_stats stats;
    if (ioctl(fd, CHAR_GET_STATS, &stats) >= 0) {
        printf(COLOR_GREEN "Device: " COLOR_WHITE "%s" COLOR_RESET "\n", DEVICE_PATH);
        printf(COLOR_GREEN "Buffer: " COLOR_WHITE "%d/%d bytes used (%.1f%%)" COLOR_RESET "\n", 
               stats.buffer_used, stats.buffer_size,
               (stats.buffer_size > 0) ? (100.0 * stats.buffer_used / stats.buffer_size) : 0.0);
        printf(COLOR_GREEN "Operations: " COLOR_WHITE "R: %d, W: %d, Total: %d" COLOR_RESET "\n",
               stats.read_count, stats.write_count, stats.read_count + stats.write_count);
    }
    printf("\n");
}

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

void* thread_function(void* arg) {
    thread_args* args = (thread_args*)arg;
    char buffer[1024];
    char thread_id_str[16];
    ssize_t result;
    
    sprintf(thread_id_str, "[Thread %d]", args->thread_id);
    
    for (int i = 0; i < args->iterations; i++) {
        if (args->operation == 0 || args->operation == 2) {  // Read
            lseek(args->fd, 0, SEEK_SET);
            result = read(args->fd, buffer, sizeof(buffer));
            printf("%s Read %ld bytes\n", thread_id_str, result);
        }
        
        if (args->operation == 1 || args->operation == 2) {  // Write
            sprintf(buffer, "Thread %d iteration %d", args->thread_id, i);
            result = write(args->fd, buffer, strlen(buffer));
            printf("%s Wrote %ld bytes\n", thread_id_str, result);
        }
        
        usleep(1000);  // Small delay
    }
    
    return NULL;
}

void write_data(int fd) {
    char buffer[MAX_BUFFER_SIZE];
    char filename[256];
    int choice;
    ssize_t written;
    
    printf(COLOR_BLUE "\n[WRITE OPERATION]\n" COLOR_RESET);
    printf("1. Enter text manually\n");
    printf("2. Load from file\n");
    printf("3. Generate pattern\n");
    printf("Choice: ");
    scanf("%d", &choice);
    while (getchar() != '\n');
    
    size_t data_size = 0;
    
    switch (choice) {
        case 1:
            printf("Enter text (end with empty line or Ctrl+D):\n");
            printf(COLOR_CYAN "> " COLOR_RESET);
            
            char line[256];
            while (fgets(line, sizeof(line), stdin) != NULL && strlen(line) > 1) {
                size_t line_len = strlen(line);
                if (line[line_len-1] == '\n') line[line_len-1] = '\0';
                memcpy(buffer + data_size, line, line_len);
                data_size += line_len;
                printf(COLOR_CYAN "> " COLOR_RESET);
            }
            break;
            
        case 2:
            printf("Enter filename: ");
            scanf("%255s", filename);
            while (getchar() != '\n');
            
            FILE* file = fopen(filename, "r");
            if (!file) {
                printf(COLOR_RED "Error opening file: %s\n" COLOR_RESET, strerror(errno));
                return;
            }
            
            data_size = fread(buffer, 1, MAX_BUFFER_SIZE - 1, file);
            fclose(file);
            
            if (data_size == 0) {
                printf(COLOR_RED "File is empty or error reading\n" COLOR_RESET);
                return;
            }
            printf("Loaded %ld bytes from file\n", data_size);
            break;
            
        case 3:
            printf("Enter pattern length (max %d): ", MAX_BUFFER_SIZE - 1);
            scanf("%ld", &data_size);
            while (getchar() != '\n');
            
            if (data_size > MAX_BUFFER_SIZE - 1) {
                data_size = MAX_BUFFER_SIZE - 1;
            }
            
            printf("Pattern type:\n");
            printf("1. Sequential (0,1,2,...)\n");
            printf("2. Repeated byte\n");
            printf("3. Random data\n");
            printf("Choice: ");
            int pattern_type;
            scanf("%d", &pattern_type);
            while (getchar() != '\n');
            
            if (pattern_type == 1) {
                for (size_t i = 0; i < data_size; i++) {
                    buffer[i] = i % 256;
                }
            } else if (pattern_type == 2) {
                printf("Enter byte value (0-255): ");
                int byte_val;
                scanf("%d", &byte_val);
                while (getchar() != '\n');
                memset(buffer, byte_val, data_size);
            } else {
                srand(time(NULL));
                for (size_t i = 0; i < data_size; i++) {
                    buffer[i] = rand() % 256;
                }
            }
            break;
            
        default:
            printf(COLOR_RED "Invalid choice\n" COLOR_RESET);
            return;
    }
    
    if (data_size == 0) {
        printf(COLOR_YELLOW "No data to write\n" COLOR_RESET);
        return;
    }
    
    printf("Write options:\n");
    printf("1. Overwrite from current position\n");
    printf("2. Append to end\n");
    printf("3. Write at specific position\n");
    printf("Choice: ");
    int write_option;
    scanf("%d", &write_option);
    while (getchar() != '\n');
    
    off_t original_pos = lseek(fd, 0, SEEK_CUR);
    
    switch (write_option) {
        case 1:
            // Write at current position
            break;
        case 2:
            lseek(fd, 0, SEEK_END);
            break;
        case 3:
            printf("Enter position (bytes from start): ");
            off_t pos;
            scanf("%ld", &pos);
            while (getchar() != '\n');
            lseek(fd, pos, SEEK_SET);
            break;
    }
    
    double start_time = get_time_ms();
    written = write(fd, buffer, data_size);
    double end_time = get_time_ms();
    
    if (written < 0) {
        printf(COLOR_RED "Write failed: %s\n" COLOR_RESET, strerror(errno));
    } else {
        printf(COLOR_GREEN "Successfully wrote %ld bytes in %.2f ms (%.2f KB/s)\n" COLOR_RESET,
               written, end_time - start_time, 
               (written / 1024.0) / ((end_time - start_time) / 1000.0));
    }
    
    lseek(fd, original_pos, SEEK_SET);
}

void read_data(int fd) {
    char buffer[MAX_BUFFER_SIZE];
    size_t bytes_to_read;
    off_t position;
    int choice;
    
    printf(COLOR_BLUE "\n[READ OPERATION]\n" COLOR_RESET);
    printf("1. Read from current position\n");
    printf("2. Read from beginning\n");
    printf("3. Read from specific position\n");
    printf("Choice: ");
    scanf("%d", &choice);
    while (getchar() != '\n');
    
    off_t original_pos = lseek(fd, 0, SEEK_CUR);
    
    switch (choice) {
        case 1:
            // Read from current position
            position = original_pos;
            break;
        case 2:
            lseek(fd, 0, SEEK_SET);
            position = 0;
            break;
        case 3:
            printf("Enter position (bytes from start): ");
            scanf("%ld", &position);
            while (getchar() != '\n');
            lseek(fd, position, SEEK_SET);
            break;
        default:
            printf(COLOR_RED "Invalid choice\n" COLOR_RESET);
            return;
    }
    
    printf("Enter bytes to read (max %d): ", MAX_BUFFER_SIZE - 1);
    scanf("%ld", &bytes_to_read);
    while (getchar() != '\n');
    
    if (bytes_to_read <= 0 || bytes_to_read > MAX_BUFFER_SIZE - 1) {
        printf(COLOR_RED "Invalid size\n" COLOR_RESET);
        lseek(fd, original_pos, SEEK_SET);
        return;
    }
    
    double start_time = get_time_ms();
    ssize_t bytes_read = read(fd, buffer, bytes_to_read);
    double end_time = get_time_ms();
    
    lseek(fd, original_pos, SEEK_SET);
    
    if (bytes_read < 0) {
        printf(COLOR_RED "Read failed: %s\n" COLOR_RESET, strerror(errno));
    } else if (bytes_read == 0) {
        printf(COLOR_YELLOW "No data available at this position\n" COLOR_RESET);
    } else {
        buffer[bytes_read] = '\0';
        
        printf(COLOR_GREEN "\nRead %ld bytes in %.2f ms (%.2f KB/s)\n" COLOR_RESET,
               bytes_read, end_time - start_time,
               (bytes_read / 1024.0) / ((end_time - start_time) / 1000.0));
        
        // Check if data is printable
        int is_text = 1;
        for (int i = 0; i < bytes_read && i < 1024; i++) {
            if (!isprint(buffer[i]) && !isspace(buffer[i]) && buffer[i] != '\0') {
                is_text = 0;
                break;
            }
        }
        
        printf("\n" COLOR_CYAN "DATA PREVIEW:" COLOR_RESET "\n");
        printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
        
        if (is_text) {
            // Display as text
            int preview_size = bytes_read > 1024 ? 1024 : bytes_read;
            for (int i = 0; i < preview_size; i++) {
                putchar(buffer[i]);
                if ((i + 1) % 80 == 0) printf("\n");
            }
            if (bytes_read > 1024) {
                printf("\n[... %ld more bytes ...]\n", bytes_read - 1024);
            }
        } else {
            // Display hex dump
            int lines = (bytes_read + 15) / 16;
            lines = lines > 32 ? 32 : lines;  // Limit to 32 lines
            
            for (int line = 0; line < lines; line++) {
                printf(COLOR_YELLOW "%08lx: " COLOR_RESET, position + line * 16);
                
                // Hex bytes
                for (int i = 0; i < 16; i++) {
                    int idx = line * 16 + i;
                    if (idx < bytes_read) {
                        printf("%02x ", (unsigned char)buffer[idx]);
                    } else {
                        printf("   ");
                    }
                    if (i == 7) printf(" ");
                }
                
                printf(" ");
                
                // ASCII representation
                for (int i = 0; i < 16; i++) {
                    int idx = line * 16 + i;
                    if (idx < bytes_read) {
                        unsigned char c = buffer[idx];
                        printf("%c", isprint(c) ? c : '.');
                    } else {
                        printf(" ");
                    }
                }
                
                printf("\n");
            }
            
            if (bytes_read > lines * 16) {
                printf(COLOR_YELLOW "... %ld more bytes ...\n" COLOR_RESET, bytes_read - lines * 16);
            }
        }
        
        printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    }
}

void hex_view(int fd) {
    char buffer[256];
    off_t position = 0;
    int lines = 16;
    
    printf(COLOR_BLUE "\n[HEX VIEWER]\n" COLOR_RESET);
    printf("Enter starting position (in bytes): ");
    scanf("%ld", &position);
    while (getchar() != '\n');
    
    printf("Enter number of lines (16 bytes each): ");
    scanf("%d", &lines);
    while (getchar() != '\n');
    
    if (lines <= 0 || lines > 100) {
        printf(COLOR_RED "Invalid number of lines (1-100)\n" COLOR_RESET);
        return;
    }
    
    off_t original_pos = lseek(fd, 0, SEEK_CUR);
    lseek(fd, position, SEEK_SET);
    
    printf("\n" COLOR_CYAN "Hex Dump from position 0x%08lx\n" COLOR_RESET, position);
    printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    for (int line = 0; line < lines; line++) {
        ssize_t bytes_read = read(fd, buffer, 16);
        
        if (bytes_read <= 0) {
            if (bytes_read < 0) {
                printf(COLOR_RED "Read error at line %d\n" COLOR_RESET, line);
            }
            break;
        }
        
        // Print address
        printf(COLOR_YELLOW "%08lx: " COLOR_RESET, position + line * 16);
        
        // Print hex bytes
        for (int i = 0; i < 16; i++) {
            if (i < bytes_read) {
                printf("%02x ", (unsigned char)buffer[i]);
            } else {
                printf("   ");
            }
            if (i == 7) printf(" ");
        }
        
        printf(" ");
        
        // Print ASCII representation
        for (int i = 0; i < bytes_read; i++) {
            unsigned char c = buffer[i];
            if (isprint(c)) {
                printf("%c", c);
            } else {
                printf(".");
            }
        }
        
        printf("\n");
    }
    
    printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    lseek(fd, original_pos, SEEK_SET);
}

void benchmark(int fd) {
    int test_size;
    int iterations;
    
    printf(COLOR_BLUE "\n[BENCHMARK TOOL]\n" COLOR_RESET);
    printf("Enter test data size (KB, 1-1024): ");
    scanf("%d", &test_size);
    while (getchar() != '\n');
    
    if (test_size < 1 || test_size > 1024) {
        printf(COLOR_RED "Invalid size\n" COLOR_RESET);
        return;
    }
    
    printf("Enter number of iterations: ");
    scanf("%d", &iterations);
    while (getchar() != '\n');
    
    if (iterations < 1) {
        printf(COLOR_RED "Invalid iterations\n" COLOR_RESET);
        return;
    }
    
    size_t data_size = test_size * 1024;
    char* test_data = malloc(data_size);
    if (!test_data) {
        printf(COLOR_RED "Memory allocation failed\n" COLOR_RESET);
        return;
    }
    
    // Fill with pattern
    for (size_t i = 0; i < data_size; i++) {
        test_data[i] = i % 256;
    }
    
    printf("\n" COLOR_CYAN "Running benchmark with %d KB, %d iterations...\n" COLOR_RESET, 
           test_size, iterations);
    printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    // Reset buffer first
    ioctl(fd, CHAR_RESET_BUFFER);
    
    // Write benchmark
    double write_start = get_time_ms();
    for (int i = 0; i < iterations; i++) {
        lseek(fd, 0, SEEK_SET);
        ssize_t written = write(fd, test_data, data_size);
        if (written != data_size) {
            printf(COLOR_RED "Write error at iteration %d\n" COLOR_RESET, i);
            break;
        }
    }
    double write_end = get_time_ms();
    double write_time = write_end - write_start;
    
    // Read benchmark
    double read_start = get_time_ms();
    for (int i = 0; i < iterations; i++) {
        lseek(fd, 0, SEEK_SET);
        ssize_t read_bytes = read(fd, test_data, data_size);
        if (read_bytes != data_size) {
            printf(COLOR_RED "Read error at iteration %d\n" COLOR_RESET, i);
            break;
        }
    }
    double read_end = get_time_ms();
    double read_time = read_end - read_start;
    
    free(test_data);
    
    // Calculate statistics
    double total_data_mb = (iterations * data_size) / (1024.0 * 1024.0);
    double write_speed = total_data_mb / (write_time / 1000.0);
    double read_speed = total_data_mb / (read_time / 1000.0);
    
    printf("\n" COLOR_GREEN "BENCHMARK RESULTS:\n" COLOR_RESET);
    printf(COLOR_CYAN "──────────────────────────────────────────────────────\n" COLOR_RESET);
    printf("Test size:        %d KB per iteration\n", test_size);
    printf("Iterations:       %d\n", iterations);
    printf("Total data:       %.2f MB\n", total_data_mb);
    printf("\n");
    printf("Write time:       %.2f ms\n", write_time);
    printf("Write speed:      %.2f MB/s\n", write_speed);
    printf("Write latency:    %.3f ms/op\n", write_time / iterations);
    printf("\n");
    printf("Read time:        %.2f ms\n", read_time);
    printf("Read speed:       %.2f MB/s\n", read_speed);
    printf("Read latency:     %.3f ms/op\n", read_time / iterations);
    printf(COLOR_CYAN "──────────────────────────────────────────────────────\n" COLOR_RESET);
}

void concurrent_test(int fd) {
    int num_threads;
    int iterations;
    int operation;
    
    printf(COLOR_BLUE "\n[CONCURRENT ACCESS TEST]\n" COLOR_RESET);
    printf("Number of threads (1-%d): ", MAX_THREADS);
    scanf("%d", &num_threads);
    while (getchar() != '\n');
    
    if (num_threads < 1 || num_threads > MAX_THREADS) {
        printf(COLOR_RED "Invalid number of threads\n" COLOR_RESET);
        return;
    }
    
    printf("Iterations per thread: ");
    scanf("%d", &iterations);
    while (getchar() != '\n');
    
    printf("Operation:\n");
    printf("1. Read only\n");
    printf("2. Write only\n");
    printf("3. Read and write\n");
    printf("Choice: ");
    scanf("%d", &operation);
    while (getchar() != '\n');
    
    if (operation < 1 || operation > 3) {
        printf(COLOR_RED "Invalid operation\n" COLOR_RESET);
        return;
    }
    
    // Reset buffer first
    ioctl(fd, CHAR_RESET_BUFFER);
    
    printf("\n" COLOR_CYAN "Starting %d threads with %d iterations each...\n" COLOR_RESET,
           num_threads, iterations);
    
    pthread_t threads[MAX_THREADS];
    thread_args args[MAX_THREADS];
    
    double start_time = get_time_ms();
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        args[i].fd = fd;
        args[i].thread_id = i + 1;
        args[i].iterations = iterations;
        args[i].operation = operation - 1;  // Convert to 0-based
        
        if (pthread_create(&threads[i], NULL, thread_function, &args[i]) != 0) {
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
    
    printf("\n" COLOR_GREEN "CONCURRENT TEST COMPLETE\n" COLOR_RESET);
    printf(COLOR_CYAN "──────────────────────────────────────────────────────\n" COLOR_RESET);
    printf("Threads:          %d\n", num_threads);
    printf("Iterations:       %d each\n", iterations);
    printf("Total operations: %d\n", num_threads * iterations * 
           (operation == 3 ? 2 : 1));
    printf("Total time:       %.2f ms\n", total_time);
    printf("Throughput:       %.1f ops/sec\n", 
           (num_threads * iterations * (operation == 3 ? 2 : 1)) / (total_time / 1000.0));
    printf(COLOR_CYAN "──────────────────────────────────────────────────────\n" COLOR_RESET);
}

void stress_test(int fd) {
    int duration;
    int max_threads;
    
    printf(COLOR_BLUE "\n[STRESS TEST]\n" COLOR_RESET);
    printf("WARNING: This test may cause high CPU and memory usage!\n");
    printf("Test duration (seconds, 1-60): ");
    scanf("%d", &duration);
    while (getchar() != '\n');
    
    printf("Maximum concurrent threads (1-20): ");
    scanf("%d", &max_threads);
    while (getchar() != '\n');
    
    if (duration < 1 || duration > 60) {
        printf(COLOR_RED "Invalid duration\n" COLOR_RESET);
        return;
    }
    
    if (max_threads < 1 || max_threads > 20) {
        printf(COLOR_RED "Invalid thread count\n" COLOR_RESET);
        return;
    }
    
    printf("\n" COLOR_CYAN "Starting stress test for %d seconds with up to %d threads...\n" COLOR_RESET,
           duration, max_threads);
    printf(COLOR_YELLOW "Press Ctrl+C to abort\n" COLOR_RESET);
    
    // Reset buffer first
    ioctl(fd, CHAR_RESET_BUFFER);
    
    time_t start_time = time(NULL);
    time_t end_time = start_time + duration;
    int total_operations = 0;
    
    while (time(NULL) < end_time) {
        // Random number of threads for this batch
        int threads_in_batch = rand() % max_threads + 1;
        pthread_t threads[threads_in_batch];
        
        // Create threads
        for (int i = 0; i < threads_in_batch; i++) {
            thread_args* args = malloc(sizeof(thread_args));
            args->fd = fd;
            args->thread_id = i;
            args->iterations = rand() % 10 + 1;
            args->operation = rand() % 3;  // 0=read, 1=write, 2=both
            
            pthread_create(&threads[i], NULL, thread_function, args);
            total_operations += args->iterations * (args->operation == 2 ? 2 : 1);
        }
        
        // Wait for threads
        for (int i = 0; i < threads_in_batch; i++) {
            pthread_join(threads[i], NULL);
        }
        
        // Small delay
        usleep(10000);  // 10ms
    }
    
    double actual_duration = difftime(time(NULL), start_time);
    
    struct char_stats stats;
    ioctl(fd, CHAR_GET_STATS, &stats);
    
    printf("\n" COLOR_GREEN "STRESS TEST COMPLETE\n" COLOR_RESET);
    printf(COLOR_CYAN "──────────────────────────────────────────────────────\n" COLOR_RESET);
    printf("Duration:         %.1f seconds\n", actual_duration);
    printf("Total operations: %d\n", total_operations);
    printf("Operations/sec:   %.1f\n", total_operations / actual_duration);
    printf("\n");
    printf("Final statistics:\n");
    printf("  Reads:          %d\n", stats.read_count);
    printf("  Writes:         %d\n", stats.write_count);
    printf("  Buffer used:    %d/%d bytes\n", stats.buffer_used, stats.buffer_size);
    printf(COLOR_CYAN "──────────────────────────────────────────────────────\n" COLOR_RESET);
}

void display_stats(int fd) {
    struct char_stats stats;
    
    if (ioctl(fd, CHAR_GET_STATS, &stats) < 0) {
        printf(COLOR_RED "Failed to get statistics\n" COLOR_RESET);
        return;
    }
    
    printf(COLOR_BLUE "\n[DEVICE STATISTICS]\n" COLOR_RESET);
    printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    printf(COLOR_GREEN "General Information:\n" COLOR_RESET);
    printf("  Device:          %s\n", DEVICE_PATH);
    printf("  Driver version:  2.0\n");
    
    printf("\n" COLOR_GREEN "Buffer Status:\n" COLOR_RESET);
    printf("  Total size:      %d bytes\n", stats.buffer_size);
    printf("  Used:            %d bytes\n", stats.buffer_used);
    printf("  Free:            %d bytes\n", stats.buffer_size - stats.buffer_used);
    printf("  Usage:           %.1f%%\n", 
           (stats.buffer_size > 0) ? (100.0 * stats.buffer_used / stats.buffer_size) : 0.0);
    
    printf("\n" COLOR_GREEN "Operation Counters:\n" COLOR_RESET);
    printf("  Read operations: %d\n", stats.read_count);
    printf("  Write operations:%d\n", stats.write_count);
    printf("  Total:           %d\n", stats.read_count + stats.write_count);
    
    printf("\n" COLOR_GREEN "Performance Indicators:\n" COLOR_RESET);
    if (stats.read_count + stats.write_count > 0) {
        printf("  Read ratio:      %.1f%%\n", 
               100.0 * stats.read_count / (stats.read_count + stats.write_count));
        printf("  Write ratio:     %.1f%%\n", 
               100.0 * stats.write_count / (stats.read_count + stats.write_count));
    }
    
    printf(COLOR_CYAN "══════════════════════════════════════════════════════════\n" COLOR_RESET);
}

void pattern_test(int fd) {
    int pattern_type;
    size_t size;
    
    printf(COLOR_BLUE "\n[PATTERN TEST]\n" COLOR_RESET);
    printf("Pattern types:\n");
    printf("1. Sequential bytes (00, 01, 02...)\n");
    printf("2. Checkerboard (AA, 55, AA...)\n");
    printf("3. Walking ones (01, 02, 04...)\n");
    printf("4. Random data\n");
    printf("5. String pattern\n");
    printf("Choice: ");
    scanf("%d", &pattern_type);
    while (getchar() != '\n');
    
    if (pattern_type < 1 || pattern_type > 5) {
        printf(COLOR_RED "Invalid pattern type\n" COLOR_RESET);
        return;
    }
    
    printf("Size in bytes (max %d): ", MAX_BUFFER_SIZE);
    scanf("%ld", &size);
    while (getchar() != '\n');
    
    if (size <= 0 || size > MAX_BUFFER_SIZE) {
        printf(COLOR_RED "Invalid size\n" COLOR_RESET);
        return;
    }
    
    char* pattern = malloc(size);
    if (!pattern) {
        printf(COLOR_RED "Memory allocation failed\n" COLOR_RESET);
        return;
    }
    
    // Generate pattern
    switch (pattern_type) {
        case 1:  // Sequential
            for (size_t i = 0; i < size; i++) {
                pattern[i] = i % 256;
            }
            break;
            
        case 2:  // Checkerboard
            for (size_t i = 0; i < size; i++) {
                pattern[i] = (i % 2 == 0) ? 0xAA : 0x55;
            }
            break;
            
        case 3:  // Walking ones
            for (size_t i = 0; i < size; i++) {
                pattern[i] = 1 << (i % 8);
            }
            break;
            
        case 4:  // Random
            srand(time(NULL));
            for (size_t i = 0; i < size; i++) {
                pattern[i] = rand() % 256;
            }
            break;
            
        case 5:  // String
            printf("Enter pattern string: ");
            char str[256];
            fgets(str, sizeof(str), stdin);
            str[strcspn(str, "\n")] = 0;
            
            for (size_t i = 0; i < size; i++) {
                pattern[i] = str[i % strlen(str)];
            }
            break;
    }
    
    // Write pattern
    lseek(fd, 0, SEEK_SET);
    ssize_t written = write(fd, pattern, size);
    
    if (written != size) {
        printf(COLOR_RED "Failed to write pattern\n" COLOR_RESET);
    } else {
        printf(COLOR_GREEN "Pattern written successfully\n" COLOR_RESET);
        
        // Verify pattern
        lseek(fd, 0, SEEK_SET);
        char* verify = malloc(size);
        ssize_t read_bytes = read(fd, verify, size);
        
        if (read_bytes != size) {
            printf(COLOR_RED "Failed to read back pattern\n" COLOR_RESET);
        } else if (memcmp(pattern, verify, size) != 0) {
            printf(COLOR_RED "Pattern verification failed!\n" COLOR_RESET);
            
            // Find first mismatch
            for (size_t i = 0; i < size; i++) {
                if (pattern[i] != verify[i]) {
                    printf("First mismatch at byte %ld: expected 0x%02x, got 0x%02x\n",
                           i, (unsigned char)pattern[i], (unsigned char)verify[i]);
                    break;
                }
            }
        } else {
            printf(COLOR_GREEN "Pattern verification successful!\n" COLOR_RESET);
        }
        
        free(verify);
    }
    
    free(pattern);
}

void search_pattern(int fd) {
    char search_str[256];
    char buffer[4096];
    off_t position = 0;
    int found = 0;
    
    printf(COLOR_BLUE "\n[PATTERN SEARCH]\n" COLOR_RESET);
    printf("Enter pattern to search (max 255 chars): ");
    fgets(search_str, sizeof(search_str), stdin);
    search_str[strcspn(search_str, "\n")] = 0;
    
    if (strlen(search_str) == 0) {
        printf(COLOR_RED "Empty pattern\n" COLOR_RESET);
        return;
    }
    
    printf("\n" COLOR_CYAN "Searching for '%s'...\n" COLOR_RESET, search_str);
    
    off_t original_pos = lseek(fd, 0, SEEK_CUR);
    lseek(fd, 0, SEEK_SET);
    
    while (1) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            break;
        }
        
        // Search in buffer
        for (int i = 0; i <= bytes_read - strlen(search_str); i++) {
            if (memcmp(buffer + i, search_str, strlen(search_str)) == 0) {
                printf(COLOR_GREEN "Found at position 0x%08lx (byte %ld)\n" COLOR_RESET,
                       position + i, position + i);
                found++;
            }
        }
        
        position += bytes_read;
        
        // Move back to allow overlapping patterns
        if (bytes_read == sizeof(buffer)) {
            lseek(fd, position - strlen(search_str) + 1, SEEK_SET);
            position -= strlen(search_str) - 1;
        }
    }
    
    lseek(fd, original_pos, SEEK_SET);
    
    if (found == 0) {
        printf(COLOR_YELLOW "Pattern not found\n" COLOR_RESET);
    } else {
        printf(COLOR_GREEN "\nTotal occurrences: %d\n" COLOR_RESET, found);
    }
}

void buffer_operations(int fd) {
    int choice;
    
    printf(COLOR_BLUE "\n[BUFFER OPERATIONS]\n" COLOR_RESET);
    printf("1. Reset buffer (clear all data)\n");
    printf("2. Get buffer size\n");
    printf("3. Set buffer size\n");
    printf("Choice: ");
    scanf("%d", &choice);
    while (getchar() != '\n');
    
    switch (choice) {
        case 1:
            if (ioctl(fd, CHAR_RESET_BUFFER) < 0) {
                printf(COLOR_RED "Failed to reset buffer\n" COLOR_RESET);
            } else {
                printf(COLOR_GREEN "Buffer reset successfully\n" COLOR_RESET);
            }
            break;
            
        case 2: {
            int size;
            if (ioctl(fd, CHAR_GET_SIZE, &size) < 0) {
                printf(COLOR_RED "Failed to get buffer size\n" COLOR_RESET);
            } else {
                printf(COLOR_GREEN "Current buffer size: %d bytes\n" COLOR_RESET, size);
            }
            break;
        }
            
        case 3: {
            int new_size;
            printf("Enter new buffer size (1-65536): ");
            scanf("%d", &new_size);
            while (getchar() != '\n');
            
            if (new_size < 1 || new_size > 65536) {
                printf(COLOR_RED "Invalid size\n" COLOR_RESET);
                break;
            }
            
            if (ioctl(fd, CHAR_SET_BUFFER_SIZE, &new_size) < 0) {
                printf(COLOR_RED "Failed to set buffer size\n" COLOR_RESET);
            } else {
                printf(COLOR_GREEN "Buffer size set to %d bytes\n" COLOR_RESET, new_size);
            }
            break;
        }
            
        default:
            printf(COLOR_RED "Invalid choice\n" COLOR_RESET);
    }
}

void show_menu() {
    printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf(COLOR_BOLD "MAIN MENU:\n" COLOR_RESET);
    printf(" 1. " COLOR_CYAN "Write data" COLOR_RESET "              6. " COLOR_CYAN "Stress test" COLOR_RESET "\n");
    printf(" 2. " COLOR_CYAN "Read data" COLOR_RESET "               7. " COLOR_CYAN "Device statistics" COLOR_RESET "\n");
    printf(" 3. " COLOR_CYAN "Hex viewer" COLOR_RESET "             8. " COLOR_CYAN "Pattern test" COLOR_RESET "\n");
    printf(" 4. " COLOR_CYAN "Benchmark" COLOR_RESET "              9. " COLOR_CYAN "Search pattern" COLOR_RESET "\n");
    printf(" 5. " COLOR_CYAN "Concurrent test" COLOR_RESET "       10. " COLOR_CYAN "Buffer operations" COLOR_RESET "\n");
    printf(" 0. " COLOR_RED "Exit" COLOR_RESET "\n");
    printf(COLOR_MAGENTA "══════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf("Enter choice: ");
}

int main(int argc, char *argv[]) {
    int fd;
    int choice;
    
    // Check for command line arguments
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Advanced Character Device Application\n");
            printf("Usage: %s [option]\n", argv[0]);
            printf("Options:\n");
            printf("  --help, -h     Show this help\n");
            printf("  --bench        Run benchmark test\n");
            printf("  --stress       Run stress test\n");
            printf("  --test         Run pattern test\n");
            printf("  --stats        Show device statistics\n");
            printf("  --interactive  Start interactive mode (default)\n");
            return 0;
        }
    }
    
    // Open device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, COLOR_RED "Failed to open device: %s\n" COLOR_RESET, strerror(errno));
        fprintf(stderr, "Make sure the driver is loaded:\n");
        fprintf(stderr, "  sudo insmod simple_char.ko\n");
        fprintf(stderr, "  sudo mknod /dev/simple_char c 240 0\n");
        fprintf(stderr, "  sudo chmod 666 /dev/simple_char\n");
        return EXIT_FAILURE;
    }
    
    // Handle command line mode
    if (argc > 1) {
        if (strcmp(argv[1], "--bench") == 0) {
            benchmark(fd);
        } else if (strcmp(argv[1], "--stress") == 0) {
            stress_test(fd);
        } else if (strcmp(argv[1], "--test") == 0) {
            pattern_test(fd);
        } else if (strcmp(argv[1], "--stats") == 0) {
            display_stats(fd);
        }
        close(fd);
        return EXIT_SUCCESS;
    }
    
    // Interactive mode
    enable_raw_mode();
    
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
                write_data(fd);
                break;
            case 2:
                read_data(fd);
                break;
            case 3:
                hex_view(fd);
                break;
            case 4:
                benchmark(fd);
                break;
            case 5:
                concurrent_test(fd);
                break;
            case 6:
                stress_test(fd);
                break;
            case 7:
                display_stats(fd);
                break;
            case 8:
                pattern_test(fd);
                break;
            case 9:
                search_pattern(fd);
                break;
            case 10:
                buffer_operations(fd);
                break;
            case 0:
                close(fd);
                disable_raw_mode();
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
    disable_raw_mode();
    return EXIT_SUCCESS;
}
