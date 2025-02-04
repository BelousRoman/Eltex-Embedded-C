#ifndef _PROCFS_MODULE_H
#define _PROCFS_MODULE_H

/*
* Eltex's academy part of homework #25 for lecture 55 
* "Sys and Proc RAM Filesystems".
*
* Procfs module
*
* This code compiles into kernel object file, which creates a directory
* PROC_DIR_NAME inside of /proc/ directory with PROC_FILE_NAME file. This file
* allows access to module internal variable 'msg', containing a char array of
* bytes.
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/string.h>

#define PROC_DIR_NAME                       "example"
#define PROC_FILE_NAME                      "string"

#define BUF_LEN								15

char msg[BUF_LEN] = "String\n\0";

/* global variables for procfs folder and file */
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_file;

static ssize_t read_proc(struct file *file, char __user *buffer, size_t length, loff_t *offset);

static ssize_t write_proc(struct file *file, const char __user *buffer, size_t length, loff_t *offset);

#endif // _PROCFS_MODULE_H