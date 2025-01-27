/*
* Eltex's academy homework #24 for lectures 53-54 "Linux Kernel Modules"
*
* This is a simple character device driver that sends to user buffer an array
* of chars on reading /dev/test_module file and allows to modify this array by
* writing to the same file.
*
* There might be a problem writing to chardev file even with superuser rights,
* the following call should help:
*   chmod 0666 /dev/test_module
*
* Logs can be accessed by calling dmesg like this:
*   sudo dmesg | tail -n10
*/
#include <linux/module.h>
#include <linux/kernel.h> /* for sprintf() */
#include <linux/fs.h>
#include <linux/rwlock.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/version.h>

/* Set this to manually create file in /dev/ dir,
* Example: 'mknod test_module c <major> <minor>'
*/
#define MKNOD_DEV_FILE              0
/* Redefine chardev file handlers for openning and releasing the file. */
#define REDEF_OPEN_RELEASE_HNDLRS   0
/* For some reason setting this to 0 aka using simple_read_from_buffer instead
* of put_user gives Bad Address error (code: -14) on reading chardev file with
* 'cat' util, althought 'less -f /dev/test_module' works fine.
*/
#define OLDER_READ_HANDLER          1

#if REDEF_OPEN_RELEASE_HNDLRS
    #include <linux/atomic.h>
#endif

#if OLDER_READ_HANDLER
    #include <linux/uaccess.h> /* for get_user() and put_user() */
#endif

/* Prototypes- this would normally go in a .h file */
#if REDEF_OPEN_RELEASE_HNDLRS
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
#endif
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, 
                            loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "test_module" /* Dev name as it appears in /proc/devices */
#define BUF_LEN 15 /* Max length of the message from the device */

/* Global variables are declared as static, so are global only within the file, not whole kernel. */
static int major; /* major number assigned to our device driver */
static rwlock_t lock;
#if REDEF_OPEN_RELEASE_HNDLRS
enum {
    CDEV_NOT_USED,
    CDEV_EXCLUSIVE_OPEN,
};
#endif

#if REDEF_OPEN_RELEASE_HNDLRS
/* Used to prevent multiple access to device */
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);
#endif
/* The msg the device will return upon read */
static char msg[BUF_LEN] = "A string\0"; 
#if !MKNOD_DEV_FILE
static struct class *cls;
#endif
static struct file_operations module_fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
#if REDEF_OPEN_RELEASE_HNDLRS
    .open = device_open,
    .release = device_release,
#endif
};

static int __init test_module_init(void)
{
    pr_info("[TEST MODULE] Module has loaded!\n");
    rwlock_init(&lock);
    major = register_chrdev(0, DEVICE_NAME, &module_fops);
    if(major < 0)
    {
        pr_alert("[TEST MODULE] Failed to register char device: %d\n", major);
        return major;
    }
    pr_info("[TEST MODULE] Major number: %d.\n", major);

#if !MKNOD_DEV_FILE
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    cls = class_create(DEVICE_NAME);
    #else
    cls= class_create(THIS_MODULE, DEVICE_NAME);
    #endif
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    pr_info("[TEST MODULE] Device created on /dev/%s\n", DEVICE_NAME);
#endif
    return SUCCESS;
}

static void __exit test_module_cleanup(void)
{
#if !MKNOD_DEV_FILE
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
#endif
    /*Unregister the device */
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("[TEST MODULE] Cleanup module\n");
}

/* Methods */
#if REDEF_OPEN_RELEASE_HNDLRS
/*  Called when a process tries to open the device file, like
*   "sudo cat /dev/test_module"
*/
static int device_open(struct inode *inode, struct file *file)
{
    if(atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN))
        return -EBUSY;

    try_module_get(THIS_MODULE);
    return SUCCESS;
}

/* Called when a process closes the device file. */
static int device_release(struct inode *inode, struct file *file)
{
    /*We're now ready for our next caller */
    atomic_set(&already_open, CDEV_NOT_USED);
    /*Decrement the usage count, or else once you opened the file, you will
    *never get rid of the module.
    */
    module_put(THIS_MODULE);
    return SUCCESS;
}
#endif

/* Called when a process, which already opened the dev file, attempts to
* read from it.
*/
static ssize_t device_read(struct file *filp, /* see include/linux/fs.h */
                            char __user *buffer, /* buffer to fill with data */
                            size_t length, /* length of the buffer */
                            loff_t *offset)
{
#if !OLDER_READ_HANDLER
    size_t ret;

    /* Block here if there is write lock, otherwise nothing will happen */
    read_lock(&lock);
    ret = simple_read_from_buffer(buffer, length, offset, msg, BUF_LEN);
    read_unlock(&lock);

    return ret;
#else
    /*Number of bytes actually written to the buffer */
    int bytes_read = 0;
    const char *msg_ptr = msg;

    if(!*(msg_ptr + *offset))
    { /* we are at the end of message */
        *offset = 0; /* reset the offset */
        return 0; /* signify end of file */
    }

    msg_ptr += *offset;
    /*Actually put the data into the buffer */
    while (length && *msg_ptr)
    {
        /* The buffer is in the user data segment, not the kernel
        * segment so "*" assignment won't work. We have to use
        * put_user which copies data from the kernel data segment to
        * the user data segment.
        */
        put_user(*(msg_ptr++), buffer++);
        length--;
        bytes_read++;
    }
    *offset += bytes_read;
    /*Most read functions return the number of bytes put into the buffer. */
    return bytes_read;
#endif
}

/* Called when a process writes to dev file: echo "string" > /dev/test_module sudo chmod 0666 /dev/test_module then echo to file*/
static ssize_t device_write(struct file *filp, const char __user *buff,
                            size_t len, loff_t *off)
{
    size_t ret = 0;

    if (len > BUF_LEN)
    {
        pr_alert("[TEST MODULE] Provided argument exceed maximum length of %d bytes!\n", BUF_LEN);
        return -EINVAL;
    }

    write_lock(&lock);
    ret = simple_write_to_buffer(msg, BUF_LEN, off, buff, len);
    write_unlock(&lock);

    pr_info("[TEST MODULE] String has been overwritten!\n");
    return ret;
}

module_init(test_module_init);
module_exit(test_module_cleanup);

MODULE_LICENSE("GPL");
