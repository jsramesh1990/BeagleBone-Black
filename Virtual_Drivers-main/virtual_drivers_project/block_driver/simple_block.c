#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/bio.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <linux/fs.h>

#define DEVICE_NAME "simple_block"
#define SECTOR_SIZE 512
#define DEFAULT_SECTORS 2048  // 1MB default size
#define MAX_SECTORS 65536     // 32MB max size

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Enhanced Block Device Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("2.0");

static struct block_device_operations block_ops;
static struct gendisk *simple_disk = NULL;
static struct request_queue *queue = NULL;
static u8 *device_data = NULL;
static int major_number = 0;
static DEFINE_MUTEX(device_mutex);

static unsigned long device_sectors = DEFAULT_SECTORS;
static unsigned long read_ops = 0;
static unsigned long write_ops = 0;

// Block device request processing
static void simple_block_request(struct request_queue *q) {
    struct request *req;
    struct bio_vec bvec;
    struct req_iterator iter;
    char *buffer;
    sector_t sector;
    unsigned int bytes;
    
    while ((req = blk_fetch_request(q)) != NULL) {
        if (req->cmd_type != REQ_TYPE_FS) {
            printk(KERN_ERR "SimpleBlock: Non-fs request\n");
            __blk_end_request_all(req, -EIO);
            continue;
        }
        
        sector = blk_rq_pos(req);
        bytes = blk_rq_bytes(req);
        
        mutex_lock(&device_mutex);
        
        if ((sector + bytes / SECTOR_SIZE) > device_sectors) {
            printk(KERN_ERR "SimpleBlock: Request beyond device limits\n");
            mutex_unlock(&device_mutex);
            __blk_end_request_all(req, -EIO);
            continue;
        }
        
        rq_for_each_segment(bvec, req, iter) {
            buffer = kmap(bvec.bv_page) + bvec.bv_offset;
            
            if (rq_data_dir(req) == READ) {
                // Read operation
                memcpy(buffer, device_data + (sector * SECTOR_SIZE) + bvec.bv_offset, bvec.bv_len);
                read_ops++;
                printk(KERN_DEBUG "SimpleBlock: Read %u bytes from sector %llu\n",
                       bvec.bv_len, sector);
            } else {
                // Write operation
                memcpy(device_data + (sector * SECTOR_SIZE) + bvec.bv_offset, buffer, bvec.bv_len);
                write_ops++;
                printk(KERN_DEBUG "SimpleBlock: Wrote %u bytes to sector %llu\n",
                       bvec.bv_len, sector);
            }
            
            kunmap(bvec.bv_page);
        }
        
        mutex_unlock(&device_mutex);
        __blk_end_request_all(req, 0);
    }
}

// Block device operations
static int block_open(struct block_device *bdev, fmode_t mode) {
    printk(KERN_INFO "SimpleBlock: Device opened by process %d\n", current->pid);
    return 0;
}

static void block_release(struct gendisk *disk, fmode_t mode) {
    printk(KERN_INFO "SimpleBlock: Device closed\n");
}

static int block_ioctl(struct block_device *bdev, fmode_t mode,
                       unsigned int cmd, unsigned long arg) {
    if (cmd == BLKGETSIZE) {
        // Return size in sectors
        put_user(device_sectors, (unsigned long *)arg);
        return 0;
    }
    
    if (cmd == BLKGETSIZE64) {
        // Return size in bytes
    u64 size = (u64)device_sectors * SECTOR_SIZE;
        if (copy_to_user((u64 *)arg, &size, sizeof(size)))
            return -EFAULT;
        return 0;
    }
    
    return -ENOTTY;
}

static struct block_device_operations block_ops = {
    .owner = THIS_MODULE,
    .open = block_open,
    .release = block_release,
    .ioctl = block_ioctl,
};

static int __init block_init(void) {
    printk(KERN_INFO "SimpleBlock: Initializing enhanced driver\n");
    
    // Allocate major number
    major_number = register_blkdev(0, DEVICE_NAME);
    if (major_number <= 0) {
        printk(KERN_ERR "SimpleBlock: Failed to register block device\n");
        return -EBUSY;
    }
    
    printk(KERN_INFO "SimpleBlock: Registered with major number %d\n", major_number);
    
    // Allocate memory for device data
    device_data = vmalloc(device_sectors * SECTOR_SIZE);
    if (!device_data) {
        printk(KERN_ERR "SimpleBlock: Failed to allocate device memory\n");
        unregister_blkdev(major_number, DEVICE_NAME);
        return -ENOMEM;
    }
    memset(device_data, 0, device_sectors * SECTOR_SIZE);
    
    // Initialize with a welcome message
    const char *welcome_msg = "=== Simple Block Device Storage ===\n"
                             "Total sectors: %lu\n"
                             "Total size: %lu KB\n"
                             "Use this device for block I/O operations\n";
    char init_msg[512];
    snprintf(init_msg, sizeof(init_msg), welcome_msg, 
             device_sectors, (device_sectors * SECTOR_SIZE) / 1024);
    memcpy(device_data, init_msg, strlen(init_msg));
    
    // Create request queue
    queue = blk_init_queue(simple_block_request, NULL);
    if (!queue) {
        printk(KERN_ERR "SimpleBlock: Failed to create request queue\n");
        vfree(device_data);
        unregister_blkdev(major_number, DEVICE_NAME);
        return -ENOMEM;
    }
    
    // Set queue parameters
    blk_queue_logical_block_size(queue, SECTOR_SIZE);
    blk_queue_physical_block_size(queue, SECTOR_SIZE);
    queue->queuedata = device_data;
    
    // Create gendisk structure
    simple_disk = alloc_disk(1);
    if (!simple_disk) {
        printk(KERN_ERR "SimpleBlock: Failed to allocate disk structure\n");
        blk_cleanup_queue(queue);
        vfree(device_data);
        unregister_blkdev(major_number, DEVICE_NAME);
        return -ENOMEM;
    }
    
    // Set up the disk
    simple_disk->major = major_number;
    simple_disk->first_minor = 0;
    simple_disk->fops = &block_ops;
    simple_disk->queue = queue;
    simple_disk->private_data = device_data;
    snprintf(simple_disk->disk_name, DISK_NAME_LEN, DEVICE_NAME);
    set_capacity(simple_disk, device_sectors);
    
    // Add disk to the system
    add_disk(simple_disk);
    
    mutex_init(&device_mutex);
    
    printk(KERN_INFO "SimpleBlock: Driver initialized successfully\n");
    printk(KERN_INFO "SimpleBlock: Device size: %lu sectors (%lu KB)\n",
           device_sectors, (device_sectors * SECTOR_SIZE) / 1024);
    printk(KERN_INFO "SimpleBlock: Device node: /dev/%s\n", DEVICE_NAME);
    
    return 0;
}

static void __exit block_exit(void) {
    if (simple_disk) {
        del_gendisk(simple_disk);
        put_disk(simple_disk);
    }
    
    if (queue) {
        blk_cleanup_queue(queue);
    }
    
    if (device_data) {
        vfree(device_data);
    }
    
    if (major_number) {
        unregister_blkdev(major_number, DEVICE_NAME);
    }
    
    mutex_destroy(&device_mutex);
    
    printk(KERN_INFO "SimpleBlock: Driver removed\n");
    printk(KERN_INFO "SimpleBlock: Total reads: %lu, writes: %lu\n", read_ops, write_ops);
}

module_init(block_init);
module_exit(block_exit);
