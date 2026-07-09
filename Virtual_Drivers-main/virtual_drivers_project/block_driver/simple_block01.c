#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/bio.h>
#include <linux/vmalloc.h>

#define DEVICE_NAME "simple_block"
#define SECTOR_SIZE 512
#define DEVICE_SIZE_SECTORS 1024  // 512KB device
#define DEVICE_SIZE (DEVICE_SIZE_SECTORS * SECTOR_SIZE)

static struct block_device_operations block_ops;
static struct gendisk *simple_disk = NULL;
static struct request_queue *queue = NULL;
static u8 *device_data = NULL;

static int major_number = 0;

// Process read/write requests
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
        
        if ((sector + bytes / SECTOR_SIZE) > DEVICE_SIZE_SECTORS) {
            printk(KERN_ERR "SimpleBlock: Request beyond device limits\n");
            __blk_end_request_all(req, -EIO);
            continue;
        }
        
        // Process each segment of the request
        rq_for_each_segment(bvec, req, iter) {
            buffer = page_address(bvec.bv_page) + bvec.bv_offset;
            
            if (rq_data_dir(req) == READ) {
                // Read operation
                memcpy(buffer, device_data + (sector * SECTOR_SIZE) + bvec.bv_offset, bvec.bv_len);
                printk(KERN_INFO "SimpleBlock: Read %u bytes from sector %llu\n",
                       bvec.bv_len, sector);
            } else {
                // Write operation
                memcpy(device_data + (sector * SECTOR_SIZE) + bvec.bv_offset, buffer, bvec.bv_len);
                printk(KERN_INFO "SimpleBlock: Wrote %u bytes to sector %llu\n",
                       bvec.bv_len, sector);
            }
        }
        
        __blk_end_request_all(req, 0);
    }
}

// Block device operations
static int block_open(struct block_device *bdev, fmode_t mode) {
    printk(KERN_INFO "SimpleBlock: Device opened\n");
    return 0;
}

static void block_release(struct gendisk *disk, fmode_t mode) {
    printk(KERN_INFO "SimpleBlock: Device closed\n");
}

static struct block_device_operations block_ops = {
    .owner = THIS_MODULE,
    .open = block_open,
    .release = block_release,
};

static int __init block_init(void) {
    dev_t dev_num;
    
    printk(KERN_INFO "SimpleBlock: Initializing driver\n");
    
    // Allocate major number
    if (register_blkdev(0, DEVICE_NAME) < 0) {
        printk(KERN_ERR "SimpleBlock: Failed to register block device\n");
        return -EBUSY;
    }
    
    major_number = register_blkdev(0, DEVICE_NAME);
    
    // Allocate memory for device data
    device_data = vmalloc(DEVICE_SIZE);
    if (!device_data) {
        printk(KERN_ERR "SimpleBlock: Failed to allocate device memory\n");
        unregister_blkdev(major_number, DEVICE_NAME);
        return -ENOMEM;
    }
    memset(device_data, 0, DEVICE_SIZE);
    
    // Initialize a simple message
    const char *init_msg = "Welcome to Simple Block Device!";
    memcpy(device_data, init_msg, strlen(init_msg));
    
    // Create request queue
    queue = blk_init_queue(simple_block_request, NULL);
    if (!queue) {
        printk(KERN_ERR "SimpleBlock: Failed to create request queue\n");
        vfree(device_data);
        unregister_blkdev(major_number, DEVICE_NAME);
        return -ENOMEM;
    }
    
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
    set_capacity(simple_disk, DEVICE_SIZE_SECTORS);
    
    // Add disk to the system
    add_disk(simple_disk);
    
    printk(KERN_INFO "SimpleBlock: Driver initialized successfully\n");
    printk(KERN_INFO "SimpleBlock: Device size: %d KB\n", DEVICE_SIZE / 1024);
    
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
    
    printk(KERN_INFO "SimpleBlock: Driver removed\n");
}

module_init(block_init);
module_exit(block_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple Block Device Driver");
MODULE_VERSION("1.0");
