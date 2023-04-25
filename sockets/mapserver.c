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

// Log
#define LOG "mapserver.log"
int log_fd = -1;

// Errors
#define CHAR_ERR -1
#define HEIGHT_ERR -2
#define WIDTH_ERR -3
const char* ERRORS[] = {"ERROR: Unrecognized char",
			"ERROR: Height is negative",
			"ERROR: Width is negative"};

void log_message(const char* msg) {
	if (log_fd >= 0)
		write(log_fd, msg, strlen(msg));
	write(log_fd, "\n", 1);
}

void fatal(const char* msg) {
	perror(msg);
	if (msg != NULL)
		log_message(msg);
	exit(1);
}

int request_check(const client_map_req* cli_req)
{
	int returnval = 0;

	if (cli_req->width < 0)
		returnval = WIDTH_ERR;
	else if (cli_req->width != 0 && cli_req->height < 0)
		returnval = HEIGHT_ERR;
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
	srv_resp.err_len = strlen(ERRORS[index]);
	strncpy(msg, ERRORS[index], 50);

	n = write(connfd, ERROR, 1);
	if (n < 0)
		fatal(NULL);

	n = write(connfd, &srv_resp, sizeof(srv_resp));
	if (n < 0)
		fatal(NULL);

	n = write(connfd, msg, strlen(msg) + 1);
	if (n < 0)
		fatal(NULL);

	log_message("Responded to error with message.");
	return 0;
}

int request_response(int connfd, const client_map_req* cli_req)
{
	int width,
	    height;

	width = cli_req->width;
	height = cli_req->height;
	server_map_resp map_resp;

	int err = 0;
	if ((err = request_check(cli_req)) < 0)
	{
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
	char msg[MSGLEN] = {0};
	int str_len = 0;
	int map_len = 0;

	memset(&msg[str_len], BSIZE, sizeof(char));
	str_len += sizeof(char);
	memcpy(&msg[str_len], &map_resp, sizeof(map_resp));
	str_len += sizeof(map_resp);

	map_len = MIN(MSGLEN - str_len - 1, (width + 1) * height);

	{
		int fd = creat("tmp.map", S_IRWXU);
		if (fd < 0)
			fatal("Could not create tmp.map");

		write(fd, map, strlen(map));
		close(fd);

		fd = creat("map4client.map", S_IRWXU);
		int save_out = dup(STDOUT_FILENO);

		if (-1 == dup2(fd, STDOUT_FILENO))
			fatal("Failed to redirect stdout");

		pid_t pid = fork();

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
		else
		{
			int err = 0;
			wait(NULL);

			fflush(stdout);
			close(fd);

			dup2(save_out, STDOUT_FILENO);

			close(fd);
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

	{
		char loginfo[80] = {0};
		snprintf(loginfo, 80, "Responded to map request with width %d and height %d", width, height);
		log_message(loginfo);
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
	log_fd = open(LOG, O_TRUNC | O_CREAT | O_WRONLY, S_IRWXU); 

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

		if (connfd < 0)
			fatal(NULL);

		memset(&serv_addr, 0, sizeof(serv_addr));

		{
			pid_t pid = fork();
			if (pid == 0)
			{
				char cmd = 0;
				n = read(connfd, &cmd, sizeof(cmd));
				if (n < 0)
					fatal(NULL);

				switch (cmd)
				{
				case 'M':
					{
						log_message("Received map request.");
						client_map_req cli_req;
						n = read(connfd, &cli_req, sizeof(cli_req));
						if (n < 0)
							fatal(NULL);
						request_response(connfd, &cli_req);
						log_message("Successfully responded to map request.");
					}
					break;
				default:
					log_message("Received unrecognized request.");
					respond_err(connfd, CHAR_ERR);
					log_message("Successfully responded to unrecognized request.");
					break;
				}

				log_message("Closing connection.");
				close(connfd);
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

	close(log_fd);

}
