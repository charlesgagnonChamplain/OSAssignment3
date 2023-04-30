#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>

#define MIN(a, b) (a < b ? a : b)

/* Error codes */
#define ECHAR -1
#define ENEGHEIGHT -2
#define ENEGWIDTH -3

#define LOG_FILENAME "mapserver.log"

int logfd = -1;

const char* ERR_MSGS[] = {"ERROR: Unrecognized char",
			"ERROR: Height is negative",
			"ERROR: Width is negative"};

void logmsg(const char* msg)
{
	if (logfd >= 0)
		write(logfd, msg, strlen(msg));
	write(logfd, "\n", 1);
}

void fatal(const char* msg)
{
	perror(msg);
	if (msg != NULL)
		logmsg(msg);
	exit(1);
}

int is_request_good(const client_map_req* cli_req)
{
	int returnval = 0;

	if (cli_req->width < 0)
		returnval = ENEGWIDTH;
	else if (cli_req->width != 0 && cli_req->height < 0)
		returnval = ENEGHEIGHT;
	else
		returnval = 0;

	return returnval;
}

int respond_err(int connfd, int err)
{
	char msg[50] = {0};
	int n;
	int index;
	index = err * -1 - 1;

	server_err_resp srv_resp;
	srv_resp.err_len = strlen(ERR_MSGS[index]);
	strncpy(msg, ERR_MSGS[index], 50);

	n = write(connfd, SRV_ERR_CHAR, 1);
	if (n < 0)
		fatal(NULL);

	n = write(connfd, &srv_resp, sizeof(srv_resp));
	if (n < 0)
		fatal(NULL);

	n = write(connfd, msg, strlen(msg) + 1);
	if (n < 0)
		fatal(NULL);

	logmsg("Responded to error with message.");

	/* Log extra info */
	{
		char errmsg[80] = {0};
		strncpy(errmsg, ERR_MSGS[index], 79);
		logmsg("Error message:");
		logmsg(errmsg);
	}

#ifdef _DEBUG
	printf("Wrote message: %s\n", msg);
	printf("Message length: %d\n", n);
#endif

	return 0;
}

