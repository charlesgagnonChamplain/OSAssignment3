#include "common.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

void print_map(char *map, int width, int height)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			printf("%c", map[i * width + j]);
		}
		printf("\n");
	}
}

int main (int argc, char *argv[])
{
	int sockfd = 0, n = 0;
	struct sockaddr_in serv_addr;

	int port = PORT;
	char* ip = IP;
	int width, height;

	if(argc == 2)
	{
		ip = argv[1];
	}

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 

	printf("Enter width and height (separated by a space)");
    printf("If no customization of size is desired, enter 0 for both: ");
    scanf("%d %d", &width, &height);

    char reqBuff[3];
    reqBuff[0] = 'M';

    if(width == 0 && height == 0)
    {
        reqBuff[1] = 0;
        write(sockfd, reqBuff, 2);
    }
    else
    {
        reqBuff[1] = width;
        reqBuff[2] = height;
        write(sockfd, reqBuff, 3);
    }

	char response;
	if (read(sockfd, &response, 1) != 1)
	{
		fprintf(stderr, "\nError : Failed to read response type\n");
		close(sockfd);
		return 1;
	}

	if (response == 'M')
	{
		server_map_resp serverMap;
		read(sockfd, &serverMap, sizeof(serverMap));
		int buffSize = serverMap.width * serverMap.height;
		char *mapBuff = malloc(buffSize + 1);
		
		while((n = read(sockfd, mapBuff, sizeof(mapBuff))) > 0)
		{
			mapBuff[n] = 0;
			printf("%s", mapBuff);
		}

		if (n < 0)
		{
			perror("\nError : Failed to receive reply\n");
		}
		
		print_map(mapBuff, serverMap.width, serverMap.height);
	}
	else
	{
		server_err_resp serverErr;
		read(sockfd, &serverErr, sizeof(serverErr));
		int buffSize = serverErr.err_len;
		char *errBuff = malloc(buffSize + 1);
		
		while((n = read(sockfd, errBuff, sizeof(errBuff))) > 0)
		{
			errBuff[n] = 0;
			fprintf(stderr, "%s\n", errBuff);
		}

		if(n < 0)
		{
			perror("Error: Unrecognized protocol message");
		}
	}

	close(sockfd);
	return 0;
	
}
