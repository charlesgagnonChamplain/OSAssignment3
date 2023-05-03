#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

#define DEVICE "/dev/asciimap"
#define BUF_SIZE 2048


int main()
{
    int fd, j, n;
    char buf[BUF_SIZE];

	char read_buf[BUF_SIZE];
    char write_buf[] = "Hello, driver!";

    // Open device file
    if((fd = open(DEVICE, O_RDWR)) < 0)
    {
        perror("Could not open device");
        exit(1);
    }

    // Read data from the driver's device file
    ssize_t num_bytes_read = read(fd, read_buf, sizeof(read_buf));
    if (num_bytes_read < 0) {
        perror("Failed to read from /dev/asciimap");
        exit(EXIT_FAILURE);
    }
    printf("Read %ld bytes from /dev/asciimap: %s\n", num_bytes_read, read_buf);

    // Write data to the driver's device file
    ssize_t num_bytes_written = write(fd, write_buf, sizeof(write_buf));
    if (num_bytes_written < 0) {
        perror("Failed to write to /dev/asciimap");
        exit(EXIT_FAILURE);
    }
    printf("Wrote %ld bytes to /dev/asciimap: %s\n", num_bytes_written, write_buf);

    // Read the data written to the driver's device file
    memset(read_buf, 0, BUF_SIZE);
    num_bytes_read = read(fd, read_buf, sizeof(write_buf));
    if (num_bytes_read < 0) {
        perror("Failed to read from /dev/asciimap");
        exit(EXIT_FAILURE);
    }
    printf("Read %ld bytes from /dev/asciimap: %s\n", num_bytes_read, read_buf);

    // Zero out the buffer
    printf("Zeroing out the buffer:\n");
    if(ioctl(fd, IOCTL_ZERO_OUT, 0) < 0)
    {
        perror("ioctl");
    }
    n = read(fd, buf, BUF_SIZE);
    for(j = 0; j < n; j++) {
        printf("%c", buf[j]);
    }
    printf("\n");

    // Resetting the map
    printf("Resetting map:\n");
    if(ioctl(fd, IOCTL_RESET_MAP, 0) < 0) {
        perror("ioctl");
    }
    n = read(fd, buf, BUF_SIZE);
    for(j = 0; j < n; j++)
    {
        printf("%c",buf[j]);
    }
    printf("\n");

    // Checking for consistency
    printf("Checking for consistency:\n");
    if(ioctl(fd, IOCTL_CHECK_CONSISTENCY, 0) < 0)
    {
        perror("ioctl");
    } else {
        printf("Map is consistent\n");
    }

    // Using lseek to demonstrate all three forms of "whence"
    printf("Using lseek() and all three forms of \"whence\":\n");
    lseek(fd, 0, SEEK_SET);
    n = read(fd, buf, 5);
    for(j = 0; j < n; j++)
    {
        printf("%c", buf[j]);
    }
    printf("\n");

    lseek(fd, -5, SEEK_END);
    n = read(fd, buf, 5);
    for(j = 0; j < n; j++)
    {
        printf("%c", buf[j]);
    }
    printf("\n");

    lseek(fd, 5, SEEK_CUR);
    n = read(fd, buf, 5);
    for(j = 0; j < n; j++)
    {
        printf("%c", buf[j]);
    }
    printf("\n");

    // Close device
    close(fd);
    return 0;
}