int respond_to_map_request(int connfd, const client_map_req* cli_req)
{
	int width,
	    height;

	width = cli_req->width;
	height = cli_req->height;
	server_map_resp map_resp;

	
	/* Check message validity */
	int err = 0;
	if ((err = is_request_good(cli_req)) < 0)
	{
#ifdef _DEBUG
		printf("Problem with map request\n");
#endif
		respond_err(connfd, err);
		return -1;
	}

	if (width == 0)
	{
		width = 50;
		height = 50;
	}

	map_resp.width = width;
	map_resp.height = height;

	int n;
	char map[BSIZE];
	int mapfd = open("/dev/asciimap", O_RDONLY);
	if (mapfd < 0)
		fatal("Error opening /dev/asciimap");

	n = read(mapfd, map, BSIZE);
	if (n < 0)
		fatal("Error reading /dev/asciimap");
	close(mapfd);

#define MSGLEN (BSIZE + 50)

	/* The complete message to write to the socket */
	char msg[MSGLEN] = {0};

	/* The number of characters to write to the socket */
	int str_len = 0;

	/* The actual length of the map to write */
	int map_len = 0;

	memset(&msg[str_len], SRV_MAP_CHAR[0], sizeof(char));
	str_len += sizeof(char);
	memcpy(&msg[str_len], &map_resp, sizeof(map_resp));
	str_len += sizeof(map_resp);

	map_len = MIN(MSGLEN - str_len - 1, (width + 1) * height);

	/* Cut up the lines until they're the correct width */
	{
		/* Create a file for the other program to read */
		int fd = creat("tmp.map", S_IRWXU);
		if (fd < 0)
			fatal("Could not create tmp.map");

		/* Write to the new file */
		write(fd, map, strlen(map));

		/* Close the file for someone else to use */
		close(fd);

		/* Create a file for the other program to write to */
		fd = creat("map4client.map", S_IRWXU);

		/* Save our stdout so we don't lose it */
		int save_out = dup(STDOUT_FILENO);

		/* dup2 stdout -- writing to stdout won't work until we set
		 * this back to normal. */
		if (-1 == dup2(fd, STDOUT_FILENO))
			fatal("Failed to redirect stdout");

		/* Get ready to spawn a new process... */
		pid_t pid = fork();

		/* Child */
		if (pid == 0)
		{
			char* new_argv[5];
			char arg1[20] = {0};
			char arg2[20] = {0};
			char arg3[20] = {0};

			snprintf(arg1, 20, "-w%d", width);
			snprintf(arg2, 20, "-h%d", height);
			snprintf(arg3, 20, "tmp.map");

			new_argv[0] = "../testForkExec";
			new_argv[1] = arg1;
			new_argv[2] = arg2;
			new_argv[3] = arg3;
			new_argv[4] = NULL;

			execve("../testForkExec", new_argv, NULL);
			fatal("execve failed");
		}
		/* Parent */
		else
		{
			int err = 0;
			wait(NULL);

			/* Clean up the mess we made for the child */
			fflush(stdout);
			close(fd);

			dup2(save_out, STDOUT_FILENO);

			close(fd);

			/* Now read our lovely new map file */
			fd = open("map4client.map", O_RDONLY);

			n = read(fd, map, map_len);
			if (n < 0)
				fatal("Error reading map");

			close(fd);

			err = unlink("map4client.map");
			if (err < 0)
				fatal("Unable to unlink map4client.map");
			err = unlink("tmp.map");
			if (err < 0)
				fatal("Unable to unlink tmp.map");
		}
	}

	strncat(&msg[str_len], map, map_len);
	str_len += strlen(&msg[str_len]) + 1;

	n = write(connfd, msg, str_len);
	if (n < 0)
		fatal(NULL);

#undef MSGLEN

#ifdef _DEBUG
	printf("Received map request\n");
	printf("Wrote message:\n");
	write(STDOUT_FILENO, msg, str_len);
	printf("Message has length: %d\n", str_len);
#endif

	/* Handle logging */
	{
		char loginfo[80] = {0};
		snprintf(loginfo, 80, "Responded to map request with width %d and height %d", width, height);
		logmsg(loginfo);
	}

	return 0;
}


int main(void)
{
	int sockfd;
	int connfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	/* init logging */
	logfd = open(LOG_FILENAME, O_TRUNC | O_CREAT | O_WRONLY, S_IRWXU); 

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		fatal(NULL);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PORT);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0)
		fatal(NULL);

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	while (1)
	{
		connfd = accept(sockfd,
				(struct sockaddr *) &cli_addr,
				&clilen);
#ifdef _DEBUG
		printf("Received request\n");
#endif

		if (connfd < 0)
			fatal(NULL);

		memset(&serv_addr, 0, sizeof(serv_addr));

		/* Identify request */
		{
			pid_t pid = fork();
			if (pid == 0)
			{
				char cmd = 0;
				n = read(connfd, &cmd, sizeof(cmd));
				if (n < 0)
					fatal(NULL);

#ifdef _DEBUG
				printf("Command char: %c\n", cmd);
#endif
				switch (cmd)
				{
				case 'M':
					{
						logmsg("Received map request.");
						client_map_req cli_req;
						n = read(connfd, &cli_req, sizeof(cli_req));
						if (n < 0)
							fatal(NULL);
						respond_to_map_request(connfd, &cli_req);
						logmsg("Successfully responded to map request.");
					}
					break;
				default:
					logmsg("Received unrecognized request.");
					respond_err(connfd, ECHAR);
					logmsg("Successfully responded to unrecognized request.");
					break;
				}

				logmsg("Closing connection.");
				close(connfd);
#ifdef _DEBUG
				printf("Child, signing off!\n");
#endif
				exit(0);
			}
			else if (pid < 0)
				fatal("fork error");
			else
			{
				close(connfd);
			}
		}
	}

	/* Stop logging */
	close(logfd);

}
