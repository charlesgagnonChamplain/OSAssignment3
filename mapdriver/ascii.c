/****
 * ascii.c
 *
 * The ASCII character device driver implementation
 *
 * CREDITS:
 *   o Many parts of the driver code has to be credited to
 *     Ori Pomerantz, in his chardev.c (Copyright (C) 1998-1999)
 *
 *     Source:  The Linux Kernel Module Programming Guide (specifically,
 *              http://www.tldp.org/LDP/lkmpg/2.6/html/index.html)
 */

#include "ascii.h"

#define BSIZE 1024
static char buffer_data[BSIZE];
static int buffer_length = 0;
static int buffer_current_pointer = 0;

char initialsBuf[BSIZE*BSIZE];
char initials[] = "ACCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCH\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A             :::        ::::::::    :::::::::   H\n"
					"A          :+: :+:     :+:    :+:   :+:    :+:   H\n"
					"A        +:+   +:+    +:+          +:+    +:+    H\n"
					"A      +#++:++#++:   +#++:++#++   +#++:++#+      H\n"
					"A     +#+     +#+          +#+   +#+    +#+      H\n"
					"A    #+#     #+#   #+#    #+#   #+#    #+#       H\n"
					"A   ###     ###    ########    #########         H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                ::::::::       ::::::::         H\n"
					"A              :+:    :+:     :+:    :+:         H\n"
					"A             +:+            +:+                 H\n"
					"A            +#+            :#:                  H\n"
					"A           +#+            +#+   +#+#            H\n"
					"A          #+#    #+#     #+#    #+#             H\n"
					"A          ########       ########               H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A               :::    :::       :::    :::      H\n"
					"A              :+:    :+:       :+:    :+:       H\n"
					"A             +:+    +:+       +:+    +:+        H\n"
					"A            +#++:++#++       +#++:++#++         H\n"
					"A           +#+    +#+       +#+    +#+          H\n"
					"A          #+#    #+#       #+#    #+#           H\n"
					"A         ###    ###       ###    ###            H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"A                                                H\n"
					"ACCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCH\0";

/* Driver's Status is kept here */
static driver_status_t status =
{
	'a',   /* Starting ASCII char is '0' */
	false, /* Not busy at the beginning */
	{0},   /* buffer */
	NULL,  /* buffer's ptr */
	-1,    /* major */
	-1     /* minor */	
};


/* This function is called whenever a process
 * attempts to open the device file
 */
static int device_open(inode, file)
	struct inode* inode;
	struct file*  file;
{
	static int counter = 0;

#ifdef _DEBUG
	printk("device_open(%p,%p)\n", inode, file);
#endif

	/* This is how you get the minor device number in
	 * case you have more than one physical device using
	 * the driver.
	 */
	status.minor = inode->i_rdev >> 8;
	status.minor = inode->i_rdev & 0xFF;

	printk
	(
		"Device: %d.%d, busy: %d\n",
		status.major,
		status.minor,
		status.busy
	);

	/* We don't want to talk to two processes at the
	 * same time
	 */
	if(status.busy)
		return -EBUSY;

	/* If this was a process, we would have had to be
	 * more careful here.
	 *
	 * In the case of processes, the danger would be
	 * that one process might have check busy
	 * and then be replaced by the schedualer by another
	 * process which runs this function. Then, when the
	 * first process was back on the CPU, it would assume
	 * the device is still not open.
	 *
	 * However, Linux guarantees that a process won't be
	 * replaced while it is running in kernel context.
	 *
	 * In the case of SMP, one CPU might increment
	 * busy while another CPU is here, right after
	 * the check. However, in version 2.0 of the
	 * kernel this is not a problem because there's a lock
	 * to guarantee only one CPU will be kernel module at
	 * the same time. This is bad in  terms of
	 * performance, so version 2.2 changed it.
	 */

	status.busy = true;

	/* Initialize the message. */
	sprintf
	(
		status.buf,
		"If I told you once, I told you %d times - %s",
		counter++,
		"Hello, world\n"
	);

	/* The only reason we're allowed to do this sprintf
	 * is because the maximum length of the message
	 * (assuming 32 bit integers - up to 10 digits
	 * with the minus sign) is less than DRV_BUF_SIZE, which
	 * is 80. BE CAREFUL NOT TO OVERFLOW BUFFERS,
	 * ESPECIALLY IN THE KERNEL!!!
	 */

	status.buf_ptr = status.buf;

	return SUCCESS;
}


/* This function is called when a process closes the
 * device file.
 */
