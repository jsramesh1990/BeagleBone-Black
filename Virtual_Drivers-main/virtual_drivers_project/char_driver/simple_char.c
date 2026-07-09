#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>

#define DEVICE_NAME "simple_char"
#define CLASS_NAME "simple_char_class"
#define BUFFER_SIZE 4096

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple Character Device Driver with Enhanced Features");
MODULE_LICENSE("GPL");
MODULE_VERSION("2.0");

static int major_number;
static struct class* char_class = NULL;
static struct device* char_device = NULL;
static struct cdev char_cdev;
static DEFINE_MUTEX(device_mutex);

static char *device_buffer = NULL;
static int buffer_size = BUFFER_SIZE;
static int buffer_offset = 0;
static int read_count = 0;
static int write_count = 0;

// IOCTL definitions
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

// Function prototypes
static int dev_open(struct inode*, struct file*);
static int dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);
static long dev_ioctl(struct file*, unsigned int, unsigned long);
static loff_t dev_llseek(struct file*, loff_t, int);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl,
    .llseek = dev_llseek,
};

static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "SimpleChar: Device opened by process %d\n", current->pid);
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "SimpleChar: Device closed\n");
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int bytes_to_read;
    int bytes_read;
    
    mutex_lock(&device_mutex);
    
    if (*offset >= buffer_offset) {
        mutex_unlock(&device_mutex);
        return 0;
    }
    
    bytes_to_read = min(len, (size_t)(buffer_offset - *offset));
    
    if (copy_to_user(buffer, device_buffer + *offset, bytes_to_read)) {
        mutex_unlock(&device_mutex);
        return -EFAULT;
    }
    
    *offset += bytes_to_read;
    read_count++;
    
    mutex_unlock(&device_mutex);
    
    printk(KERN_DEBUG "SimpleChar: Read %d bytes at offset %lld\n", bytes_to_read, *offset);
    return bytes_to_read;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    int bytes_to_write;
    
    mutex_lock(&device_mutex);
    
    // If writing beyond buffer, resize buffer
    if (*offset + len > buffer_size) {
        int new_size = max(buffer_size * 2, *offset + len);
        char *new_buffer = krealloc(device_buffer, new_size, GFP_KERNEL);
        if (!new_buffer) {
            mutex_unlock(&device_mutex);
            return -ENOMEM;
        }
        device_buffer = new_buffer;
        buffer_size = new_size;
        printk(KERN_INFO "SimpleChar: Buffer resized to %d bytes\n", buffer_size);
    }
    
    bytes_to_write = min(len, (size_t)(buffer_size - *offset));
    
    if (copy_from_user(device_buffer + *offset, buffer, bytes_to_write)) {
        mutex_unlock(&device_mutex);
        return -EFAULT;
    }
    
    if (*offset + bytes_to_write > buffer_offset) {
        buffer_offset = *offset + bytes_to_write;
    }
    
    *offset += bytes_to_write;
    write_count++;
    
    mutex_unlock(&device_mutex);
    
    printk(KERN_DEBUG "SimpleChar: Wrote %d bytes at offset %lld\n", bytes_to_write, *offset);
    return bytes_to_write;
}

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    struct char_stats stats;
    
    switch (cmd) {
        case CHAR_GET_SIZE:
            if (copy_to_user((int *)arg, &buffer_size, sizeof(int)))
                return -EFAULT;
            break;
            
        case CHAR_RESET_BUFFER:
            mutex_lock(&device_mutex);
            memset(device_buffer, 0, buffer_size);
            buffer_offset = 0;
            mutex_unlock(&device_mutex);
            printk(KERN_INFO "SimpleChar: Buffer reset\n");
            break;
            
        case CHAR_GET_STATS:
            mutex_lock(&device_mutex);
            stats.read_count = read_count;
            stats.write_count = write_count;
            stats.buffer_used = buffer_offset;
            stats.buffer_size = buffer_size;
            mutex_unlock(&device_mutex);
            
            if (copy_to_user((struct char_stats *)arg, &stats, sizeof(stats)))
                return -EFAULT;
            break;
            
        case CHAR_SET_BUFFER_SIZE:
            {
                int new_size;
                if (copy_from_user(&new_size, (int *)arg, sizeof(int)))
                    return -EFAULT;
                
                if (new_size < 1 || new_size > 65536) {
                    return -EINVAL;
                }
                
                mutex_lock(&device_mutex);
                char *new_buffer = krealloc(device_buffer, new_size, GFP_KERNEL);
                if (!new_buffer) {
                    mutex_unlock(&device_mutex);
                    return -ENOMEM;
                }
                device_buffer = new_buffer;
                buffer_size = new_size;
                if (buffer_offset > buffer_size) {
                    buffer_offset = buffer_size;
                }
                mutex_unlock(&device_mutex);
                printk(KERN_INFO "SimpleChar: Buffer size set to %d\n", new_size);
            }
            break;
            
        default:
            return -ENOTTY;
    }
    
    return 0;
}

