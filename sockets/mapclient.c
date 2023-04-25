#include "common.h"

int main (int argc, char *argv[])
{
	port = DEFAULT_PORT;
	ip = IP;

	if(argc > 1)
	{
		ip = argv[1];
	}
}
