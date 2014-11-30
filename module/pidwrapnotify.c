#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/tracepoint.h>
#include <linux/sched.h>

static struct tracepoint *sched_process_fork_tp;

static void sched_process_fork_probe(void *data, struct task_struct *parent,
				     struct task_struct *child)
{
	pr_info("forked! parent=%d child=%d\n", parent->pid, child->pid);
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

static int __init init(void)
{
	int ret = connect_probes();

	if (ret)
		return ret;

	return 0;
}

static void __exit fini(void)
{
	tracepoint_probe_unregister(sched_process_fork_tp,
				    sched_process_fork_probe, NULL);
	tracepoint_synchronize_unregister();
}

module_init(init);
module_exit(fini);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Philippe Proulx <eepp.ca>");
MODULE_DESCRIPTION("PID wrapping notifier");
