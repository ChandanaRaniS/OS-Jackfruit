#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include "monitor_ioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("Container Memory Monitor");

// Linked list node
struct process_node {
    pid_t pid;
    unsigned long soft_limit;
    unsigned long hard_limit;
    int soft_triggered;
    struct list_head list;
};

static LIST_HEAD(process_list);
static DEFINE_MUTEX(list_lock);

static int major;
static struct task_struct *monitor_thread;

// ================= DEVICE =================

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct monitor_request req;
    struct process_node *node, *tmp;

    switch (cmd) {

    case IOCTL_REGISTER:
        if (copy_from_user(&req, (void __user *)arg, sizeof(req)))
            return -EFAULT;

        node = kmalloc(sizeof(*node), GFP_KERNEL);
        if (!node) return -ENOMEM;

        node->pid = req.pid;
        node->soft_limit = req.soft_limit;
        node->hard_limit = req.hard_limit;
        node->soft_triggered = 0;

        mutex_lock(&list_lock);
        list_add(&node->list, &process_list);
        mutex_unlock(&list_lock);

        printk(KERN_INFO "[monitor] Registered PID %d\n", req.pid);
        break;

    case IOCTL_UNREGISTER:
        pid_t pid;
        if (copy_from_user(&pid, (void __user *)arg, sizeof(pid)))
            return -EFAULT;

        mutex_lock(&list_lock);
        list_for_each_entry_safe(node, tmp, &process_list, list) {
            if (node->pid == pid) {
                list_del(&node->list);
                kfree(node);
                printk(KERN_INFO "[monitor] Unregistered PID %d\n", pid);
                break;
            }
        }
        mutex_unlock(&list_lock);
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

static struct file_operations fops = {
    .unlocked_ioctl = device_ioctl,
};

// ================= MEMORY CHECK =================

static unsigned long get_rss(struct task_struct *task)
{
    if (!task || !task->mm) return 0;
    return get_mm_rss(task->mm) << PAGE_SHIFT; // bytes
}

// ================= MONITOR THREAD =================

static int monitor_fn(void *data)
{
    struct process_node *node, *tmp;

    while (!kthread_should_stop()) {

        mutex_lock(&list_lock);

        list_for_each_entry_safe(node, tmp, &process_list, list) {

            struct task_struct *task;
            unsigned long rss;

            task = pid_task(find_vpid(node->pid), PIDTYPE_PID);

            if (!task) {
                list_del(&node->list);
                kfree(node);
                continue;
            }

            rss = get_rss(task);

            // SOFT LIMIT
            if (rss > node->soft_limit && !node->soft_triggered) {
                printk(KERN_WARNING "[monitor] PID %d exceeded SOFT limit (%lu MB)\n",
                       node->pid, rss >> 20);
                node->soft_triggered = 1;
            }

            // HARD LIMIT
            if (rss > node->hard_limit) {
                printk(KERN_ERR "[monitor] PID %d exceeded HARD limit. Killing...\n",
                       node->pid);

                send_sig(SIGKILL, task, 0);

                list_del(&node->list);
                kfree(node);
            }
        }

        mutex_unlock(&list_lock);

        msleep(1000); // check every 1 sec
    }

    return 0;
}

// ================= INIT & EXIT =================

static int __init monitor_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);

    if (major < 0) {
        printk(KERN_ALERT "Register failed\n");
        return major;
    }

    printk(KERN_INFO "Monitor module loaded. Major: %d\n", major);

    monitor_thread = kthread_run(monitor_fn, NULL, "monitor_thread");

    return 0;
}

static void __exit monitor_exit(void)
{
    struct process_node *node, *tmp;

    if (monitor_thread)
        kthread_stop(monitor_thread);

    mutex_lock(&list_lock);
    list_for_each_entry_safe(node, tmp, &process_list, list) {
        list_del(&node->list);
        kfree(node);
    }
    mutex_unlock(&list_lock);

    unregister_chrdev(major, DEVICE_NAME);

    printk(KERN_INFO "Monitor module unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);