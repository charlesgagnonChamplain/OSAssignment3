/****
 * main.c
 *
 * The ASCII character device driver mini test
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "common.h"

static char* initials =
"ACCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCH\n"
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
"ACCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCH\n";

int main(argc, argv)
	int   argc;
	char* argv[];
{
    char buf[BSIZE];	
	char intialsBuf[(BSIZE + 1)*BSIZE];
	char *ptr = buf;
    int fd, i, j, n;
	int bytes_written = 0;
	int height, width;

	int k = 0;
	for (int i = 0; i < BSIZE; i++)
	{
		for (int j = 0; j < BSIZE; j++, k++)
		{
		    if (k < strlen(initials))
		    {
		        intialsBuf[k] = initials[k];
		    } else {
		        intialsBuf[k] = 0;
		    }
		}
		intialsBuf[k] = '\n';
		k++;
	}
	intialsBuf[k] = 0;

	if((fd = open("/dev/asciimap", O_RDWR)) >= 0)
	{
		for(i = 10; i <= 60; i += 5)
		{
			n = read(fd, ptr, i);

			if(i != n)
			{
				(void) fprintf(stderr, "*** wanted %d bytes, got %d\n", i, n);

				if(n < 0)
					perror("read(asciimap) failed");
			}
			else
			{
				(void) printf("received:  ");
				for (j = 0; j < n; j++) (void) printf("%c ", buf[j]);
				(void) printf("\n");
			}

			if(i == 30)
			{
				n = write(fd, ptr, BSIZE);
                bytes_written += n;

				char *newline = strchr(ptr, '\n');
                width = newline - ptr + 1;
                height = bytes_written / width;

				if(n < 0)
					perror("write(/dev/ASCII) failed");
			}
		}

		close(fd);
	}
	else
	{
		perror("open(/dev/asciimap) failed");
		exit(1);
	}

    exit(0);
}

/* EOF */
