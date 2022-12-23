#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/pid.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/mm_types.h>
#include "linux/mm.h"
#include <linux/sched/cputime.h>
#include <linux/sched/types.h>

#define BUF_SIZE 512

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Цыпандин Николай Петрович");
MODULE_DESCRIPTION("Lab2 | procfs: task_cputime, vm_area_struct");
MODULE_VERSION("1.0");

static struct proc_dir_entry *parent;
static int pid;

static int __init lab_driver_init(void);
static void __exit lab_driver_exit(void);

static int open_proc(struct inode *inode, struct file *file);
static int release_proc(struct inode *inode, struct file *file);
static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length,
			 loff_t *offset);
static ssize_t write_proc(struct file *filp, const char __user *buff,
			  size_t len, loff_t *off);

static struct file_operations proc_fops = { .open = open_proc,
					    .read = read_proc,
					    .write = write_proc,
					    .release = release_proc };

static int open_proc(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Файл открыт");
	return 0;
}

static int release_proc(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Файл закрыт");
	return 0;
}

static size_t write_vm_area_struct(char __user *ubuf, struct task_struct *ts)
{
	struct mm_struct *mm = ts->mm;
	struct vm_area_struct *vm = mm->mmap;

	size_t len = 0;
	char buf[BUF_SIZE];

	if (mm == NULL) {
		len += sprintf(
			buf + len,
			"\nУ task_struct отсутствует mm, получить vm_area_struct невозможно!\n");

		if (copy_to_user(ubuf, buf, len)) {
			printk(KERN_ERR
			       "Ошибка копирования данных пользователю!");
			return -1;
		}
		return len;
	}

	len += sprintf(buf + len, "\nИнформация о vm_area_struct PID = %d:\n ",
		       pid);
	len += sprintf(buf + len, "\tvm_start = %lu\n", vm->vm_start);
	len += sprintf(buf + len, "\tvm_end = %lu\n", vm->vm_end);
	len += sprintf(buf + len, "\tvm_flags = %lu\n", vm->vm_flags);
	len += sprintf(buf + len, "\tvm_offset = %lu\n\n", vm->vm_pgoff);

	if (copy_to_user(ubuf, buf, len)) {
		printk(KERN_ERR "Ошибка копирования данных пользователю!");
		return -1;
	}
	return len;
}

static size_t write_task_cputime_struct(char __user *ubuf,
					struct task_struct *ts)
{
	char buf[BUF_SIZE];
	size_t len = 0;

	unsigned long long stime;
	unsigned long long utime;
	unsigned long long sum_exec_runtime;

	task_cputime(ts, &utime, &stime);
	sum_exec_runtime = ts->se.sum_exec_runtime;

	len += sprintf(buf + len, "\nИнформация о task_cputime PID = %d:\n ",
		       pid);
	len += sprintf(buf + len, "\tstime = %llu\n", stime);
	len += sprintf(buf + len, "\tutime = %llu\n", utime);
	len += sprintf(buf + len, "\tsum_exec_runtime = %llu\n\n",
		       sum_exec_runtime);

	if (copy_to_user(ubuf, buf, len)) {
		printk(KERN_ERR "Ошибка копирования данных пользователю!");
		return -1;
	}
	return len;
}

static ssize_t read_proc(struct file *filp, char __user *ubuf, size_t count,
			 loff_t *ppos)
{
	struct task_struct *ts = get_pid_task(find_get_pid(pid), PIDTYPE_PID);

	char buf[BUF_SIZE];
	size_t len = 0;
	int struct_id;

	printk(KERN_INFO "Происходит чтение из файла...");

	if (*ppos > 0) {
		return -1;
	}

	if (ts == NULL) {
		len += sprintf(buf, "\nОшибка! Заданного PID не существует!\n");

		if (copy_to_user(ubuf, buf, len)) {
			printk(KERN_ERR
			       "Ошибка копирования данных пользователю!");
			return -1;
		}
		*ppos = len;
		return len;
	}

	sscanf(ubuf, "%d", &struct_id);
    printk(KERN_INFO "Считан struct_id=%d", struct_id);



	switch (struct_id) {
	case 1:
		len += write_task_cputime_struct(ubuf, ts);
		break;
	case 2:

		len += write_vm_area_struct(ubuf, ts);
		break;
	default:
		len += sprintf(buf,
			       "\nОшибка! Заданной struct_id не существует!\n");

		if (copy_to_user(ubuf, buf, len)) {
			printk(KERN_ERR
			       "Ошибка копирования данных пользователю!");
			return -1;
		}
		return -1;
	}

	*ppos = len;
	return len;
}

static ssize_t write_proc(struct file *filp, const char __user *ubuf,
			  size_t count, loff_t *ppos)
{
	char buf[BUF_SIZE];

	printk(KERN_INFO "Происходит запись в файл...");
	if (*ppos > 0 || count > BUF_SIZE) {
		return -1;
	}

	if (copy_from_user(buf, ubuf, count)) {
		printk(KERN_ERR "Ошибка копирования данных от пользователя!");
		return -1;
	}

	sscanf(buf, "%d", &pid);
    printk(KERN_INFO "Считан pid=%d", pid);

	*ppos = strlen(buf);
    printk(KERN_INFO "Запись прошла успешно");
	return strlen(buf);
}

static int __init lab_driver_init(void)
{
	parent = proc_mkdir("Lab2", NULL);
	if (parent == NULL) {
		printk(KERN_ERR "Ошибка при создании proc директории!");
		return -1;
	}
	proc_create("my_driver", 0666, parent, &proc_fops);
	printk(KERN_INFO "Драйвер загружен!");
	return 0;
}

static void __exit lab_driver_exit(void)
{
	proc_remove(parent);
	printk(KERN_INFO "Драйвер удален!");
}

module_init(lab_driver_init);
module_exit(lab_driver_exit);
