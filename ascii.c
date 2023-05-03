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
#include "driverAndTestHeader.h"

#define BSIZE 2048
#define STATIC_ROWSIZE 50
#define STATIC_COLSIZE 51
#define STATIC_BSIZE ((STATIC_COLSIZE * STATIC_ROWSIZE) + 1)

const char* initials = "ACCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCH\n"
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
	0,	   /* map size */
	NULL,  /* buffer's ptr */
	-1,    /* major */
	-1     /* minor */	
};

static int memory_copy(char* dst, const char* src)
{
	int count = 0;
	{
		while ((*dst++ = *src++))
		{
			count++;
		}
		count++;
	}
	return count;

}

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

	status.busy = true;

	/* Initialize the message. */
	sprintf
	(
		status.buf,
		"If I told you once, I told you %d times - %s",
		counter++,
		"Hello, world\n"
	);

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
    loff_t      offset;  /* Our offset in the file */
{
    int bytes_read = 0;
    int error = 0;

    while (length > 0) {
        error = put_user(*status.buf_ptr++, buffer++);
        if (error == -EFAULT)
        {
            return error;
        }
        bytes_read++;
        length--;
    }

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
	int bytes_written = 0;
	int error = 0;

	while (length > 0 && status.buf_ptr - status.buf < BSIZE - 1) /* saving room for \0 */
	{
		/* get_user is the weirdest macro ever. */
		error = get_user(*status.buf_ptr, buffer);

		if (error == -EFAULT)
		{
			return error;
		}

		status.buf_ptr++;
		buffer++;
		bytes_written++;
		length--;

		if (status.buf_ptr - status.buf > status.map_byte_length)
		{
			status.map_byte_length++;
		}
	}

	if (status.buf_ptr - status.buf == status.map_byte_length)
	{
		*(status.buf_ptr + 1) = '\0';
	}

	return bytes_written;
}

static long device_ioctl(file, ioctl_num, ioctl_param)
	struct file* file;
	unsigned int ioctl_num; /* number and param for ioctl  */
	unsigned long ioctl_param;
{
	char *temp;

    switch (ioctl_num) {
        case IOCTL_RESET_MAP: // Reset map back to original default
            temp = status.buf;
            while(*temp)
            {
                *temp = '\0';
                temp++;
            }
            status.map_byte_length = memory_copy(status.buf, initials) - 1;
            status.buf_ptr = status.buf;
            break;
        case IOCTL_ZERO_OUT: // Zero out entire buffer
            temp = status.buf;
            while(*temp)
            {
                *temp = '\0';
                temp++;
            }
            status.buf_ptr = status.buf;
            break;
        case IOCTL_CHECK_CONSISTENCY: // Check map for consistency
            int width = 0;
            int line_count = 0;
            temp = status.buf;
            while(*temp && *temp != '\n')
            {
                width++;
                temp++;
            }

            temp = status.buf;

            while(*temp)
            {
                line_count++;
                if(*temp == '\n')
                {
                    line_count--;
                    if(line_count != width)
                        return -1;
                    else
                        line_count = 0;
                }
                else if (*temp < 32)
                    return -1;
            }
            temp++;
            break;
        default:
            break;
	}
	return 1;
}


static loff_t device_seek(struct file* file, loff_t offset, int whence) {
    int error = 1;
    int new_pointer;
    switch (whence) {
        case SEEK_SET: // Seek from beginning of file
            if(offset < BSIZE && offset >= 0)
            {
                status.buf_ptr = (status.buf + offset);
                error = 0;
            }
            break;
        case SEEK_CUR: // Seek from current position
            new_pointer = (status.buf_ptr - status.buf) + offset;
            if(new_pointer < BSIZE && new_pointer > 0)
            {
                status.buf_ptr = (status.buf + new_pointer);
                error = 0;
            }
            break;
        case SEEK_END: // Seek from end of file
            if (offset <= 0 && offset > BSIZE)
            {
                status.buf_ptr = ((status.buf + BSIZE - 1) + offset);
                error = 0;
            }
            break;
        default:
            break;
    }
    if (error == 1) {
        return -ESPIPE;
    }

    return 0;
}



/* Initialize the module - Register the character device */
int
init_module(void)
{
	int i = 0;
	int j = 0; 
	int k = 0;	
	for (i = 0; i < BSIZE; i++)
	{
		for (j = 0; j < BSIZE; j++, k++)
		{
		    if (k < BSIZE)
		    {
		        status.buf[k] = initials[k];
		    } else {
		        status.buf[k] = 0;
		    }
		}
		status.buf[k] = '\n';
		k++;
	}
	status.buf[k] = 0;

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