static loff_t dev_llseek(struct file *filep, loff_t offset, int whence) {
    loff_t newpos;
    
    mutex_lock(&device_mutex);
    
    switch (whence) {
        case SEEK_SET:
            newpos = offset;
            break;
        case SEEK_CUR:
            newpos = filep->f_pos + offset;
            break;
        case SEEK_END:
            newpos = buffer_offset + offset;
            break;
        default:
            mutex_unlock(&device_mutex);
            return -EINVAL;
    }
    
    if (newpos < 0) {
        mutex_unlock(&device_mutex);
        return -EINVAL;
    }
    
    filep->f_pos = newpos;
    mutex_unlock(&device_mutex);
    
    return newpos;
}

static int __init char_init(void) {
    dev_t dev_num;
    
    printk(KERN_INFO "SimpleChar: Initializing enhanced driver\n");
    
    // Allocate buffer
    device_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!device_buffer) {
        printk(KERN_ALERT "SimpleChar: Failed to allocate buffer\n");
        return -ENOMEM;
    }
    memset(device_buffer, 0, BUFFER_SIZE);
    
    // Allocate major number
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        kfree(device_buffer);
        printk(KERN_ALERT "SimpleChar: Failed to allocate major number\n");
        return -1;
    }
    
    major_number = MAJOR(dev_num);
    printk(KERN_INFO "SimpleChar: Registered with major number %d\n", major_number);
    
    // Initialize cdev
    cdev_init(&char_cdev, &fops);
    char_cdev.owner = THIS_MODULE;
    
    if (cdev_add(&char_cdev, dev_num, 1) < 0) {
        kfree(device_buffer);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "SimpleChar: Failed to add cdev\n");
        return -1;
    }
    
    // Create class
    char_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(char_class)) {
        kfree(device_buffer);
        cdev_del(&char_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "SimpleChar: Failed to create class\n");
        return PTR_ERR(char_class);
    }
    
    // Create device
    char_device = device_create(char_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(char_device)) {
        kfree(device_buffer);
        class_destroy(char_class);
        cdev_del(&char_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "SimpleChar: Failed to create device\n");
        return PTR_ERR(char_device);
    }
    
    mutex_init(&device_mutex);
    
    printk(KERN_INFO "SimpleChar: Driver initialized successfully\n");
    printk(KERN_INFO "SimpleChar: Device buffer size: %d bytes\n", buffer_size);
    
    return 0;
}

static void __exit char_exit(void) {
    dev_t dev_num = MKDEV(major_number, 0);
    
    if (char_device)
        device_destroy(char_class, dev_num);
    if (char_class)
        class_destroy(char_class);
    cdev_del(&char_cdev);
    unregister_chrdev_region(dev_num, 1);
    
    if (device_buffer)
        kfree(device_buffer);
    
    mutex_destroy(&device_mutex);
    
    printk(KERN_INFO "SimpleChar: Driver removed\n");
}

module_init(char_init);
module_exit(char_exit);
