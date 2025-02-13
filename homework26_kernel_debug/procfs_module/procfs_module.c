#include "procfs_module.h"

static struct proc_ops p_ops = {
	.proc_read = read_proc,
	.proc_write = write_proc,
};

/**
 * @brief Read data out of the buffer
 */
static ssize_t read_proc(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
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
}

/**
 * @brief Write data to buffer
 */
static ssize_t write_proc(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
	size_t ret = 0;

    if (length > BUF_LEN)
    {
        pr_alert("[TEST MODULE] Provided argument exceed maximum length of %d bytes!\n", BUF_LEN);
        return -EINVAL;
    }

	memset(msg, 0, BUF_LEN);
    ret = simple_write_to_buffer(msg, BUF_LEN, offset, buffer, length);

    pr_info("[TEST MODULE] String has been overwritten to <%s>!\n", msg);
    return ret;
}


static int __init example_init(void)
{
	pr_info("[TEST MODULE] Module has loaded!\n");

	proc_dir = proc_mkdir(PROC_DIR_NAME, NULL);
	if(proc_dir == NULL)
		goto dir_error;

	proc_file = proc_create(PROC_FILE_NAME, 0666, proc_dir, &p_ops);
	if(proc_file == NULL)
		goto file_error;

	pr_info("[TEST MODULE] File /proc/%s/%s has been created!\n", PROC_DIR_NAME, PROC_FILE_NAME);
	
	return 0;

file_error:
	proc_remove(proc_dir);
dir_error:
	pr_err("[TEST MODULE] Failed to establish Proc filesystem file!\n");
	return -ENOMEM;
}

static void __exit example_exit(void)
{
	proc_remove(proc_file);
	proc_remove(proc_dir);

	pr_info("[TEST MODULE] Unload Module!\n");
}

module_init(example_init);
module_exit(example_exit);

MODULE_LICENSE("GPL");