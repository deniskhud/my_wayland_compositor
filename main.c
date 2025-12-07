#include "src/include/server.h"

int main() {

	struct server server;

	server_init(&server);
	server_start(&server);
	server_run(&server);

	server_destroy(&server);
	return 0;
}