static int device_release(inode, file)
	struct inode* inode;
	struct file*  file;
{
#ifdef _DEBUG
	printk ("device_release(%p,%p)\n", inode, file);
#endif

	/* We're now ready for our next caller */
	status.busy = false;

	return SUCCESS;
}


/* This function is called whenever a process which
 * have already opened the device file attempts to
 * read from it.
 */
static ssize_t device_read(file, buffer, length, offset)
    struct file* file;
    char*        buffer;  /* The buffer to fill with data */
    size_t       length;  /* The length of the buffer */
    loff_t*      offset;  /* Our offset in the file */
{
    int bytes_read = 0;
    int lines_read = 0;
    char *map_ptr;

    // determine starting line, end line, and number of lines to read
    int start_line = *offset / (MAX_LINE_LEN + 1);
    int end_line = (*offset + length) / (MAX_LINE_LEN + 1);
    int num_lines = end_line - start_line + 1;

    if (*offset >= MAP_SIZE || num_lines <= 0) {
        return 0; // end of file
    }

    // adjust buffer and length based on offset and remaining map size
    if (*offset + length > MAP_SIZE) {
        length = MAP_SIZE - *offset;
    }
    buffer += *offset;
    
    // copy lines into buffer using copy_to_user()
    map_ptr = initials + (start_line * (MAX_LINE_LEN + 1)); // +1 for null terminator
    while (lines_read < num_lines && *map_ptr != '\0') {
        int len = strnlen(map_ptr, MAX_LINE_LEN); // limit to max line length
        if (len > 0) {
            if (copy_to_user(buffer, map_ptr, len) != 0) {
                return -EFAULT; // error copying data
            }
            buffer += len;
            bytes_read += len;
            lines_read++;
        }
        map_ptr += MAX_LINE_LEN + 1; // move to next line
    }

    // update offset and status.curr_char
    *offset += bytes_read;
    status.curr_char = *(map_ptr - 1); // set curr_char to last char copied

    return bytes_read;
}


/* This function is called when somebody tries to write
 * into our device file.
 */
static ssize_t device_write(file, buffer, length, offset)
	struct file* file;
	const char*  buffer;  /* The buffer */
	size_t       length;  /* The length of the buffer */
	loff_t*      offset;  /* Our offset in the file */
{
	int nbytes = 0;
	int space_left;

#ifdef _DEBUG
	printk
	(
		"ascii::device_write() - Length: [%d], Buf: [%s]\n",
		length,
		buffer
	);
#endif

	space_left = BSIZE - *offset;
    if (length > space_left) {
        /* Cannot write beyond the end of the buffer */
        return -ENOSPC;
    }

    /* Copy the user buffer to the driver buffer */
    if (copy_from_user(&buffer_data[*offset], buffer, length) != 0) {
        return -EFAULT;
    }

    /* Update the current buffer pointer and length */
    *offset += length;
    buffer_length = *offset;
    buffer_current_pointer = *offset;

    nbytes = length;

    return nbytes;
}


/* Initialize the module - Register the character device */
int
init_module(void)
{
	int i = 0;
	int j = 0; 
	int k = 0;

	/* Register the character device (atleast try) */
	status.major = register_chrdev
	(
		0,
		DEVICE_NAME,
		&Fops
	);

	/* Negative values signify an error */
	if(status.major < 0)
	{
		printk
		(
			"Sorry, registering the ASCII device failed with %d\n",
			status.major
		);

		return status.major;
	}

	printk
	(
		"Registeration is a success. The major device number is %d.\n",
		status.major
	);

	printk
	(
		"If you want to talk to the device driver,\n" \
		"you'll have to create a device file. \n" \
		"We suggest you use:\n\n" \
		"mknod %s c %d <minor>\n\n" \
		"You can try different minor numbers and see what happens.\n",
		DEVICE_NAME,
		status.major
	);

	// TODO
	for (i = 0; i < BSIZE; i++)
	{
		for (j = 0; j < BSIZE; j++, k++)
		{
		    if (k < BSIZE)
		    {
		        initialsBuf[k] = initials[k];
		    } else {
		        initialsBuf[k] = 0;
		    }
		}
		initialsBuf[k] = '\n';
		k++;
	}
	initialsBuf[k] = 0;

	return SUCCESS;
}


/* Cleanup - unregister the appropriate file from /proc */
void
cleanup_module(void)
{
	unregister_chrdev(status.major, DEVICE_NAME);
}

MODULE_LICENSE("GPL");

/* EOF */
