#define COMMON_H

#define PORT 23032
#define IP "127.0.0.1"
#define BSIZE 1024
#define MSGLEN (BSIZE + 50)
#define SRV_ERR_CHAR "E"
#define SRV_MAP_CHAR "M"

typedef struct client_map_request
{
	int width;
	int height;
} client_map_req;

typedef struct client_error_request
{
	int err_len;
} client_err_req;

typedef struct server_error_response
{
	int err_len;
} server_err_resp;

typedef struct server_map_response
{
	int width;
	int height;
} server_map_resp;


