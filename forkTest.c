#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "driverAndTestHeader.h"
#define _GNU_SOURCE

char* genMapFromFile(char* fileName, int w, int h, int line);
void printArgUsage(void);

int main(int argc, char* argv[])
{
	int opt = 0;
	char* w = "10";
	char* h = "10";
	int line = 0;

	while((opt = getopt(argc, argv, "w:h:l:")) != -1)
	{
		switch(opt)
		{
			case 'w':
				w = optarg;
				break;
			case 'h':
				h = optarg;
				break;
			case 'l':
				line = atoi(optarg);
				break;
			default:
				printArgUsage();
				break;
		}
	}
	int nonOptArgc = argc - optind + 1;

	if(nonOptArgc == 1)
	{
		pid_t pid = fork();

		if(pid == 0)
		{
			char* genmapArgv[] = {"./genmap.sh", w, h, NULL};
			char* genmapEnviron[] = {NULL};
			execve("./genmap.sh", genmapArgv, genmapEnviron);

			perror("execve() has failed");
			exit(1);
		}
	}else{
		int i = optind;
		for(; i < argc; ++i)
		{
			pid_t pid = fork();

			if(pid == 0)
			{
				char* carvedMap = genMapFromFile(argv[i], atoi(w), atoi(h), line);

				printf("%s", carvedMap);

				free(carvedMap);
				exit(0);
			}
		}
	}
	exit(0);
}

char* genMapFromFile(char* fileName, int w, int h, int line)
{
    FILE* fid;
    char* mLine = NULL;
    if((fid = fopen(fileName, "r")))
    {
        char* tLine = NULL;
        size_t tLength = 0;

        int i = 0; 
        for(; i < line; ++i)
        {
            getline(&tLine, &tLength, fid);
        }    

        int size = w * h + h + 1;
        mLine = (char*)malloc(size);
        memset(mLine, 0, size);

        char* currLine = NULL;
        size_t currLength = 0;
        ssize_t linesRead;

        int start = 0;
        int end = 0;

        int x = 1;
        while((linesRead = getline(&currLine, &currLength, fid)) != -1 && x <= h)
        {
            strncat(mLine, currLine, w);

            char *currItr = mLine;
            while(*currItr)
            {
                if(*currItr < 32 && *currItr != '\n')
                {
                    *currItr = ' ';
                }

                currItr++;
            }

            start = w * (x - 1) + (x - 1);
            end = w * x + (x - 1);

            int eLine = start + strlen(currLine) - 1;
            if(eLine < end)
            {
                int i = eLine;
                for(; i < end; ++i)
                {
                    mLine[i] = ' ';
                }
            }

            mLine[end] = '\n';
            x++;
        }

        if(end < size - 2)
        {
            while(x <= h)
            {
                start = w * (x - 1) + (x - 1);
                end = w * x + (x - 1);

                int y = start;
                for(; y < end; ++y)
                {
                    mLine[y] = ' ';
                }

                mLine[end] = '\n';

                x++;
            }
        }

        fclose(fid);
        free(currLine);
        free(tLine);
    }
    else
    {
        printf("The file %s doesn't exist!\n", fileName);
    }

    return mLine;
}

void printArgUsage()
{
	fprintf(stderr, "Invalid arg.\nValid args are -w (width), -h (height), -l (line)\n");
}
