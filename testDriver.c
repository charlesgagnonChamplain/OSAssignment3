#include "ascii.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define DEVICE "/dev/asciimap"


int main(int argc, char *argv[])
{
    int fd, j, n;
    char buf[BSIZE];

    // Open device file
    if((fd = open(DEVICE, O_RDWR)) < 0)
    {
        perror("Could not open device");
        exit(1);
    }

    // Zero out the buffer
    printf("Zeroing out the buffer:\n");
    if(device_ioctl(fd, IOCTL_ZERO_OUT, 0) < 0)
    {
        perror("ioctl");
    }
    n = read(fd, buf, BSIZE);
    for(j = 0; j < n; j++) {
        printf("%c", buf[j]);
    }
    printf("\n");

    // Resetting the map
    printf("Resetting map:\n");
    if(device_ioctl(fd, IOCTL_RESET_MAP, 0) < 0) {
        perror("ioctl");
    }
    n = read(fd, buf, BSIZE);
    for(j = 0; j < n; j++)
    {
        printf("%c",buf[j]);
    }
    printf("\n");

    // Checking for consistency
    printf("Checking for consistency:\n");
    if(device_ioctl(fd, IOCTL_CHECK_CONSISTENCY, 0) < 0)
    {
        perror("ioctl");
    } else {
        printf("Map is consistent\n");
    }

    // Using lseek to demonstrate all three forms of "whence"
    printf("Using lseek() and all three forms of \"whence\":\n");
    device_seek(fd, 0, SEEK_SET);
    n = read(fd, buf, 5);
    for(j = 0; j < n; j++)
    {
        printf("%c", buf[j]);
    }
    printf("\n");

    device_seek(fd, -5, SEEK_END);
    n = read(fd, buf, 5);
    for(j = 0; j < n; j++)
    {
        printf("%c", buf[j]);
    }
    printf("\n");

    device_seek(fd, 5, SEEK_CUR);
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
