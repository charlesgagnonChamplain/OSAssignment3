#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

#define DEVICE "/dev/asciimap"
#define BSIZE 2048
#define CHECK_MAP_OK 1
#define CHECK_MAP_FAIL 0
#define MAJOR_NUM 	130
#define IOCTL_RESET_MAP 	_IO(MAJOR_NUM, 0) /*reset to the default map*/
#define IOCTL_ZERO_OUT		_IO(MAJOR_NUM, 1) /*zeros out the buffer */
#define IOCTL_CHECK_CONSISTENCY _IO(MAJOR_NUM, 2) /*checks that consistency*/

int main() {
    char read_buf[BSIZE];
    char write_buf[] = "Hello, driver!";
    int fd = open("/dev/asciimap", O_RDWR);
    if (fd < 0) {
        perror("Failed to open /dev/asciimap");
        exit(EXIT_FAILURE);
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
    memset(read_buf, 0, BSIZE);
    num_bytes_read = read(fd, read_buf, sizeof(write_buf));
    if (num_bytes_read < 0) {
        perror("Failed to read from /dev/asciimap");
        exit(EXIT_FAILURE);
    }
    printf("Read %ld bytes from /dev/asciimap: %s\n", num_bytes_read, read_buf);

    close(fd);
    return 0;
}
