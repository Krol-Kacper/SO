#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/kernel_stat.h>
#include <linux/smp.h>
MODULE_LICENSE("GPL");
static struct timer_list cpu_timer;
static unsigned long prev_idle[NR_CPUS], prev_total[NR_CPUS];
static unsigned int c;
module_param(log_interval, uint, 0644);
MODULE_PARM_DESC(log_interval, "Interval (in seconds) between CPU utilization
logs");
static void log_cpu_utilization(struct timer_list *t) {
unsigned long cpu, idle_time, total_time, delta_idle, delta_total;
unsigned long usage;
for_each_online_cpu(cpu) {
struct kernel_cpustat *kcs = &kcpustat_cpu(cpu);
idle_time = kcs->cpustat[CPUTIME_IDLE];
total_time = kcs->cpustat[CPUTIME_USER] +
kcs->cpustat[CPUTIME_NICE] +
kcs->cpustat[CPUTIME_SYSTEM] +
kcs->cpustat[CPUTIME_IRQ] +
kcs->cpustat[CPUTIME_SOFTIRQ] +
kcs->cpustat[CPUTIME_STEAL] +
kcs->cpustat[CPUTIME_IDLE] +
kcs->cpustat[CPUTIME_IOWAIT];
delta_idle = idle_time - prev_idle[cpu];
delta_total = total_time - prev_total[cpu];
if (delta_total > 0)
usage = (100 * (delta_total - delta_idle)) / delta_total;
else
usage = 0;
pr_info("CPU %lu Utilization: %lu%%\n", cpu, usage);
prev_idle[cpu] = idle_time;
prev_total[cpu] = total_time;
}
mod_timer(&cpu_timer, jiffies + log_interval * HZ);
}
static int __init cpu_logger_init(void) {
pr_info("CPU Utilization Logger initialized with interval: %u seconds\n",
log_interval);
memset(prev_idle, 0, sizeof(prev_idle));
memset(prev_total, 0, sizeof(prev_total));
timer_setup(&cpu_timer, log_cpu_utilization, 0);
mod_timer(&cpu_timer, jiffies + log_interval * HZ);
return 0;
}
static void __exit cpu_logger_exit(void) {
del_timer(&cpu_timer);
pr_info("CPU Utilization Logger exited\n");
}
module_init(cpu_logger_init);
module_exit(cpu_logger_exit);