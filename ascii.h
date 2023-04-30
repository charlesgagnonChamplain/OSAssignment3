/****
 * ascii.h
 *
 * Mostly macros and prototypes for the ASCII character device
 * driver.
 *
 * CREDITS:
 *   o Many parts of the driver code has to be credited to
 *     Ori Pomerantz, in his chardev.c (Copyright (C) 1998-1999)
 *
 *     Source:  The Linux Kernel Module Programming Guide (specifically,
 *              http://www.tldp.org/LDP/lkmpg/2.6/html/index.html)
 */

#ifndef _ASCII_DEVICE_H
#define _ASCII_DEVICE_H

/* The necessary header files */

/* Standard in kernel modules */
#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/ioctl.h>

/* For character devices */
#include <linux/fs.h>       /* The character device
                             * definitions are here
                             */

#include <asm/uaccess.h>  /* for put/get_user */

/* Return codes */
#define SUCCESS      0

/* Device Declarations **************************** */

/* The maximum length of the message from the device */
#define BSIZE 2048
#define BUF_SIZE (BSIZE * BSIZE + BSIZE + 1)

#define MAJOR_NUM 	130
#define IOCTL_RESET_MAP 	_IO(MAJOR_NUM, 0) /*reset to the default map*/
#define IOCTL_ZERO_OUT		_IO(MAJOR_NUM, 1) /*zeros out the buffer */
#define IOCTL_CHECK_CONSISTENCY _IO(MAJOR_NUM, 2) /*checks that consistency*/

/* The name for our device, as it will appear
 * in /proc/devices
 */
#define DEVICE_NAME  "/dev/asciimap"

/*
 * Driver status structure
 */
typedef struct _driver_status
{
	/* An ASCII character to be outputted in the next read
	 */
	char  curr_char;

	/* Is the device open right now? Used to prevent
	 * concurent access into the same device
	 */
	bool  busy;

	/* The message the device will give when asked */
	char  buf[BUF_SIZE];

	/* How far did the process reading the message
	 * get? Useful if the message is larger than the size
	 * of the buffer we get to fill in device_read.
	 */

	/* Map size */
	int map_byte_length;

	char* buf_ptr;

	/* The major device number for the device.
	 */
	int   major;

	/* The minor device number for the device.
	 */
	int   minor;

} driver_status_t;


/*
 * Driver funcitons' prototypes
 */
static int device_open(struct inode*, struct file*);
static int  device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);
static loff_t device_seek(struct file *, loff_t, int);
static long device_ioctl(struct file*, unsigned int, unsigned long);

/* Kernel module-related */

/* Module Declarations ***************************** */


/* This structure will hold the functions to be
 * called when a process does something to the device
 * we created. Since a pointer to this structure is
 * kept in the devices table, it can't be local to
 * init_module. NULL is for unimplemented functions.
 */
struct file_operations Fops =
{
	.open = device_open, /* open */
	.read = device_read, /* read */
	.write = device_write, /* write */
	.release = device_release,  /* a.k.a. close */
	.llseek = device_seek,   /* seek */
	.unlocked_ioctl = device_ioctl   /* ioctl */
#if 0
	.owner = NULL,   /* owner */
	.readdir = NULL,   /* readdir */
	.poll = NULL,   /* poll/select */
	.mmap = NULL,   /* mmap */
	.flush = NULL,   /* flush */
#endif
};


int init_module(void);
void cleanup_module(void);

#endif /* _ASCII_DEVICE_H */
