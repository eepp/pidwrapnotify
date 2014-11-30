#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/tracepoint.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

static struct tracepoint *sched_process_fork_tp;
static pid_t last_pid;
static dev_t devnum;
static struct cdev cdev;
static bool wrapped = false;
static DECLARE_WAIT_QUEUE_HEAD(wq);

static void sched_process_fork_probe(void *data, struct task_struct *parent,
				     struct task_struct *child)
{
	if (child->pid < last_pid - 1000) {
		pr_debug("pidwrapnotify: wrapping: child=%d last=%d\n",
			 child->pid, last_pid);
		wrapped = true;
		wake_up_interruptible(&wq);
	}

	last_pid = child->pid;
}

static void find_sched_process_fork_tp(struct tracepoint *tp, void *priv)
{
	if (!strcmp(tp->name, "sched_process_fork"))
		sched_process_fork_tp = tp;
}

static int connect_probes(void)
{
	int ret;

	/* find sched_process_fork tracepoint */
	for_each_kernel_tracepoint(find_sched_process_fork_tp, NULL);

	if (!sched_process_fork_tp)
		return -ENODEV;

	/* connect our probe to this tracepoint */
	ret = tracepoint_probe_register(sched_process_fork_tp,
					sched_process_fork_probe, NULL);

	if (ret)
		return ret;

	return 0;
}

static int cdev_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t cdev_read(struct file *fp, char *buf, size_t length,
			 loff_t *ppos)
{
	wait_event_interruptible(wq, wrapped);
	wrapped = false;

	return 0;
}

static int cdev_close(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations cdev_fops = {
	.owner = THIS_MODULE,
	.open = cdev_open,
	.read = cdev_read,
	.release = cdev_close,
};

static int create_cdev(void)
{
	int ret;

	/* get device number */
	ret = alloc_chrdev_region(&devnum, 0, 1, "pidwrapnotify");

	if (ret)
		return ret;

	/* initialize character device */
	cdev_init(&cdev, &cdev_fops);
	cdev.owner = THIS_MODULE;

	/* add character device */
	ret = cdev_add(&cdev, devnum, 1);

	if (ret)
		goto err_alloc_region;

	pr_info("pidwrapnotify: added char device %d:%d\n",
		MAJOR(devnum), MINOR(devnum));

	return 0;

err_alloc_region:
	unregister_chrdev_region(devnum, 1);

	return ret;
}

static void destroy_cdev(void)
{
	cdev_del(&cdev);

	if (devnum)
		unregister_chrdev_region(devnum, 1);
}

static int __init init(void)
{
	int ret;

	/* connect sched_process_fork probe */
	ret = connect_probes();

	if (ret)
		return ret;

	/* create character device */
	ret = create_cdev();

	if (ret)
		goto err_tp_unregister;

	return 0;

err_tp_unregister:
	tracepoint_probe_unregister(sched_process_fork_tp,
				    sched_process_fork_probe, NULL);

	return ret;
}

static void __exit fini(void)
{
	destroy_cdev();

	if (sched_process_fork_tp)
		tracepoint_probe_unregister(sched_process_fork_tp,
					    sched_process_fork_probe, NULL);

	tracepoint_synchronize_unregister();
}

module_init(init);
module_exit(fini);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Philippe Proulx <eepp.ca>");
MODULE_DESCRIPTION("PID wrapping notifier");
