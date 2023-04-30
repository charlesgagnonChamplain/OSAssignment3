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
	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			printf("%c", map[i * width + j]);
		}
		printf("\n");
	}
}

void cli_err(const char *err_msg)
{
    perror(err_msg);
    exit(0);
}

int main (int argc, char *argv[])
{
	int sockfd = 0, n = 0;
	struct sockaddr_in serv_addr;
    struct client_map_req request;
    int usr_height, usr_width;

	int port = PORT;
	char* ip = IP;
	request.height = 0;
    request.width = 0;

	if(argc == 2)
	{
		ip = argv[1];
	}

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        cli_err("\n Error : Could not create socket \n");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
        cli_err("\nError : Could not add IP");

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        cli_err("\n Error : Connect Failed \n");

	printf("Enter   width and height (separated by a space)");
    printf("If no customization of size is desired, enter 0 for both: ");
    scanf("%d %d", &usr_width, &usr_height);

    char message[2];
    snprintf(message, 2, "%c", 'M');
    write(sockfd, message, 1);

    if(usr_width != 0 && usr_height != 0)
    {
        request.width = usr_width;
        request.height = usr_height;
    }

    n = write(sockfd, request, sizeof(request));
    if(n < 0)
        cli_err("Error : Could not write to socket");

	char response;
    read(sockfd, &response, 1);

	if(response == 'M')
	{
		server_map_resp serverMap;
		read(sockfd, &serverMap, sizeof(serverMap));
		int buffSize = serverMap.width;
		char mapBuff = malloc(buffSize);
		
		while((n = read(sockfd, mapBuff, buffSize - 1)) > 0)
		{
			mapBuff[n] = 0;
			printf("%s", mapBuff);
		}

		if(n < 0)
		{
			perror("\nError : Failed to receive reply\n");
		}
		
		//print_map(mapBuff, serverMap.width, serverMap.height);
	}
	else
	{
		server_err_resp serverErr;
		read(sockfd, &serverErr, sizeof(serverErr));
		int buffSize = serverErr.err_len;
		char errBuff = malloc(buffSize);
		
		while((n = read(sockfd, errBuff, buffSize - 1)) > 0)
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
