#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/syscall_wrapper.h>
#include <linux/sched.h>
#include <linux/init_task.h>
#include <linux/init.h>
#include <linux/pid.h>
#include <linux/kernel.h>
#include <linux/ftrace.h>
#include <linux/unistd.h>
//#include <include/asm-generic/current.h>
#define __NR_ftrace 336

void **syscall_table;
void *real_ftrace;
int sibling_count = 0;
int child_count = 0;

asmlinkage pid_t process_tracer(const struct pt_regs *regs)
{
    pid_t trace_task = (pid_t)regs->di;
    struct task_struct *task, *child, *sibling;
    struct list_head *list;
    task = pid_task(find_vpid(trace_task), PIDTYPE_PID);
    if (!task) {
        printk("return -1\n");
        return -1;  // 해당 PID를 가진 프로세스가 없을 경우 -1 반환
    }
    printk("##### TASK INFORMATION of ''[%d] %s'' #####\n", trace_task, task->comm);
    if(task->state == 0x0000)
        printk("- task state : Running or ready\n");
    else if(task->state == 0x0001)
        printk("- task state : Wait\n");
    else if(task->state == 0x0002)
        printk("- task state : Wait with ignoring all signals\n");
    else if(task->state == 0x0004)
        printk("- task state : Stopped\n");
    else if(task->exit_state == 0x0010)
        printk("- task state : Dead\n");
    else if(task->exit_state == 0x0020)
        printk("- task state : Zombie process\n");
    else
        printk("- task state : etc.\n");
    printk("- Process Group Leader : [%d] %s\n", task->group_leader->pid, task->group_leader->comm);
    printk("- Number of context switches: %lu\n", task->nvcsw + task->nivcsw);
    printk("- Number of calling fork() : %d\n", task->fork_count);
    printk("- it's parent process: [%d] %s\n", task->real_parent->pid, task->real_parent->comm);
    printk("- it's sibling process(es) : \n");
    list_for_each(list, &task->real_parent->children) {
        sibling = list_entry(list, struct task_struct, sibling);
        if (sibling->pid != trace_task) {
            printk("        > [%d] %s\n", sibling->pid, sibling->comm);
            sibling_count++;
        }
    }
    if (sibling_count != 0)
        printk("        > This process has %d sibling process(es)\n", sibling_count);
    else
        printk("        > It has no sibling\n");
    printk("- it's child process(es) : \n");
    list_for_each(list, &task->children) {
        child = list_entry(list, struct task_struct, sibling);
        if (child != current) {
            printk("        > [%d] %s\n", child->pid, child->comm);
            child_count++;
        }
    }
    if (child_count != 0)
        printk("        > This process has %d child process(es)\n", child_count);
    else
        printk("        > It has no child\n");
    printk("##### END OF INFORMATION #####\n");
    return trace_task;
    return 0;
}
void make_ro(void *addr) {
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);
    pte->pte = pte->pte &~ _PAGE_RW;
}
void make_rw(void *addr) {
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);
    if (pte->pte &~ _PAGE_RW)
        pte->pte |= _PAGE_RW;
}
static int __init tracer_init(void) {
    syscall_table = (void**) kallsyms_lookup_name("sys_call_table");
    make_rw(syscall_table);
    real_ftrace = syscall_table[__NR_ftrace];
    syscall_table[__NR_ftrace] = process_tracer;
    return 0;
}
static void __exit tracer_exit(void) {
    syscall_table[__NR_ftrace] = real_ftrace;
    make_ro(syscall_table);
}
module_init(tracer_init);
module_exit(tracer_exit);
MODULE_LICENSE("GPL");