#ifndef COMMON_H
#define COMMON_H

#include "../ascii.h"

#define PORT 23032
#define IP "127.0.0.1"

#define REQUEST 'M'

typedef struct client_request
{
	int width;
	int height;
} client_req;
