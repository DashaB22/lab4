
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/time.h>

#define PROC_FILENAME "tsulab"
#define PROC_MESSAGE_PREFIX "Welcome to the TSU Linux Lab
"
#define MAX_MESSAGE_SIZE 256

static int speed_kmh = 5;

static struct proc_dir_entry *proc_file;

static ssize_t tsu_read_proc(struct file *file, char __user *buf, size_t count, loff_t *offset) {
    struct timeval tv;
    struct tm tm;
    long seconds_to_new_year;
    int distance;
    char message[MAX_MESSAGE_SIZE];

    do_gettimeofday(&tv);
    time_to_tm(tv.tv_sec, 0, &tm);

    struct tm new_year_tm = { .tm_year = tm.tm_year + 1, .tm_mon = 0, .tm_mday = 1, .tm_hour = 0, .tm_min = 0, .tm_sec = 0 };
    seconds_to_new_year = mktime(&new_year_tm) - tv.tv_sec;

    int speed_mps = speed_kmh * 1000 / 3600;

    distance = seconds_to_new_year * speed_mps;

    int len = snprintf(message, sizeof(message),
        "%s
Distance you can travel on a road roller from now until New Year with speed %d km/h: %d meters
", 
        PROC_MESSAGE_PREFIX, speed_kmh, distance);

    return simple_read_from_buffer(buf, count, offset, message, len);
}

static ssize_t tsu_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *offset) {
    char input[MAX_MESSAGE_SIZE];
    int new_speed;

    if (count >= MAX_MESSAGE_SIZE)
        return -EINVAL;

    if (copy_from_user(input, buf, count))
        return -EFAULT;

    input[count] = '\0';

    if (sscanf(input, "%d", &new_speed) != 1)
        return -EINVAL;

    if (new_speed > 0) {
        speed_kmh = new_speed;
        printk(KERN_INFO "Updated speed to %d km/h\n", speed_kmh);
    } else {
        return -EINVAL;
    }

    return count;
}

static const struct proc_ops proc_file_ops = {
    .proc_read = tsu_read_proc,
    .proc_write = tsu_write_proc,
};

static int __init tsu_module_init(void) {
    printk(KERN_INFO "Welcome to the Tomsk State University\n");

    proc_file = proc_create(PROC_FILENAME, 0666, NULL, &proc_file_ops);
    if (!proc_file) {
        printk(KERN_ERR "Failed to create /proc/%s\n", PROC_FILENAME);
        return -ENOMEM;
    }

    printk(KERN_INFO "/proc/%s created\n", PROC_FILENAME);
    return 0;
}

static void __exit tsu_module_exit(void) {
    proc_remove(proc_file);
    printk(KERN_INFO "Tomsk State University forever!\n");
}

module_init(tsu_module_init);
module_exit(tsu_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomsk State University");
MODULE_DESCRIPTION("TSU Linux Kernel Module");
MODULE_VERSION("1.0");
