#include "Server.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
	// check command line arguments
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	// parse command line arguments
	int port = atoi(argv[1]);
	std::string password = argv[2];

	// check port number
	if (port <= 0 || port > 65535) {
		std::cerr << "Invalid port number" << std::endl;
		return 1;
	}

	// start server
	Server server(port, password);	// create server
	server.start();					// start server
	
	return 0;
}
